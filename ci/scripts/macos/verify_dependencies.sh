#!/usr/bin/env bash
##
## This file is part of the SmuView project.
##
## Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, see <http://www.gnu.org/licenses/>.
##

set -euo pipefail

APP="/Applications/SmuView.app"
FAILURE=0

echo "Checking for Mach-O dependencies..."
while IFS= read -r -d '' file; do
	file "${file}" | grep -q 'Mach-O' || continue

	# Exclude dylib/framework's own LC_ID_DYLIB line, because it looks identical
	# to a broken dependency
	self_id="$(otool -D "${file}" 2>/dev/null | tail -n +2)"

	while IFS= read -r dep; do
		[[ -n "${self_id}" && "${dep}" == "${self_id}" ]] && continue
		if [[ "${dep}" == /opt/homebrew/* || "${dep}" == /usr/local/* ]]; then
			echo "::error::External Homebrew dependency in ${file}: ${dep}"
			FAILURE=1
		fi
	done < <(otool -L "${file}" | awk 'NR>1{print $1}')
done < <(find "${APP}" -type f -print0)

PYTHON_DEP="$(otool -L "${APP}/Contents/MacOS/smuview.real" | tail -n +2)"
REF_PATH="@executable_path/\.\./Frameworks/Python\.framework/Versions/[0-9.]+/Python"
if ! [[ "${PYTHON_DEP}" =~ ${REF_PATH} ]]; then
	echo "::error::Bundled Python is not referenced correctly: ${PYTHON_DEP}"
	FAILURE=1
fi

exit "${FAILURE}"
