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

echo "INSTALL_DIR: ${INSTALL_DIR:?INSTALL_DIR is not set}"
echo "PYTHON_VERSION: ${PYTHON_VERSION:?PYTHON_VERSION is not set}"

# Remove patch part from version
PYTHON_ABI_VERSION="${PYTHON_VERSION%.*}"

# Download and unpack Python for macOS from python.org
WORKDIR=$(mktemp -d)
wget -c --quiet -O "${WORKDIR}/python.pkg" \
	"https://www.python.org/ftp/python/${PYTHON_VERSION}/python-${PYTHON_VERSION}-macos11.pkg"
pkgutil --expand-full "${WORKDIR}/python.pkg" "${WORKDIR}/Frameworks"

# Copy the python framework into the INSTALL_DIR
DEST_DIR="${INSTALL_DIR}/Frameworks/Python.framework"
mkdir -p "${DEST_DIR}"
cp -R "${WORKDIR}/Frameworks/Python_Framework.pkg/Payload/" "${DEST_DIR}/"
chmod -R u+w "${DEST_DIR}"

PYTHON_DIR="${DEST_DIR}/Versions/${PYTHON_ABI_VERSION}"
OLD_ABS_REF="/Library/Frameworks/Python.framework/Versions/${PYTHON_ABI_VERSION}/Python"

# Fix hardcoded absolute LC_LOAD_DYLIB reference in python.org's executables.
# These are only used by CMake to detect the python runtime and will be removed
# from the DMG later
install_name_tool -change "${OLD_ABS_REF}" "@loader_path/../Python" \
	"${PYTHON_DIR}/bin/python${PYTHON_ABI_VERSION}"
install_name_tool -change "${OLD_ABS_REF}" "@loader_path/../../../../Python" \
	"${PYTHON_DIR}/Resources/Python.app/Contents/MacOS/Python"

# Verify rpaths
DYLD_TRACE=$(DYLD_PRINT_LIBRARIES=1 "${PYTHON_DIR}/bin/python${PYTHON_ABI_VERSION}" --version 2>&1)
echo "${DYLD_TRACE}"
if echo "$DYLD_TRACE" | grep -qF "${OLD_ABS_REF}"; then
	echo "::error::dyld still resolved ${OLD_ABS_REF}"
	exit 1
fi

