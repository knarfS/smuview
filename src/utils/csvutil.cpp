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

#include <string>
#include <vector>

#include "csvutil.hpp"

using std::string;
using std::vector;

namespace sv {
namespace utils {
namespace csvutil {

namespace {

void replace_all(string &str, const string &from, const string &to)
{
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

}

string format_csv_value(string value, char separator)
{
	replace_all(value, "\"", "\"\"");
	if (value.find(separator) != string::npos
			|| value.find("\"") != string::npos
			|| value.find("\n") != string::npos
			|| value.find("\r") != string::npos)
		value = "\"" + value + "\"";
	return value;
}

/*
 * Based on https://stackoverflow.com/a/30338543
 */
vector<string> parse_csv_line(const string &line, char separator)
{
	enum State { UnquotedField, QuotedField, QuotedQuote } state = UnquotedField;
	std::vector<std::string> fields{""};

	size_t index = 0; // index of the current field
	for (char chr : line) {
		switch (state) {
		case State::UnquotedField:
			if (chr == separator) {
				// end of field
				fields.push_back("");
				index++;
			}
			else if (chr == '"') {
				state = State::QuotedField;
			}
			else {
				fields[index].push_back(chr);
			}
			break;

		case State::QuotedField:
			if (chr == '"') {
				state = State::QuotedQuote;
			}
			else {
				fields[index].push_back(chr);
			}
			break;

		case State::QuotedQuote:
			if (chr == separator) {
				// , after closing quote
				fields.push_back("");
				index++;
				state = State::UnquotedField;
			}
			else if (chr == '"') {
				// "" -> "
				fields[index].push_back('"');
				state = State::QuotedField;
			}
			else {
				// end of quote
				state = State::UnquotedField;
			}
			break;
		}
	}

	return fields;
}

} // namespace csvutil
} // namespace utils
} // namespace sv
