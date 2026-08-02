##
## This file is part of the SmuView project.
##
## Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
## Copyright (C) 2012 Alexandru Gagniuc <mr.nuke.me@gmail.com>
## Copyright (C) 2022-2026 Frank Stettner <frank-stettner@gmx.net>
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

set(smuview_TEST_SOURCES
	${PROJECT_SOURCE_DIR}/src/util.cpp
	test.cpp
	util.cpp
)

# NOTE: smuview_COMPILE_DEFINITIONS, smuview_INCLUDE_DIRS,
# smuview_COMPILE_OPTIONS, smuview_LINK_DIRS and smuview_LINK_LIBS are set in
# the top-level CMakeLists.txt and are reused here so the test binary is built
#with the same defines/includes/warnings as SmuView itself.
set(smuview_TEST_COMPILE_DEFINITIONS ${smuview_COMPILE_DEFINITIONS})

# On Windows/MinGW, Boost is linked statically (forced for WIN32 builds
# in the top-level CMakeLists.txt). Everywhere else Boost.Test is used
# as a shared library, which needs this define.
if(NOT WIN32)
	list(APPEND smuview_TEST_COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK)
endif()

add_executable(smuview-test ${smuview_TEST_SOURCES})

target_include_directories(smuview-test PRIVATE ${smuview_INCLUDE_DIRS})
target_compile_definitions(smuview-test PRIVATE ${smuview_TEST_COMPILE_DEFINITIONS})
target_compile_options(smuview-test PRIVATE ${smuview_COMPILE_OPTIONS})
target_link_directories(smuview-test PRIVATE ${MyAppv_LINK_DIRS})
target_link_libraries(smuview-test PRIVATE ${smuview_LINK_LIBS})
