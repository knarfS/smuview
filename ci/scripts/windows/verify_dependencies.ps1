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

$depsExe = (Get-Command Dependencies.exe).Source
$anyMissing = $false

Get-ChildItem "$env:ProgramFiles\SmuView" -Recurse -Include *.exe,*.dll | ForEach-Object {
	$output = & $depsExe -chain -depth=1 "$($_.FullName)" 2>$null
	$missingLines = $output | Select-String -Pattern "NOT_FOUND"
	if ($missingLines) {
		Write-Output "::error::$($_.Name) has unresolved dependencies:"
		$missingLines | ForEach-Object { Write-Output "  $_" }
		$anyMissing = $true
	}
	Write-Output "$($_.Name) scanned"
}

if ($anyMissing) {
	exit 1
} else {
	Write-Output "No unresolved DLL imports found."
}
