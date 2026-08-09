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

echo "ALWAYS_SCREENSHOT: ${ALWAYS_SCREENSHOT:=false}"

FAILED=0

"/Applications/SmuView.app/Contents/MacOS/SmuView" --driver demo > stdout.log 2> stderr.log &
PID=$!
sleep 20

if ! kill -0 "${PID}" 2> /dev/null; then
	EXIT_CODE=0
	wait "${PID}" || EXIT_CODE=$?
	echo "::error::SmuView exited early with code ${EXIT_CODE}"
	FAILED=1
fi

TITLE=$(osascript -e 'tell application "System Events" to get title of front window of process "SmuView"' 2> /dev/null || echo "")
echo "Window title: '${TITLE}'"
if [[ "${FAILED}" -eq 0 && "${TITLE}" != "SmuView "* ]]; then
	echo "::error::Window title is wrong"
	FAILED=1
fi

if [[ "${FAILED}" -eq 1 || "${ALWAYS_SCREENSHOT}" == "true" ]]; then
	echo "Taking screenshot"
	screencapture -x desktop-screenshot-macos.png || true
fi

kill "${PID}" 2> /dev/null || true
wait "${PID}" 2> /dev/null || true

echo "--- stdout ---"
cat stdout.log 2> /dev/null || true
echo "--- stderr ---"
cat stderr.log 2> /dev/null || true

exit "${FAILED}"
