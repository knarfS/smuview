##
## This file is part of the SmuView project.
##
## Copyright (C) 2025-2026 Frank Stettner <frank-stettner@gmx.net>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

include(FindPackageHandleStandardArgs)

# Determin which Qt major version is used, since Qwt's package/library
# naming differs between Qt5 and Qt6 builds.
if(NOT QT_VERSION_MAJOR)
	if(TARGET Qt6::Core)
		set(QT_VERSION_MAJOR 6)
	elseif(TARGET Qt5::Core)
		set(QT_VERSION_MAJOR 5)
	else()
		message(WARNING "FindQwt: Qt5::Core / Qt6::Core target not found! "
			"Call find_package(Qt6/Qt5) BEFORE find_package(Qwt). "
			"Falling back to an unordered search which may pick the wrong Qwt build.")
	endif()
endif()

set(QWT_PATH_SUFFIXES qwt6-qt${QT_VERSION_MAJOR} qwt-qt${QT_VERSION_MAJOR} qt${QT_VERSION_MAJOR}/qwt6 qt${QT_VERSION_MAJOR}/qwt)
set(QWT_LIBRARY_NAMES qwt6-qt${QT_VERSION_MAJOR} qwt-qt${QT_VERSION_MAJOR})
# Qt-version-less fallbacks
list(APPEND QWT_PATH_SUFFIXES qwt qwt6)
list(APPEND QWT_LIBRARY_NAMES qwt qwt6)

# Try to find Qwt
find_path(QWT_INCLUDE_DIR NAMES qwt.h
  PATH_SUFFIXES include lib/qwt.framework/Headers ${QWT_PATH_SUFFIXES})
find_library(QWT_LIBRARY NAMES ${QWT_LIBRARY_NAMES}
  PATH_SUFFIXES lib ${QWT_PATH_SUFFIXES})

# Get version
if(QWT_INCLUDE_DIR AND EXISTS "${QWT_INCLUDE_DIR}/qwt_global.h")
	file(READ "${QWT_INCLUDE_DIR}/qwt_global.h" qwt_header)
	string(REGEX REPLACE ".*QWT_VERSION_STR +\"([^\"]+)\".*" "\\1" QWT_VERSION_STR "${qwt_header}")
endif()

find_package_handle_standard_args(Qwt
	REQUIRED_VARS QWT_LIBRARY QWT_INCLUDE_DIR
	VERSION_VAR QWT_VERSION_STR)

if(QWT_FOUND)
	mark_as_advanced(QWT_LIBRARY)
	mark_as_advanced(QWT_INCLUDE_DIR)
endif()
