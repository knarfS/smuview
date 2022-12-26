##
## This file is part of the SmuView project.
##
## Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
## Copyright (C) 2012 Alexandru Gagniuc <mr.nuke.me@gmail.com>
## Copyright (C) 2022 Frank Stettner <frank-stettner@gmx.net>
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

# On MinGW we need to use static linking.
if(NOT WIN32)
	add_definitions(-DBOOST_TEST_DYN_LINK)
endif()

add_executable(smuview-test
	${smuview_TEST_SOURCES}
)

target_link_libraries(smuview-test ${SMUVIEW_LINK_LIBS})
