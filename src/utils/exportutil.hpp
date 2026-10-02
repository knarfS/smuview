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

#ifndef UTILS_EXPORTUTIL_HPP
#define UTILS_EXPORTUTIL_HPP

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "src/data/analogtimesignal.hpp"

using std::ostream;
using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace utils {
namespace exportutil {

struct TimeAlignedHeader
{
	string device;
	string channel_group;
	string channel;
	string signal;
};

struct TimeAlignedRow
{
	double time;
	map<size_t, double> data; // column index, value
};

struct TimeAlignedData
{
	//vector<TimeAlignedHeader> headers;
	vector<string> headers;
	vector<TimeAlignedRow> rows;
};

TimeAlignedData align_timestamps(
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	bool absolute_time, double tolerance);

void export_csv(ostream &out, const TimeAlignedData &data,
	const string &separator);

} // namespace exporteutil
} // namespace utils
} // namespace sv

#endif // UTILS_EXPORTUTIL_HPP
