##
## This file is part of the SmuView project.
##
## Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
## Copyright (C) 2012-2013 Alexandru Gagniuc <mr.nuke.me@gmail.com>
## Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

cmake_minimum_required(VERSION 2.8.12)

include(GNUInstallDirs)

project(smuview)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")


#===============================================================================
#= User Options
#-------------------------------------------------------------------------------

option(DISABLE_WERROR "Build without -Werror" FALSE)
option(ENABLE_SIGNALS "Build with UNIX signals" TRUE)
option(ENABLE_TESTS "Enable unit tests" TRUE)
option(STATIC_PKGDEPS_LIBS "Statically link to (pkg-config) libraries" FALSE)

if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

if(WIN32)
	# On Windows/MinGW we need to statically link to libraries.
	# This option is user configurable, but enable it by default on win32.
	set(STATIC_PKGDEPS_LIBS TRUE)

	# Windows does not support UNIX signals.
	set(ENABLE_SIGNALS FALSE)
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
	"Choose the type of build (None, Debug, Release, RelWithDebInfo, MinSizeRel)."
	FORCE)
endif()


#===============================================================================
#= Dependencies
#-------------------------------------------------------------------------------

list(APPEND PKGDEPS glib-2.0>=2.28.0)
# TODO: glibmm must be >=2.52.0 until we find a solution for
#       SR_CONF_MEASURED_QUANTITY in configurable.cpp
#list(APPEND PKGDEPS glibmm-2.4>=2.28.0)
list(APPEND PKGDEPS glibmm-2.4>=2.52.0)

list(APPEND PKGDEPS libsigrokcxx>=0.6.0)

find_package(PkgConfig)
pkg_check_modules(PKGDEPS REQUIRED ${PKGDEPS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)

if(WIN32)
	# MXE workaround: Use pkg-config to find Qt5 libs.
	# https://github.com/mxe/mxe/issues/1642
	pkg_check_modules(QT5ALL REQUIRED Qt5Widgets Qt5Gui Qt5Svg)
endif()

set(QT_LIBRARIES Qt5::Gui Qt5::Widgets Qt5::Svg)

find_package(Qwt REQUIRED)

set(BOOSTCOMPS filesystem serialization system)
if(ENABLE_TESTS)
	list(APPEND BOOSTCOMPS unit_test_framework)
endif()
find_package(Boost 1.55 COMPONENTS ${BOOSTCOMPS} REQUIRED)

# Find the platform's thread library (needed for C++11 threads).
# This will set ${CMAKE_THREAD_LIBS_INIT} to the correct, OS-specific value.
find_package(Threads REQUIRED)


#===============================================================================
#= System Introspection
#-------------------------------------------------------------------------------


#===============================================================================
#= Config Header
#-------------------------------------------------------------------------------

set(SV_TITLE SmuView)
set(SV_VERSION_STRING "0.0.1")

set(SV_GLIBMM_VERSION ${PKGDEPS_glibmm-2.4_VERSION})

# Append the revision hash unless we are exactly on a tagged release.
#include(GetGitRevisionDescription)
#git_describe(SV_TAG_VERSION_STRING --match "smuview-${SV_VERSION_STRING}" --exact-match)
#if(NOT SV_TAG_VERSION_STRING)
#	get_git_head_revision(SV_REVSPEC SV_HASH)
#	if(SV_HASH)
#		string(SUBSTRING "${SV_HASH}" 0 7 SV_SHORTHASH)
#		set(SV_VERSION_STRING "${SV_VERSION_STRING}-git-${SV_SHORTHASH}")
#	endif()
#endif()

if(SV_VERSION_STRING MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-[-0-9a-z]*)?$")
	set(SV_VERSION_MAJOR ${CMAKE_MATCH_1})
	set(SV_VERSION_MINOR ${CMAKE_MATCH_2})
	set(SV_VERSION_MICRO ${CMAKE_MATCH_3})
	set(SV_VERSION_SUFFIX ${CMAKE_MATCH_4})
endif()

message("-- ${SV_TITLE} version: ${SV_VERSION_STRING}")

configure_file (
	${PROJECT_SOURCE_DIR}/config.h.in
	${PROJECT_BINARY_DIR}/config.h
)


#===============================================================================
#= Sources
#-------------------------------------------------------------------------------

set(smuview_SOURCES
  main.cpp
  src/application.cpp
  src/devicemanager.cpp
  src/mainwindow.cpp
  src/session.cpp
  src/util.cpp
  src/channels/basechannel.cpp
  src/channels/dividechannel.cpp
  src/channels/hardwarechannel.cpp
  src/channels/integratechannel.cpp
  src/channels/mathchannel.cpp
  src/channels/multiplysfchannel.cpp
  src/channels/multiplysschannel.cpp
  src/data/analogsignal.cpp
  src/data/basesignal.cpp
  src/data/datautil.cpp
  src/devices/basedevice.cpp
  src/devices/configurable.cpp
  src/devices/deviceutil.cpp
  src/devices/hardwaredevice.cpp
  src/devices/measurementdevice.cpp
  src/devices/sourcesinkdevice.cpp
  src/devices/virtualdevice.cpp
  src/devices/properties/baseproperty.cpp
  src/devices/properties/boolproperty.cpp
  src/devices/properties/floatproperty.cpp
  src/devices/properties/int32property.cpp
  src/devices/properties/measuredquantityproperty.cpp
  src/devices/properties/stringproperty.cpp
  src/devices/properties/uint64property.cpp
  src/dialogs/addmathchanneldialog.cpp
  src/dialogs/addviewdialog.cpp
  src/dialogs/aboutdialog.cpp
  src/dialogs/addviewdialog.cpp
  src/dialogs/connectdialog.cpp
  src/dialogs/plotconfigdialog.cpp
  src/dialogs/plotcurveconfigdialog.cpp
  src/dialogs/savedialog.cpp
  src/dialogs/selectsignaldialog.cpp
  src/tabs/basetab.cpp
  src/tabs/devicetab.cpp
  src/tabs/measurementtab.cpp
  src/tabs/sourcesinktab.cpp
  src/tabs/virtualtab.cpp
  src/tabs/welcometab.cpp
  src/views/baseview.cpp
  src/views/dataview.cpp
  src/views/demodmmcontrolview.cpp
  src/views/measurementcontrolview.cpp
  src/views/plotview.cpp
  src/views/powerpanelview.cpp
  src/views/sinkcontrolview.cpp
  src/views/sourcecontrolview.cpp
  src/views/valuepanelview.cpp
  src/widgets/channelgroupcombobox.cpp
  src/widgets/colorbutton.cpp
  src/widgets/configkeycombobox.cpp
  src/widgets/configurablecombobox.cpp
  src/widgets/controlbutton.cpp
  src/widgets/devicecombobox.cpp
  src/widgets/lcddisplay.cpp
  src/widgets/led.cpp
  src/widgets/optionalvaluecontrol.cpp
  src/widgets/popup.cpp
  src/widgets/quantitycombobox.cpp
  src/widgets/quantityflagslist.cpp
  src/widgets/signaltree.cpp
  src/widgets/unitcombobox.cpp
  src/widgets/valuecontrol.cpp
  src/widgets/plot/axispopup.cpp
  src/widgets/plot/basecurve.cpp
  src/widgets/plot/plot.cpp
  src/widgets/plot/plotscalepicker.cpp
  src/widgets/plot/timecurve.cpp
  src/widgets/plot/xycurve.cpp

  src/ui/datatypes/boolbutton.cpp
  src/ui/datatypes/boolcheckbox.cpp
  src/ui/datatypes/datatypehelper.cpp
  src/ui/datatypes/floatspinbox.cpp
  src/ui/datatypes/int32spinbox.cpp
  src/ui/datatypes/stringcombobox.cpp
  src/ui/datatypes/uint64spinbox.cpp
  src/ui/processing/processingwidget.cpp
  src/ui/processing/processthreadwidget.cpp
  src/ui/processing/blocks/sequencesinblock.cpp
)

if(ENABLE_SIGNALS)
	list(APPEND smuview_SOURCES signalhandler.cpp)
endif()

set(smuview_RESOURCES
	smuview.qrc
)

if(WIN32)
	# Use the sigrok icon for the smuview.exe executable.
	set(CMAKE_RC_COMPILE_OBJECT "${CMAKE_RC_COMPILER} -O coff -I${CMAKE_CURRENT_SOURCE_DIR} <SOURCE> <OBJECT>")
	enable_language(RC)
	list(APPEND smuview_SOURCES smuviewico.rc)
endif()

qt5_add_resources(smuview_RESOURCES_RCC ${smuview_RESOURCES})


#===============================================================================
#= Global Definitions
#-------------------------------------------------------------------------------

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-Wall -Wextra -Woverloaded-virtual) # -Weffc++ -Wconversion -Wsign-conversion)
add_definitions(-std=c++11)
add_definitions(-DBOOST_MATH_DISABLE_FLOAT128=1)

if(NOT DISABLE_WERROR)
	add_definitions(-Werror)
endif()

if(ENABLE_SIGNALS)
	add_definitions(-DENABLE_SIGNALS)
endif()


#===============================================================================
#= Global Include Directories
#-------------------------------------------------------------------------------

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QWT_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

if(STATIC_PKGDEPS_LIBS)
	include_directories(${PKGDEPS_STATIC_INCLUDE_DIRS})
else()
	include_directories(${PKGDEPS_INCLUDE_DIRS})
endif()


#===============================================================================
#= Linker Configuration
#-------------------------------------------------------------------------------

link_directories(${Boost_LIBRARY_DIRS})

set(SMUVIEW_LINK_LIBS
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
	${QWT_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
#	${LIBATOMIC_LIBRARY}
)

if(STATIC_PKGDEPS_LIBS)
	link_directories(${PKGDEPS_STATIC_LIBRARY_DIRS})
	list(APPEND SMUVIEW_LINK_LIBS ${PKGDEPS_STATIC_LDFLAGS})
else()
	link_directories(${PKGDEPS_LIBRARY_DIRS})
	list(APPEND SMUVIEW_LINK_LIBS ${PKGDEPS_LIBRARIES})
endif()

if(WIN32)
	# On Windows we need to statically link the libqsvg imageformat
	# plugin (and the QtSvg component) for SVG graphics/icons to work.
	# We also need QWindowsIntegrationPlugin, Qt5PlatformSupport, and all
	# Qt libs and their dependencies.
	add_definitions(-DQT_STATICPLUGIN)
	list(APPEND SMUVIEW_LINK_LIBS Qt5::QSvgPlugin)
	list(APPEND SMUVIEW_LINK_LIBS Qt5::QWindowsIntegrationPlugin)
	list(APPEND SMUVIEW_LINK_LIBS -lQt5PlatformSupport ${QT5ALL_LDFLAGS})

	# Form Qt 5.8 on, Qt5PlatformSupport is split into several plugins:
	# QtAccessibilitySupport QtCliboardSupport QtEventDispatcherSupport
	# QtFontDatabaseSupport QtGraphicsSupport QtThemeSupport (<- some are wrong)
	#list(APPEND SMUVIEW_LINK_LIBS -lQt5AccessibilitySupport -lQt5EventDispatcherSupport)
	#list(APPEND SMUVIEW_LINK_LIBS -lQt5FontDatabaseSupport -lQt5ThemeSupport)
	#list(APPEND SMUVIEW_LINK_LIBS -lQt5ClipboardSupport -lQt5GraphicsSupport)
	#list(APPEND SMUVIEW_LINK_LIBS ${QT5ALL_LDFLAGS})
endif()

add_executable(${PROJECT_NAME} ${smuview_SOURCES} ${smuview_RESOURCES_RCC})

target_link_libraries(${PROJECT_NAME} ${SMUVIEW_LINK_LIBS})

if(WIN32 AND NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	# Pass -mwindows so that no "DOS box" opens when SmuView is started.
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
endif()


#===============================================================================
#= Installation
#-------------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME} DESTINATION bin/)


#===============================================================================
#= Packaging (handled by CPack)
#-------------------------------------------------------------------------------


#===============================================================================
#= Tests
#-------------------------------------------------------------------------------
