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
		return a.time > b.time;
	}
};

typedef priority_queue<SamplePtr, vector<SamplePtr>, SampleTimeCompare>
	signal_min_heap_t;

void replace_all(string &str, const string &from, const string &to)
{
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

string format_csv_string_value(string value)
{
	replace_all(value, "\"", "\\\"");
	return value;
}

void pop_and_push(signal_min_heap_t &heap,
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	const SamplePtr &sample_ptr, bool relative_time)
{
	heap.pop();

	size_t new_sample_pos = sample_ptr.sample_pos + 1;
	auto signal = signals.at(sample_ptr.signal_pos);

	if (signal->sample_count() <= new_sample_pos)
		return;

	auto sample = signal->get_sample(new_sample_pos, relative_time);
	heap.push(SamplePtr {
		sample.first, sample.second, sample_ptr.signal_pos, new_sample_pos });
}

} // namespace

TimeAlignedData align_timestamps(
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	bool relative_time, double tolerance)
{
	TimeAlignedData time_aligned_data {};
	// time_aligned_data.headers.push_back({"Time", "Time", "Time", "Time"});
	time_aligned_data.headers.push_back("Time");

	signal_min_heap_t min_heap;

	for (size_t signal_pos = 0; signal_pos < signals.size(); ++signal_pos) {
		SamplePtr ptr = {
			signals.at(signal_pos)->get_sample(0, relative_time).first,
			signals.at(signal_pos)->get_sample(0, relative_time).second,
			signal_pos, 0
		};
		min_heap.push(ptr);
		// time_aligned_data.headers.push_back({
		// 	signals.at(signal_pos)->parent_channel()->parent_device()->name(),
		// 	//signals.at(signal_pos)->parent_channel()->channel_group_names(),
		// 	"",
		// 	signals.at(signal_pos)->parent_channel()->name(),
		// 	signals.at(signal_pos)->name()});
		time_aligned_data.headers.push_back(signals.at(signal_pos)->name());
	}

	while (!min_heap.empty()) {
		auto ptr = min_heap.top();
		double top_time = min_heap.top().time;
		TimeAlignedRow row { top_time, {{ptr.signal_pos, ptr.data}} };
		pop_and_push(min_heap, signals, ptr, relative_time);

		while (!min_heap.empty() && top_time + tolerance >= min_heap.top().time) {
			ptr = min_heap.top();
			row.data.insert({ptr.signal_pos, ptr.data});
			pop_and_push(min_heap, signals, ptr, relative_time);
		}

		time_aligned_data.rows.push_back(row);
	}

	return time_aligned_data;
}

void export_csv(ostream &out, const TimeAlignedData &data,
	const string &separator)
{
	string sep("");
	for (const auto &name : data.headers) {
		out << sep << format_csv_string_value(name);
		sep = separator;
	}
	out << std::endl;

	size_t data_col_cnt = data.headers.size() - 1;
	for (const auto &row : data.rows) {
		out << QString::number(row.time, 'f', 4).toStdString();
		for (size_t col_pos = 0; col_pos < data_col_cnt; col_pos++) {
			out << sep;
			if (row.data.count(col_pos)) {
				const string d = QString::number(row.data.at(col_pos), 'g',
					QLocale::FloatingPointShortest).toStdString();
				out << d;
			}
		}
		out << std::endl;
	}
}

} // namespace exportutil
} // namespace utils
} // namespace sv
