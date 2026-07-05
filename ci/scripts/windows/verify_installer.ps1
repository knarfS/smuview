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

$failed = $false

$exe = "$env:ProgramFiles\SmuView\smuview.exe"
$proc = Start-Process -FilePath $exe -ArgumentList "--driver demo" -PassThru `
	-RedirectStandardOutput "stdout.log" `
	-RedirectStandardError "stderr.log"
Start-Sleep -Seconds 10
$proc.Refresh()

Write-Output "HasExited: $($proc.HasExited)"
if ($proc.HasExited) {
	Write-Output "::error::SmuView exited early with code $($proc.ExitCode)"
	$failed = $true
}

Write-Output "MainWindowTitle: '$($proc.MainWindowTitle)'"
Write-Output "MainWindowHandle: $($proc.MainWindowHandle)"
if (!$failed -and !$proc.MainWindowTitle.StartsWith("SmuView ")) {
	Write-Output "::error::Window title is wrong, taking screenshot"

	Add-Type -AssemblyName System.Windows.Forms,System.Drawing
	$bounds = [System.Windows.Forms.SystemInformation]::VirtualScreen
	$bmp = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
	$graphics = [System.Drawing.Graphics]::FromImage($bmp)
	$graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
	$bmp.Save("$env:GITHUB_WORKSPACE\desktop-screenshot-windows.png")

	$failed = $true
}

Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue

Write-Output "--- stdout ---"
Get-Content stdout.log -ErrorAction SilentlyContinue
Write-Output "--- stderr ---"
Get-Content stderr.log -ErrorAction SilentlyContinue

if ($failed) {
	Write-Error "SmuView verification failed"
	exit 1
}
