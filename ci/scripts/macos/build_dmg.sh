#!/usr/bin/env bash
##
## This file is part of the SmuView project.
##
## Copyright (C) 2017 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2021-2026 Frank Stettner <frank-stettner@gmx.net>
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
echo "BREW_QT_VERSION: ${BREW_QT_VERSION:?BREW_QT_VERSION is not set}"
echo "BREW_PYTHON_VERSION: ${BREW_PYTHON_VERSION:?BREW_PYTHON_VERSION is not set}"

# Path to Qt binaries.
QT_BIN_DIR=$(brew list "${BREW_QT_VERSION}" | grep bin | head -n 1 | xargs dirname)

# Path to Python 3 framework.
PYTHON_FRAMEWORK_DIR=$(brew list "${BREW_PYTHON_VERSION}" | grep Python.framework/Python | head -n 1 | xargs dirname)
PYTHON_PREFIX_DIR=$(brew --prefix "${BREW_PYTHON_VERSION}")

# Get Python version
PYTHON_VERSION=$(python3 -c 'import sys; print(".".join(map(str, sys.version_info[0:2])))')

DMG_BUILD_DIR=./build_dmg
mkdir "${DMG_BUILD_DIR}"
cd "${DMG_BUILD_DIR}"

CONTENTS_DIR="${SV_TITLE}.app/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
FRAMEWORKS_DIR="${CONTENTS_DIR}/Frameworks"
SHARE_DIR="${CONTENTS_DIR}/share"
PYTHON_DIR="${FRAMEWORKS_DIR}/Python.framework/Versions/${PYTHON_VERSION}"

mkdir -p "${MACOS_DIR}" "${FRAMEWORKS_DIR}" "${SHARE_DIR}"

# Copy executable
cp "${INSTALL_DIR}/bin/${SV_BIN_NAME}" "${MACOS_DIR}"
# Copy smuscripts
cp -R "${INSTALL_DIR}"/share/smuview/smuscript "${SHARE_DIR}"

"${QT_BIN_DIR}"/macdeployqt "${SV_TITLE}".app -verbose=1

# Copy Python framework and fix it up.
cp -R "${PYTHON_FRAMEWORK_DIR}" "${FRAMEWORKS_DIR}"
chmod 644 "${PYTHON_DIR}"/lib/libpython*.dylib
rm -rf "${PYTHON_DIR}"/Headers
rm -rf "${PYTHON_DIR}"/bin
rm -rf "${PYTHON_DIR}"/include
rm -rf "${PYTHON_DIR}"/share
rm -rf "${PYTHON_DIR}"/lib/pkgconfig
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/lib2to3
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/distutils
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/idlelib
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/test
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/**/test
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/tkinter
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/turtledemo
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/unittest
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/__pycache__
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/**/__pycache__
rm -rf "${PYTHON_DIR}"/lib/python"${PYTHON_VERSION}"/**/**/__pycache__
rm -rf "${PYTHON_DIR}"/Resources

# Replace paths
install_name_tool -change \
	"${PYTHON_PREFIX_DIR}"/Frameworks/Python.framework/Versions/"${PYTHON_VERSION}"/Python \
	@executable_path/../Frameworks/Python.framework/Versions/"${PYTHON_VERSION}"/Python \
	"${MACOS_DIR}/${SV_BIN_NAME}"

# Add SmuView wrapper (sets PYTHONHOME).
mv "${MACOS_DIR}/${SV_BIN_NAME}" "${MACOS_DIR}/${SV_BIN_NAME}.real"
cat > "${MACOS_DIR}/${SV_BIN_NAME}" << EOF
#!/bin/sh

DIR="\$(dirname "\$0")"
cd "\$DIR"
export PYTHONHOME="../Frameworks/Python.framework/Versions/"${PYTHON_VERSION}""
exec "./${SV_BIN_NAME}.real" "\$@"
EOF
chmod 755 "${MACOS_DIR}/${SV_BIN_NAME}"

cp ../smuview/build/packaging/macos/Info.plist "${CONTENTS_DIR}"
cp ../smuview/packaging/macos/smuview.icns "${CONTENTS_DIR}"/Resources

hdiutil create "${SV_TITLE}-${SV_VERSION_STRING}.dmg" \
	-volname "$SV_TITLE $SV_VERSION_STRING" \
	-fs HFS+ -srcfolder "$SV_TITLE.app"

# Move DMG to parent directory, so it is accessible without knowing ${DMG_BUILD_DIR}
mv "${SV_TITLE}-${SV_VERSION_STRING}.dmg" ..
