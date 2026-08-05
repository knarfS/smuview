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
echo "APP_DIR: ${APP_DIR:?APP_DIR is not set}"
echo "ARCH: ${ARCH:?ARCH is not set}"
echo "SV_VERSION_STRING ${SV_VERSION_STRING:?SV_VERSION_STRING is not set}"

# Download linuxdeploy
mkdir -p downloads
cd downloads
wget --no-verbose https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-"${ARCH}".AppImage
chmod +x linuxdeploy-"${ARCH}".AppImage
wget --no-verbose https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-"${ARCH}".AppImage
chmod +x linuxdeploy-plugin-qt-"${ARCH}".AppImage
cd ..

# Copy the python stdlib and strip bloat.
mkdir -p "${APP_DIR}"/usr/lib/python3.10
cp -a /usr/lib/python3.10/. "${APP_DIR}"/usr/lib/python3.10/
find "${APP_DIR}"/usr/lib/python3.10 \( -name "test" -o -name "idle_test" -o -name "__pycache__" \) -exec rm -rf {} +

# Generate python hook
mkdir -p "${APP_DIR}"/apprun-hooks
cat > "${APP_DIR}"/apprun-hooks/smuview-python-hook.sh <<\EOF
# AppDir/apprun-hooks/smuview-python-hook.sh
export PYTHONHOME="$this_dir/usr"
export PYTHONPATH="$this_dir/usr/lib/python3.10"
export PYTHONDONTWRITEBYTECODE=1   # avoid __pycache__ writes into a read-only-ish AppDir
export LD_LIBRARY_PATH="$this_dir/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
EOF

export LINUXDEPLOY_OUTPUT_VERSION="${SV_VERSION_STRING}"
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${INSTALL_DIR}/lib:/usr/local/lib" ./downloads/linuxdeploy-"${ARCH}".AppImage \
	--appdir "${APP_DIR}" \
	--output appimage \
	--plugin qt
