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

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "src/data/analogtimesignal.hpp"

using std::optional;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace utils {
namespace exportutil {

struct TimeAlignedRow
{
	double time;
	vector<optional<double>> data;
};

struct TimeAlignedData
{
	vector<string> headers;
	vector<TimeAlignedRow> rows;
	bool relative_time;
};

TimeAlignedData align_timestamps(
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	bool relative_time, double tolerance);

void export_csv_aligned(ostream &out, const TimeAlignedData &data,
	char separator);

void export_csv_parallel(ostream &out,
	const vector<shared_ptr<sv::data::AnalogTimeSignal>> &signals,
	char separator, bool relative_time);

} // namespace exportutil
} // namespace utils
} // namespace sv

#endif // UTILS_EXPORTUTIL_HPP
