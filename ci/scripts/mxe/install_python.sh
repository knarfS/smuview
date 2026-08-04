#!/usr/bin/env bash
##
## This file is part of the SmuView project.
##
## Copyright (C) 2018-2026 Frank Stettner <frank-stettner@gmx.net>
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

set -e

echo "INSTALL_DIR: ${INSTALL_DIR:?INSTALL_DIR is not set}"
echo "MXE_DIR: ${MXE_DIR:?MXE_DIR is not set}"
echo "TARGET: ${TARGET:?TARGET is not set}"

PY_VER=3.14.6
PY_ABI=314
# We only build amd64 packages, no win32 any more
PY_ARCH="amd64"

mkdir -p "${INSTALL_DIR}"

# Get Python headers
wget -c --quiet https://www.python.org/ftp/python/"${PY_VER}"/Python-"${PY_VER}".tgz -O "${INSTALL_DIR}"/Python-src.tgz
mkdir -p "${INSTALL_DIR}"/Python3/include
tar xzf "${INSTALL_DIR}"/Python-src.tgz -C "${INSTALL_DIR}"/Python3/include \
	--strip-components=2 Python-"${PY_VER}"/Include
tar xzf "${INSTALL_DIR}"/Python-src.tgz -C "${INSTALL_DIR}"/Python3/include \
	--strip-components=2 Python-"${PY_VER}"/PC/pyconfig.h
rm -f "${INSTALL_DIR}"/Python-src.tgz

# Get Python runtime + DLL
wget -c --quiet https://www.python.org/ftp/python/"${PY_VER}"/python-"${PY_VER}"-embed-"${PY_ARCH}".zip \
	-O "${INSTALL_DIR}"/python-embed.zip
mkdir -p "${INSTALL_DIR}"/Python3/runtime
unzip -q "${INSTALL_DIR}"/python-embed.zip -d "${INSTALL_DIR}"/Python3/runtime
rm -f "${INSTALL_DIR}"/python-embed.zip

# Generate the MinGW-w64 import lib from the DLL
cp "${INSTALL_DIR}"/Python3/runtime/python"${PY_ABI}".dll .
"${MXE_DIR}"/usr/"${TARGET}"-w64-mingw32.static.posix/bin/gendef python"${PY_ABI}".dll
"${MXE_DIR}"/usr/bin/"${TARGET}"-w64-mingw32.static.posix-dlltool \
	--dllname python"${PY_ABI}".dll --def python"${PY_ABI}".def \
	--output-lib libpython"${PY_ABI}".a
mkdir -p "${INSTALL_DIR}"/Python3/libs
mv -f libpython"${PY_ABI}".a "${INSTALL_DIR}"/Python3/libs
rm -f python"${PY_ABI}".dll python"${PY_ABI}".def
