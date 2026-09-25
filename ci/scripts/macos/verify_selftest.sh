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

APP_DIR="/Applications/SmuView.app/Contents"

OUTPUT=$(${APP_DIR}"/MacOS/SmuView" --selftest)
RET=$?
echo "$OUTPUT"

if ! "${RET}" ; then
    echo "::error::Selftest failed"
    exit 1 
fi

FAILED=0

SYS_PATH_PREFIX="Embedded sys.prefix: "
SYS_PATH=$(echo "${OUTPUT}" | grep -E "^${SYS_PATH_PREFIX}" | sed -E "s/^${SYS_PATH_PREFIX}//")
if echo "${SYS_PATH}" | grep -qi "homebrew"; then
    echo "::error::sys.path contains homebrew fragment"
    FAILED=1
fi
if echo "${SYS_PATH}" | grep -qi "/usr/local/lib/python"; then
    echo "::error::sys.path contains /usr/local/lib/python fragment"
    FAILED=1
fi
if echo "${SYS_PATH}" | grep -qi "/Library/Frameworks/Python.framework"; then
    echo "::error::sys.path contains /Library/Frameworks/Python.framework fragment"
    FAILED=1
fi

EXPECTED_PYTHON_ROOT="${APP_DIR}"/Frameworks/Python.framework
OS_FILE_PREFIX="Embedded os.__file__: "
OS_FILE=$(echo "${OUTPUT}" | grep -E "^${OS_FILE_PREFIX}" | sed -E "s/^${OS_FILE_PREFIX}//")
if [[ "${OS_FILE}" != "${EXPECTED_PYTHON_ROOT}"* ]]; then
    echo "::error::os.__file__ is not inside the bundled python framework"
    FAILED=1
fi

exit "${FAILED}"
