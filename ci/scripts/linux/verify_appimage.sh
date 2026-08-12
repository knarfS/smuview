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

echo "ALWAYS_SCREENSHOT: ${ALWAYS_SCREENSHOT:=false}"

xvfb-run -a bash <<'EOF'

set -euo pipefail

FAILED=0
EXIT_CODE=0
PID=""

./SmuView.AppImage --driver demo > stdout.log 2> stderr.log &
PID=$!
sleep 20

if ! kill -0 "${PID}" 2> /dev/null; then
	wait "${PID}" || EXIT_CODE=$?
	echo "::error::SmuView exited early with code ${EXIT_CODE}"
	FAILED=1
fi

if [[ "${FAILED}" -eq 0 ]]; then
	TITLE=""
	for _ in {1..20}; do
		WINDOW_ID="$(xdotool search --onlyvisible --pid "${PID}" 2>/dev/null | head -n1 || true)"
		if [[ -n "${WINDOW_ID}" ]]; then
			TITLE="$(xdotool getwindowname "${WINDOW_ID}" 2>/dev/null || true)"
			break
		fi
		sleep 1
	done

	echo "Window title: '${TITLE}'"

	if [[ "${TITLE}" != SmuView\ * ]]; then
		echo "::error::Window title is wrong"
		FAILED=1
	fi
fi

if [[ "${FAILED}" -eq 1 || "${ALWAYS_SCREENSHOT}" == "true" ]]; then
	echo "Taking screenshot"
	import -window root desktop-screenshot-linux.png || true
fi

kill "${PID}" 2> /dev/null || true
wait "${PID}" 2> /dev/null || true

echo "--- stdout ---"
cat stdout.log 2> /dev/null || true
echo "--- stderr ---"
cat stderr.log 2> /dev/null || true

exit "${FAILED}"
EOF
