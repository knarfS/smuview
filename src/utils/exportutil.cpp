/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <queue>
#include <vector>

#include <QLocale>
#include <QString>

#include "exportutil.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/utils/csvutil.hpp"
#include "src/util.hpp"

using std::priority_queue;
using std::vector;

namespace sv {
namespace utils {
namespace exportutil {

namespace {

struct SamplePtr {
	double time;
	double data;
	size_t signal_pos;
	size_t sample_pos;
};

struct SampleTimeCompare {
	bool operator()(const SamplePtr &a, const SamplePtr &b) const {
		if (a.time == b.time) {
			if (a.signal_pos == b.signal_pos) {
				return a.sample_pos > b.sample_pos;
			}
			return a.signal_pos > b.signal_pos;
		}
		return a.time > b.time;
	}
};

typedef priority_queue<SamplePtr, vector<SamplePtr>, SampleTimeCompare>
	signal_min_heap_t;

string get_signal_header(const shared_ptr<sv::data::AnalogTimeSignal> &signal)
{
	string header;
	if (signal->parent_channel()) {
		const auto &parent_channel = signal->parent_channel();
		if (parent_channel->parent_device())
			header = parent_channel->parent_device()->name() + " / ";
		if (!parent_channel->channel_group_names().empty()) {
			string chg_sep = "";
			for (const auto &chg : parent_channel->channel_group_names()) {
				header += chg_sep + chg;
				chg_sep = "+";
			}
			header += " / ";
		}
		header += parent_channel->name() + " / ";
	}
	header += signal->name();
	return header;
}

void pop_and_push(signal_min_heap_t &heap,
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	bool relative_time)
{
	SamplePtr sample_ptr = heap.top();

	heap.pop();

	auto signal = signals.at(sample_ptr.signal_pos);
	size_t new_sample_pos = sample_ptr.sample_pos + 1;
	auto sample = signal->get_sample(new_sample_pos, relative_time);
	if (!sample.has_value())
		return;

	heap.push(SamplePtr {
		sample->first, sample->second,
		sample_ptr.signal_pos, new_sample_pos });
}

} // namespace

TimeAlignedData align_timestamps(
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	bool relative_time, double tolerance)
{
	if (tolerance < 0.)
		tolerance = 0.;

	TimeAlignedData time_aligned_data {};

	string time_header = "Time";
	if (relative_time)
		time_header += " [s]";
	time_aligned_data.headers.push_back(time_header);
	time_aligned_data.relative_time = relative_time;

	signal_min_heap_t min_heap;

	for (size_t signal_pos = 0; signal_pos < signals.size(); ++signal_pos) {
		time_aligned_data.headers.push_back(
			get_signal_header(signals.at(signal_pos)));

		auto sample = signals.at(signal_pos)->get_sample(0, relative_time);
		if (!sample.has_value())
			continue;

		min_heap.push(SamplePtr {
			sample.value().first, sample.value().second, signal_pos, 0});
	}

	while (!min_heap.empty()) {
		double top_time = min_heap.top().time;
		vector<optional<double>> data(signals.size());
		data[min_heap.top().signal_pos] = min_heap.top().data;
		pop_and_push(min_heap, signals, relative_time);

		while (!min_heap.empty()
				&& top_time + tolerance >= min_heap.top().time
				&& !data.at(min_heap.top().signal_pos).has_value()) {
			data[min_heap.top().signal_pos] = min_heap.top().data;
			pop_and_push(min_heap, signals, relative_time);
		}

		time_aligned_data.rows.push_back(
			TimeAlignedRow {top_time, std::move(data)});
	}

	return time_aligned_data;
}

void export_csv_aligned(ostream &out, const TimeAlignedData &data,
	char separator)
{
	string sep("");
	for (const auto &name : data.headers) {
		out << sep << csvutil::format_csv_value(name, separator);
		sep = separator;
	}
	out << "\n";

	for (const auto &row : data.rows) {
		string t;
		if (data.relative_time)
			t = QString::number(row.time, 'f', 4).toStdString();
		else
			t = sv::util::format_time_date(row.time).toStdString();
		out << csvutil::format_csv_value(t, separator);

		for (const auto &e : row.data) {
			out << sep;
			if (e.has_value()) {
				const string d = QString::number(e.value(), 'g',
					QLocale::FloatingPointShortest).toStdString();
				out << csvutil::format_csv_value(d, separator);
			}
		}
		out << "\n";
	}
}

void export_csv_parallel(ostream &out,
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	char separator, bool relative_time)
{
	if (signals.empty())
		return;

	string sep("");
	bool has_more_samples = false;
	for (const auto &signal : signals) {
		string signal_header = get_signal_header(signal);
		string time_header = "Time";
		if (relative_time)
			time_header += " [s]";
		time_header += ": " + signal_header;
		out << sep << csvutil::format_csv_value(time_header, separator);
		sep = separator;
		out << sep << csvutil::format_csv_value(signal_header, separator);

		if (!has_more_samples)
			has_more_samples = signal->sample_count() > 0;
	}
	out << "\n";

	size_t current_sample_pos = 0;
	while (has_more_samples) {
		sep = "";
		has_more_samples = false;
		for (const auto &signal : signals) {
			const auto sample =
				signal->get_sample(current_sample_pos, relative_time);
			if (sample.has_value()) {
				QString t;
				if (relative_time)
					t = QString::number(sample.value().first, 'f', 4);
				else
					t = sv::util::format_time_date(sample.value().first);
				out << sep
					<< csvutil::format_csv_value(t.toStdString(), separator);
				sep = separator;

				const string d = QString::number(sample.value().second, 'g',
					QLocale::FloatingPointShortest).toStdString();
				out << sep << csvutil::format_csv_value(d, separator);

				if (signal->sample_count() > current_sample_pos + 1)
					has_more_samples = true;
			}
			else {
				out << sep << separator;
				sep = separator;
			}
		}
		out << "\n";

		if (has_more_samples)
			current_sample_pos++;
	}
}

} // namespace exportutil
} // namespace utils
} // namespace sv
