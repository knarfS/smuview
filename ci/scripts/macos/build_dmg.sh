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

set -euo pipefail

echo "INSTALL_DIR: ${INSTALL_DIR:?INSTALL_DIR is not set}"
echo "BREW_QT_VERSION: ${BREW_QT_VERSION:?BREW_QT_VERSION is not set}"
echo "PYTHON_VERSION: ${PYTHON_VERSION:?PYTHON_VERSION is not set}"
echo "SV_TITLE: ${SV_TITLE:?SV_TITLE is not set}"
echo "SV_BIN_NAME: ${SV_BIN_NAME:?SV_BIN_NAME is not set}"
echo "SV_VERSION_STRING: ${SV_VERSION_STRING:?SV_VERSION_STRING is not set}"

# Path to Qt binaries
QT_PREFIX_DIR=$(brew --prefix "${BREW_QT_VERSION}")
QT_BIN_DIR="${QT_PREFIX_DIR}/bin"
# Path to Python 3 framework
PYTHON_PREFIX_DIR="${INSTALL_DIR}/Frameworks/Python.framework"
# Remove patch part from version
PYTHON_ABI_VERSION="${PYTHON_VERSION%.*}"

DMG_BUILD_DIR=./build_dmg
mkdir -p "${DMG_BUILD_DIR}"
cd "${DMG_BUILD_DIR}"

CONTENTS_DIR="${SV_TITLE}.app/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
FRAMEWORKS_DIR="${CONTENTS_DIR}/Frameworks"
SHARE_DIR="${CONTENTS_DIR}/share"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"
PYTHON_DIR="${FRAMEWORKS_DIR}/Python.framework/Versions/${PYTHON_ABI_VERSION}"

mkdir -p "${MACOS_DIR}" "${FRAMEWORKS_DIR}" "${SHARE_DIR}" "${RESOURCES_DIR}"

# Copy executable
cp "${INSTALL_DIR}/bin/${SV_BIN_NAME}" "${MACOS_DIR}"
# Copy smuscripts
cp -R "${INSTALL_DIR}"/share/smuview/smuscript "${SHARE_DIR}"
# Copy DMG specific files
cp ../smuview/build/packaging/macos/Info.plist "${CONTENTS_DIR}"
cp ../smuview/packaging/macos/smuview.icns "${RESOURCES_DIR}"

"${QT_BIN_DIR}"/macdeployqt "${SV_TITLE}".app -verbose=1

# Copy Python framework
cp -R "${PYTHON_PREFIX_DIR}" "${FRAMEWORKS_DIR}"

# Remove stuff we don't want/need or that breaks signing (site-packages)
rm -rf "${FRAMEWORKS_DIR}/Python.framework/Headers"
rm -rf "${PYTHON_DIR}/Headers"
rm -rf "${PYTHON_DIR}/bin"
rm -rf "${PYTHON_DIR}/include"
rm -rf "${PYTHON_DIR}/share"
rm -rf "${PYTHON_DIR}/Frameworks"
rm -rf "${PYTHON_DIR}/Resources/Python.app"
rm -rf "${PYTHON_DIR}/lib/pkgconfig"
rm -rf "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}/idlelib"
rm -rf "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}/tkinter"
rm -rf "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}/turtledemo"
rm -rf "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}/unittest"
rm -rf "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}/site-packages"
rm -rf "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}/lib-dynload/_tkinter"*.so
find "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}" -depth -name test -type d -exec rm -rf {} +
find "${PYTHON_DIR}/lib/python${PYTHON_ABI_VERSION}" -depth -name __pycache__ -type d -exec rm -rf {} +

# Fix smuview binary's reference to Python
PYTHON_OLD_PATH="/Library/Frameworks/Python.framework/Versions/${PYTHON_ABI_VERSION}/Python"
PYTHON_NEW_PATH="@executable_path/../Frameworks/Python.framework/Versions/${PYTHON_ABI_VERSION}/Python"
install_name_tool -change "${PYTHON_OLD_PATH}" "${PYTHON_NEW_PATH}" "${MACOS_DIR}/${SV_BIN_NAME}"

OLD_LIB_PREFIX="/Library/Frameworks/Python.framework/Versions/${PYTHON_ABI_VERSION}/lib/"
NEW_LIB_PREFIX="@executable_path/../Frameworks/Python.framework/Versions/${PYTHON_ABI_VERSION}/lib/"
while IFS= read -r -d '' file; do
	# A file's own self-id (dylibs only, not .so bundles) can share the
	# same prefix as a real dependency, exclude it from fixing the references.
	self_ids="$(otool -D "${file}" 2>/dev/null | grep -v ':$' || true)"
	while read -r old_ref _; do
		grep -qxF "${old_ref}" <<< "${self_ids}" && continue
		new_ref="${NEW_LIB_PREFIX}${old_ref#"${OLD_LIB_PREFIX}"}"
		echo "  Fixing ${file}: ${old_ref} -> ${new_ref}"
		install_name_tool -change "${old_ref}" "${new_ref}" "${file}"
	done < <(otool -L "${file}" | grep -F "${OLD_LIB_PREFIX}" | sort -u || true)
done < <(find "${PYTHON_DIR}/lib" \( -name "*.dylib" -o -name "*.so" \) ! -type l -print0)

# Add SmuView wrapper (sets PYTHONHOME)
mv "${MACOS_DIR}/${SV_BIN_NAME}" "${MACOS_DIR}/${SV_BIN_NAME}.real"
cat > "${MACOS_DIR}/${SV_BIN_NAME}" << EOF
#!/bin/sh
DIR="\$(dirname "\$0")"
cd "\${DIR}"
unset PYTHONPATH
export PYTHONHOME="../Frameworks/Python.framework/Versions/${PYTHON_ABI_VERSION}"
exec "./${SV_BIN_NAME}.real" "\$@"
EOF
chmod 755 "${MACOS_DIR}/${SV_BIN_NAME}"

# Ad-hoc signing
# Sign nested files
find "${SV_TITLE}.app" -type f \( -perm -u+x -o -name "*.dylib" -o -name "*.so" \) -print0 |
while IFS= read -r -d '' bin; do
	if file "${bin}" | grep -q "Mach-O"; then
		codesign --force --sign - --timestamp=none "${bin}"
	fi
done
# Sign the Python framework bundle
codesign --force --sign - --timestamp=none "${FRAMEWORKS_DIR}/Python.framework"
# Sign SmuView
codesign --force --sign - --timestamp=none "${SV_TITLE}.app"
# Verify signing
codesign --verify --deep --strict --verbose=2 "${SV_TITLE}.app"

# Creat DMG
hdiutil create \
	-volname "${SV_TITLE} ${SV_VERSION_STRING}" \
	-srcfolder "${SV_TITLE}.app" \
	-fs APFS -format UDZO \
	-ov \
	"${SV_TITLE}-${SV_VERSION_STRING}.dmg"

# Move DMG to parent directory, so it is accessible without knowing ${DMG_BUILD_DIR}
mv "${SV_TITLE}-${SV_VERSION_STRING}.dmg" ..
