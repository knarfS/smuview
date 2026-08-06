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

set -e

echo "INSTALL_DIR: ${INSTALL_DIR:?INSTALL_DIR is not set}"
echo "QMAKE_CMD: ${QMAKE_CMD:?QMAKE_CMD is not set}"
echo "LIBSERIALPORT_REPO: ${LIBSERIALPORT_REPO:?LIBSERIALPORT_REPO is not set}"
echo "LIBSIGROK_REPO: ${LIBSIGROK_REPO:?LIBSIGROK_REPO is not set}"

mkdir -p "${INSTALL_DIR}"

BUILD_DIR=./build
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# libserialport
git clone --depth=1 "${LIBSERIALPORT_REPO}" libserialport
cd libserialport
./autogen.sh
./configure --prefix="${INSTALL_DIR}"
make -j"$(nproc)"
make install
cd ..

# libsigrok
git clone --depth=1 -b "${LIBSIGROK_BRANCH:-master}" "${LIBSIGROK_REPO}" libsigrok
cd libsigrok
./autogen.sh
PKG_CONFIG_PATH="${INSTALL_DIR}/lib/pkgconfig" ./configure --prefix="${INSTALL_DIR}" --disable-java --disable-ruby
make -j"$(nproc)"
make install
cd ..
