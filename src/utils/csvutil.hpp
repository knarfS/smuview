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

#ifndef UTILS_CSVUTIL_HPP
#define UTILS_CSVUTIL_HPP

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace sv {
namespace utils {
namespace csvutil {

/**
 * Format and escape a single CSV field value:
 * - Double quote
 * - Suround with quote when value contains quotes, \n, \r or the separator
 *
 * @param[in] value The raw value
 * @param[in] separator the CSV field separator
 *
 * @return The escaped and quoted CSV field
 */
string format_csv_value(string value, char separator);

/**
 * Parse a single CSV line.
 * Based on https://stackoverflow.com/a/30338543
 *
 * @param[in] line The CSV line to parse
 * @param[in] separator the CSV field separator
 *
 * @return A vector with the CSV field values
 */
vector<string> parse_csv_line(const string &line, char separator);

} // namespace csvutil
} // namespace utils
} // namespace sv

#endif // UTILS_CSVUTIL_HPP
