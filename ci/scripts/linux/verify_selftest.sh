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
set -x

OUTPUT=$(xvfb-run -a ./SmuView.AppImage --selftest)
RET=$?
echo "$OUTPUT"

if [ "${RET}" -ne 0 ]; then
    echo "::error::Selftest failed"
    exit 1
fi

FAILED=0

EXPECTED_PYTHON_ROOT="/xxlltmp"
OS_FILE_PREFIX="Embedded os.__file__: "
OS_FILE=$(echo "${OUTPUT}" | grep -E "^${OS_FILE_PREFIX}" | sed -E "s/^${OS_FILE_PREFIX}//")
if [[ "${OS_FILE}" != "${EXPECTED_PYTHON_ROOT}"* ]]; then
    echo "::error::os.__file__ is not inside the bundled python framework"
    FAILED=1
fi

exit "${FAILED}"
