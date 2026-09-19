#!/usr/bin/env bash
##
## This file is part of the SmuView project.
##
## Fetches Python.framework from python.org's official macOS installer and
## places it at a fixed, known location. This single copy is used for BOTH
## compiling SmuView (Python_ROOT_DIR points here) AND for bundling into the
## DMG later (build_dmg.sh copies this same directory) - there is exactly
## one Python involved end to end, no Homebrew, matching how
## ci/scripts/mxe/install_python.sh works for the Windows build.
##

set -euo pipefail

echo "INSTALL_DIR: ${INSTALL_DIR:?INSTALL_DIR is not set}"
echo "PYTHON_ORG_VERSION: ${PYTHON_VERSION:?PYTHON_VERSION is not set (e.g. 3.14.3)}"

PYTHON_DL_VERSION="${PYTHON_VERSION%.*}"   # e.g. "3.14" from "3.14.3"

PYTHON_PKG_FILE="python-${PYTHON_VERSION}-macos11.pkg"
PYTHON_PKG_URL="https://www.python.org/ftp/python/${PYTHON_VERSION}/${PYTHON_PKG_FILE}"
WORKDIR=$(mktemp -d)
trap 'rm -rf "${WORKDIR}"' EXIT

echo "Downloading ${PYTHON_PKG_URL} ..."
curl -fL --retry 3 -o "${WORKDIR}/${PYTHON_PKG_FILE}" "${PYTHON_PKG_URL}"

# A .pkg is a flat archive; expand it fully without installing anything
# system-wide (no sudo, nothing written outside WORKDIR).
echo "Expanding ${PYTHON_PKG_FILE} ..."
pkgutil --expand-full "${WORKDIR}/${PYTHON_PKG_FILE}" "${WORKDIR}/expanded"

echo "Debug"
find "${WORKDIR}/expanded" -type d

# The framework lives inside one of the pkg's component packages; the exact
# component folder name has varied across CPython releases, so locate it
# by content instead of hard-coding the component name.
PYTHON_FRAMEWORK_SRC=$(find "${WORKDIR}/expanded" -type d -path "*/Python.framework" -maxdepth 6 | head -n1)
if [[ -z "${PYTHON_FRAMEWORK_SRC}" ]]; then
    echo "ERROR: could not find Python.framework inside the expanded .pkg" >&2
    exit 1
fi

# Keep this copy pristine (Headers, bin, include, docs all intact) - it's
# used to compile against. build_dmg.sh makes its own trimmed copy for
# the actual app bundle later; it does not need to modify this one.
DEST_DIR="${INSTALL_DIR}/Python3"
mkdir -p "${DEST_DIR}"
cp -R "${PYTHON_FRAMEWORK_SRC}" "${DEST_DIR}/"
chmod -R u+w "${DEST_DIR}/Python.framework"

echo "Python ${PYTHON_VERSION} installed at: ${DEST_DIR}/Python.framework"
echo "Python_ROOT_DIR for CMake: ${DEST_DIR}/Python.framework/Versions/${PYTHON_DL_VERSION}"
