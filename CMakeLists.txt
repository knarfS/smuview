##
## This file is part of the SmuView project.
##
## Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
## Copyright (C) 2012-2013 Alexandru Gagniuc <mr.nuke.me@gmail.com>
## Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
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

cmake_minimum_required(VERSION 3.22)

project(smuview LANGUAGES CXX)

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")


#===============================================================================
#= User Options
#-------------------------------------------------------------------------------

option(DISABLE_WERROR "Build without -Werror" FALSE)
option(ENABLE_SIGNALS "Build with UNIX signals" TRUE)
option(ENABLE_TESTS "Enable unit tests" TRUE)
option(STATIC_PKGDEPS_LIBS "Statically link to (pkg-config) libraries" FALSE)

# Qt6 requires C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(WIN32)
	# On Windows/MinGW we need to statically link to libraries, and UNIX
	# signals are unsupported. Both options are user configurable, but they
	# are forced here regardless of what the user requested.
	message(STATUS "WIN32 build: forcing STATIC_PKGDEPS_LIBS=TRUE and ENABLE_SIGNALS=FALSE")
	set(STATIC_PKGDEPS_LIBS TRUE)
	set(ENABLE_SIGNALS FALSE)

	# When cross compiling this is needed for pkg-config
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
	"Choose the type of build (None, Debug, Release, RelWithDebInfo, MinSizeRel)."
	FORCE)
endif()

# Generate compile_commands.json in build/ for analyzers like clang-tidy.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "DISABLE_WERROR: ${DISABLE_WERROR}")
message(STATUS "ENABLE_SIGNALS: ${ENABLE_SIGNALS}")
message(STATUS "ENABLE_TESTS: ${ENABLE_TESTS}")
message(STATUS "STATIC_PKGDEPS_LIBS: ${STATIC_PKGDEPS_LIBS}")

#===============================================================================
#= Dependencies
#-------------------------------------------------------------------------------

list(APPEND PKGDEPS glib-2.0>=2.28.0)
list(APPEND PKGDEPS glibmm-2.4>=2.28.0)

set(LIBSR_CXX_BINDING "libsigrokcxx>=0.5.2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSRCXX REQUIRED IMPORTED_TARGET ${LIBSR_CXX_BINDING})
pkg_check_modules(PKGDEPS REQUIRED IMPORTED_TARGET ${PKGDEPS})

include(CheckSigrokFeatures)
if(STATIC_PKGDEPS_LIBS)
	check_libsigrok_features("${LIBSRCXX_STATIC_INCLUDE_DIRS}" PkgConfig::LIBSRCXX)
else()
	check_libsigrok_features("${LIBSRCXX_INCLUDE_DIRS}" PkgConfig::LIBSRCXX)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.0 QUIET COMPONENTS Core Gui Widgets Svg)
if(Qt6_FOUND)
	set(QT_VERSION_MAJOR 6)
	message(STATUS "Found Qt6: ${Qt6_VERSION}")
else()
	find_package(Qt5 5.7 REQUIRED COMPONENTS Core Gui Widgets Svg)
	set(QT_VERSION_MAJOR 5)
	message(STATUS "Found Qt5: ${Qt5_VERSION}")
endif()

set(QT_LIBRARIES
	Qt${QT_VERSION_MAJOR}::Gui
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Svg)

find_package(Qwt 6.1.2 REQUIRED)

# boost::multiprecision is required, but it's header only. So no need to specify
set(BOOSTCOMPS)
if(ENABLE_TESTS)
	list(APPEND BOOSTCOMPS unit_test_framework)
endif()
# CMP0167: CMake >= 3.30 has deprecated FindBoost(), but libboost >= 1.70 supplies a CMake config file
find_package(Boost 1.70 CONFIG REQUIRED COMPONENTS ${BOOSTCOMPS})

# Find the platform's thread library (needed for C++11 threads).
# This will set ${CMAKE_THREAD_LIBS_INIT} to the correct, OS-specific value.
find_package(Threads REQUIRED)

if(MINGW AND CMAKE_CROSSCOMPILING)
	# MXE workaround: FindPython cannot find the provided Python for cross
	# compiling, therefore we have to disable python discovery in pybind11 and
	# pass Python_ROOT_DIR to cmake.
	# We have to pass the python lib and the include directory to
	# target_link_libraries and target_link_libraries manually
	set(PYBIND11_NOPYTHON ON)

	if(NOT DEFINED Python_ROOT_DIR)
		message(FATAL_ERROR "Pass -DPython_ROOT_DIR=<path>/Python3")
	endif()

	file(GLOB MXE_PYTHON_EMBED_LIBRARY "${Python_ROOT_DIR}/libs/libpython*.a")
	if(NOT MXE_PYTHON_EMBED_LIBRARY)
		message(FATAL_ERROR "No libpythonXXX.a found under ${Python_ROOT_DIR}/libs")
	endif()

	if(NOT EXISTS "${Python_ROOT_DIR}/include/Python.h")
		message(FATAL_ERROR "No Python.h found in ${Python_ROOT_DIR}/include")
	endif()

	set(MXE_PYTHON_INCLUDE_DIR "${Python_ROOT_DIR}/include")
else()
	find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)
endif()

# SmuView has its own copy of pybind11
set(PYBIND11_SUBDIRECTORY external/pybind11)
add_subdirectory(${PYBIND11_SUBDIRECTORY})
# SmuView has its own copy of QCodeEditor
add_subdirectory(external/QCodeEditor)
# SmuView has its own copy of QtFindReplaceDialog
add_subdirectory(external/QtFindReplaceDialog/dialogs)

#===============================================================================
#= System Introspection
#-------------------------------------------------------------------------------

include(memaccess)
memaccess_check_unaligned_le(HAVE_UNALIGNED_LITTLE_ENDIAN_ACCESS)


#===============================================================================
#= Examin versions and replace in templates
#-------------------------------------------------------------------------------

set(SV_TITLE SmuView)
set(SV_VERSION_STRING "0.0.6")

# Append the revision hash unless we are exactly on a tagged release.
include(GetGitRevisionDescription)
git_describe(SV_TAG_VERSION_STRING --match "v${SV_VERSION_STRING}" --exact-match)
if(NOT SV_TAG_VERSION_STRING)
	get_git_head_revision(SV_REVSPEC SV_HASH)
	if(SV_HASH)
		string(SUBSTRING "${SV_HASH}" 0 7 SV_SHORTHASH)
		set(SV_VERSION_STRING "${SV_VERSION_STRING}-git-${SV_SHORTHASH}")
	endif()

	# Non-tagged releases use the continuous manual
	set(SV_MANUAL_VERSION "continuous")
else()
	# Tagged releases use a fixed manual version
	set(SV_MANUAL_VERSION ${SV_VERSION_STRING})
endif()

if(SV_VERSION_STRING MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-[-0-9a-z]*)?$")
	set(SV_VERSION_MAJOR ${CMAKE_MATCH_1})
	set(SV_VERSION_MINOR ${CMAKE_MATCH_2})
	set(SV_VERSION_MICRO ${CMAKE_MATCH_3})
	set(SV_VERSION_SUFFIX ${CMAKE_MATCH_4})
endif()

message(STATUS "${SV_TITLE} version: ${SV_VERSION_STRING}")

# Library versions
set(SV_GLIBMM_VERSION ${PKGDEPS_glibmm-2.4_VERSION})
get_directory_property(pybind11_VERSION DIRECTORY ${PYBIND11_SUBDIRECTORY} DEFINITION pybind11_VERSION)
get_directory_property(pybind11_VERSION_TYPE DIRECTORY ${PYBIND11_SUBDIRECTORY} DEFINITION pybind11_VERSION_TYPE)
set(SV_PYBIND11_VERSION "${pybind11_VERSION} ${pybind11_VERSION_TYPE}")
if(MINGW AND CMAKE_CROSSCOMPILING)
	# Get the Python version from patchlevel.h for MXE builds
	file(STRINGS "${MXE_PYTHON_INCLUDE_DIR}/patchlevel.h" PYTHON_VERSION_LINE REGEX "^#define PY_VERSION")
	string(REGEX REPLACE "^#define PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1" SV_PYTHON_VERSION "${PYTHON_VERSION_LINE}")
else()
	set(SV_PYTHON_VERSION ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
endif()

configure_file(
	src/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/src/config.h
)

configure_file(
	contrib/config_version.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/contrib/config_version.sh
)

# Generate DMG plist file
configure_file(
	packaging/macos/Info.plist.in
	${CMAKE_CURRENT_BINARY_DIR}/packaging/macos/Info.plist
	@ONLY
)

# Generate Windows installer script
configure_file(
	packaging/mxe/smuview_cross.nsi.in
	${CMAKE_CURRENT_BINARY_DIR}/packaging/mxe/smuview_cross.nsi
	@ONLY
)

#===============================================================================
#= Sources
#-------------------------------------------------------------------------------

set(smuview_SOURCES
	src/main.cpp
	src/application.cpp
	src/devicemanager.cpp
	src/mainwindow.cpp
	src/session.cpp
	src/settingsmanager.cpp
	src/util.cpp
	src/channels/addscchannel.cpp
	src/channels/basechannel.cpp
	src/channels/dividechannel.cpp
	src/channels/hardwarechannel.cpp
	src/channels/integratechannel.cpp
	src/channels/mathchannel.cpp
	src/channels/movingavgchannel.cpp
	src/channels/multiplysfchannel.cpp
	src/channels/multiplysschannel.cpp
	src/channels/userchannel.cpp
	src/data/analogbasesignal.cpp
	src/data/analogsamplesignal.cpp
	src/data/analogtimesignal.cpp
	src/data/basesignal.cpp
	src/data/datautil.cpp
	src/data/properties/baseproperty.cpp
	src/data/properties/boolproperty.cpp
	src/data/properties/doubleproperty.cpp
	src/data/properties/doublerangeproperty.cpp
	src/data/properties/int32property.cpp
	src/data/properties/measuredquantityproperty.cpp
	src/data/properties/rationalproperty.cpp
	src/data/properties/stringproperty.cpp
	src/data/properties/uint64property.cpp
	src/data/properties/uint64rangeproperty.cpp
	src/devices/basedevice.cpp
	src/devices/configurable.cpp
	src/devices/deviceutil.cpp
	src/devices/hardwaredevice.cpp
	src/devices/measurementdevice.cpp
	src/devices/oscilloscopedevice.cpp
	src/devices/sourcesinkdevice.cpp
	src/devices/userdevice.cpp

	src/python/bindings.cpp
	src/python/pystreambuf.cpp
	src/python/pystreamredirect.hpp
	src/python/smuscriptrunner.cpp
	src/python/uihelper.cpp
	src/python/uiproxy.cpp

	src/ui/data/quantitycombobox.cpp
	src/ui/data/quantityflagslist.cpp
	src/ui/data/unitcombobox.cpp
	src/ui/datatypes/basewidget.cpp
	src/ui/datatypes/boolbutton.cpp
	src/ui/datatypes/boolcheckbox.cpp
	src/ui/datatypes/boolled.cpp
	src/ui/datatypes/datatypehelper.cpp
	src/ui/datatypes/doublecontrol.cpp
	src/ui/datatypes/doubledisplay.cpp
	src/ui/datatypes/doubleknob.cpp
	src/ui/datatypes/doublerangecombobox.cpp
	src/ui/datatypes/doubleslider.cpp
	src/ui/datatypes/doublesmallcontrol.cpp
	src/ui/datatypes/doublespinbox.cpp
	src/ui/datatypes/int32spinbox.cpp
	src/ui/datatypes/measuredquantitycombobox.cpp
	src/ui/datatypes/rationalcombobox.cpp
	src/ui/datatypes/stringcombobox.cpp
	src/ui/datatypes/stringlabel.cpp
	src/ui/datatypes/stringled.cpp
	src/ui/datatypes/thresholdcontrol.cpp
	src/ui/datatypes/uint64combobox.cpp
	src/ui/datatypes/uint64label.cpp
	src/ui/datatypes/uint64rangecombobox.cpp
	src/ui/datatypes/uint64spinbox.cpp
	src/ui/devices/channelcombobox.cpp
	src/ui/devices/channelgroupcombobox.cpp
	src/ui/devices/configkeycombobox.cpp
	src/ui/devices/configurablecombobox.cpp
	src/ui/devices/devicecombobox.cpp
	src/ui/devices/selectconfigurableform.cpp
	src/ui/devices/selectpropertyform.cpp
	src/ui/devices/selectsignalwidget.cpp
	src/ui/devices/signalcombobox.cpp
	src/ui/devices/devicetree/devicetreemodel.cpp
	src/ui/devices/devicetree/devicetreeview.cpp
	src/ui/devices/devicetree/treeitem.cpp
	src/ui/dialogs/aboutdialog.cpp
	src/ui/dialogs/addmathchanneldialog.cpp
	src/ui/dialogs/adduserchanneldialog.cpp
	src/ui/dialogs/addviewdialog.cpp
	src/ui/dialogs/addviewdialog.cpp
	src/ui/dialogs/connectdialog.cpp
	src/ui/dialogs/generatewaveformdialog.cpp
	src/ui/dialogs/plotconfigdialog.cpp
	src/ui/dialogs/plotcurveconfigdialog.cpp
	src/ui/dialogs/plotdiffmarkerdialog.cpp
	src/ui/dialogs/selectsignaldialog.cpp
	src/ui/dialogs/selectxysignalsdialog.cpp
	src/ui/dialogs/signalsavedialog.cpp
	src/ui/tabs/basetab.cpp
	src/ui/tabs/devicetab.cpp
	src/ui/tabs/measurementtab.cpp
	src/ui/tabs/oscilloscopetab.cpp
	src/ui/tabs/smuscripttab.cpp
	src/ui/tabs/sourcesinktab.cpp
	src/ui/tabs/tabdockwidget.cpp
	src/ui/tabs/tabhelper.cpp
	src/ui/tabs/usertab.cpp
	src/ui/tabs/welcometab.cpp
	src/ui/views/baseplotview.cpp
	src/ui/views/baseview.cpp
	src/ui/views/dataview.cpp
	src/ui/views/devicesview.cpp
	src/ui/views/democontrolview.cpp
	src/ui/views/genericcontrolview.cpp
	src/ui/views/measurementcontrolview.cpp
	src/ui/views/powerpanelview.cpp
	src/ui/views/scopehorizontalcontrolview.cpp
	src/ui/views/scopetriggercontrolview.cpp
	src/ui/views/scopeverticalcontrolview.cpp
	src/ui/views/sequenceoutputview.cpp
	src/ui/views/smuscriptoutputview.cpp
	src/ui/views/smuscripttreeview.cpp
	src/ui/views/smuscriptview.cpp
	src/ui/views/sourcesinkcontrolview.cpp
	src/ui/views/timeplotview.cpp
	src/ui/views/valuepanelview.cpp
	src/ui/views/viewhelper.cpp
	src/ui/views/xyplotview.cpp
	src/ui/widgets/clickablelabel.cpp
	src/ui/widgets/colorbutton.cpp
	src/ui/widgets/monofontdisplay.cpp
	src/ui/widgets/popup.cpp
	src/ui/widgets/plot/axislocklabel.cpp
	src/ui/widgets/plot/axispopup.cpp
	src/ui/widgets/plot/basecurvedata.cpp
	src/ui/widgets/plot/curve.cpp
	src/ui/widgets/plot/plot.cpp
	src/ui/widgets/plot/plotmagnifier.cpp
	src/ui/widgets/plot/plotscalepicker.cpp
	src/ui/widgets/plot/timecurvedata.cpp
	src/ui/widgets/plot/xycurvedata.cpp
)

if(ENABLE_SIGNALS)
	list(APPEND smuview_SOURCES src/signalhandler.cpp)
endif()

set(smuview_RESOURCES
	resources/smuview.qrc
)

if(WIN32)
	# Use the sigrok icon for the smuview.exe executable.
	enable_language(RC)
	set(CMAKE_RC_COMPILE_OBJECT "${CMAKE_RC_COMPILER} -O coff -I${CMAKE_CURRENT_SOURCE_DIR} <SOURCE> <OBJECT>")
	list(APPEND smuview_SOURCES packaging/mxe/smuviewico.rc)
endif()

if(Qt6_FOUND)
	qt6_add_resources(smuview_RESOURCES_RCC ${smuview_RESOURCES})
else()
	qt5_add_resources(smuview_RESOURCES_RCC ${smuview_RESOURCES})
endif()

#===============================================================================
#= Global Definitions
#-------------------------------------------------------------------------------

set(smuview_COMPILE_DEFINITIONS
	QT_NO_KEYWORDS
	__STDC_LIMIT_MACROS
	BOOST_MATH_DISABLE_FLOAT128=1
)

set(smuview_COMPILE_OPTIONS
	-Wall
	-Wextra
	-Woverloaded-virtual
	-Wdeprecated-declarations
	# -Weffc++
	# -Wconversion
	# -Wsign-conversion
)

if(NOT DISABLE_WERROR)
	list(APPEND smuview_COMPILE_OPTIONS
		-Werror=pedantic
		-pedantic-errors
	)
endif()

if(ENABLE_SIGNALS)
	list(APPEND smuview_COMPILE_DEFINITIONS ENABLE_SIGNALS)
endif()

if(MINGW AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Fix error "too many sections (37653)" and "file too big" for mingw/MXE when building for Debug
	list(APPEND smuview_COMPILE_OPTIONS -Wa,-mbig-obj)
endif()

#===============================================================================
#= Global Include Directories
#-------------------------------------------------------------------------------

set(smuview_INCLUDE_DIRS
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QWT_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

if(MINGW AND CMAKE_CROSSCOMPILING)
	# Set the include directory manually for MXE builds
	list(APPEND smuview_INCLUDE_DIRS ${MXE_PYTHON_INCLUDE_DIR})
endif()

if(STATIC_PKGDEPS_LIBS)
	list(APPEND smuview_INCLUDE_DIRS
		${LIBSRCXX_STATIC_INCLUDE_DIRS}
		${PKGDEPS_STATIC_INCLUDE_DIRS}
	)
else()
	list(APPEND smuview_INCLUDE_DIRS
		${LIBSRCXX_INCLUDE_DIRS}
		${PKGDEPS_INCLUDE_DIRS}
	)
endif()


#===============================================================================
#= Linker Configuration
#-------------------------------------------------------------------------------

set(smuview_LINK_DIRS ${Boost_LIBRARY_DIRS})

set(smuview_LINK_LIBS
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
	${QWT_LIBRARY}
	Threads::Threads
	PkgConfig::LIBSRCXX
	PkgConfig::PKGDEPS
	pybind11::embed
	QCodeEditor
	QtFindReplaceDialog
)

if(MINGW AND CMAKE_CROSSCOMPILING)
	# Set the link library manually for MXE builds
	list(APPEND smuview_LINK_LIBS "${MXE_PYTHON_EMBED_LIBRARY}")
endif()

add_executable(${PROJECT_NAME} ${smuview_SOURCES} ${smuview_RESOURCES_RCC})

target_include_directories(${PROJECT_NAME} PRIVATE ${smuview_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${smuview_COMPILE_DEFINITIONS})
target_compile_options(${PROJECT_NAME} PRIVATE ${smuview_COMPILE_OPTIONS})
target_link_directories(${PROJECT_NAME} PRIVATE ${smuview_LINK_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${smuview_LINK_LIBS})

if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Pass -mwindows so that no "DOS box" opens when SmuView is started.
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
endif()


#===============================================================================
#= Installation
#-------------------------------------------------------------------------------

# Install the executable
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install license, readme, etc.
install(FILES COPYING DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/smuview/)
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/smuview/)
install(FILES INSTALL DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/smuview/)
install(FILES NEWS DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/smuview/)

# Install the SmuScript examples
install(DIRECTORY smuscript/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/smuview/smuscript)

# All Unix (Linux, *BSD, Macos)
if(UNIX)
	# Install the manpage.
	install(FILES packaging/linux/smuview.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)
endif()

# Only Linux
if(UNIX AND NOT APPLE)
	# Install the desktop file
	install(FILES packaging/linux/org.sigrok.SmuView.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

	# Install the AppData/AppStream file
	install(FILES packaging/linux/org.sigrok.SmuView.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)

	# Install the SmuView icons
	install(FILES resources/icons/smuview.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps)
	install(FILES resources/icons/smuview.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
endif()


#===============================================================================
#= Documentation
#-------------------------------------------------------------------------------

add_subdirectory(manual)


#===============================================================================
#= Packaging (handled by CPack)
#-------------------------------------------------------------------------------


#===============================================================================
#= Tests
#-------------------------------------------------------------------------------

if(ENABLE_TESTS)
	add_subdirectory(test)
	enable_testing()
	add_test(test ${CMAKE_CURRENT_BINARY_DIR}/test/smuview-test)
endif()
