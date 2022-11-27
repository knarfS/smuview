##
## This file is part of the SmuView project.
##
## Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

# Check if the installed libsigrok has the required features.
function(check_libsigrok_features additional_header additional_lib)
  include(CheckCXXSourceCompiles)
  include(CMakePushCheckState)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_FLAGS "-std=c++17")
  set(CMAKE_REQUIRED_INCLUDES "${additional_header}")
  set(CMAKE_REQUIRED_LIBRARIES "${additional_lib}")
  set(CMAKE_REQUIRED_QUIET 1)
  check_cxx_source_compiles("
#include <libsigrokcxx/libsigrokcxx.hpp>
const sigrok::ConfigKey *config_key;
int main() {
	config_key = sigrok::ConfigKey::MULTIPLEXER;
	return 0;
}
" HAS_FEATURES)
  cmake_pop_check_state()

  if (NOT HAS_FEATURES)
    message(FATAL_ERROR "libsigrok is too old, minimum required version is 0.6.0-git-522381a3")
  endif()
endfunction(check_libsigrok_features)
