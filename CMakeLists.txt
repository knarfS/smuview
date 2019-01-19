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

# Let AUTOMOC and AUTOUIC process GENERATED files.
if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

# Only interpret if() arguments as variables or keywords when unquoted.
if(POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)
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
#= Documentation
#-------------------------------------------------------------------------------

add_subdirectory(manual)


#===============================================================================
#= Dependencies
#-------------------------------------------------------------------------------

list(APPEND PKGDEPS glib-2.0>=2.28.0)
list(APPEND PKGDEPS glibmm-2.4>=2.28.0)

set(LIBSR_CXX_BINDING "libsigrokcxx>=0.5.1")
list(APPEND PKGDEPS "${LIBSR_CXX_BINDING}")

find_package(PkgConfig)
pkg_check_modules(LIBSRCXX ${LIBSR_CXX_BINDING})
if(NOT LIBSRCXX_FOUND OR NOT LIBSRCXX_VERSION)
	message(FATAL_ERROR "libsigrok C++ bindings missing, check libsigrok's 'configure' output (missing dependencies?)")
endif()
pkg_check_modules(PKGDEPS REQUIRED ${PKGDEPS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)

if(WIN32)
	# MXE workaround: Use pkg-config to find Qt5 libs.
	# https://github.com/mxe/mxe/issues/1642
	# Not required (and doesn't work) on MSYS2.
	if(NOT DEFINED ENV{MSYSTEM})
		pkg_check_modules(QT5ALL REQUIRED Qt5Widgets Qt5Gui Qt5Svg)
	endif()
endif()

set(QT_LIBRARIES Qt5::Gui Qt5::Widgets Qt5::Svg)

find_package(Qwt 6.1.2 REQUIRED)

# Only boost::config and boost::multiprecision are required, so no need to
# specify any boost libraries
find_package(Boost 1.55 REQUIRED)

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
  src/channels/multiplysfchannel.cpp
  src/channels/multiplysschannel.cpp
  src/channels/userchannel.cpp
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
  src/devices/properties/doubleproperty.cpp
  src/devices/properties/int32property.cpp
  src/devices/properties/measuredquantityproperty.cpp
  src/devices/properties/stringproperty.cpp
  src/devices/properties/uint64property.cpp

  src/ui/data/quantitycombobox.cpp
  src/ui/data/quantityflagslist.cpp
  src/ui/data/unitcombobox.cpp
  src/ui/datatypes/basewidget.cpp
  src/ui/datatypes/boolbutton.cpp
  src/ui/datatypes/boolcheckbox.cpp
  src/ui/datatypes/boolled.cpp
  src/ui/datatypes/datatypehelper.cpp
  src/ui/datatypes/doublecontrol.cpp
  src/ui/datatypes/doubleknob.cpp
  src/ui/datatypes/doublelcddisplay.cpp
  src/ui/datatypes/doublespinbox.cpp
  src/ui/datatypes/int32spinbox.cpp
  src/ui/datatypes/measuredquantitycombobox.cpp
  src/ui/datatypes/stringcombobox.cpp
  src/ui/datatypes/thresholdcontrol.cpp
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
  src/ui/devices/signaltree.cpp
  src/ui/dialogs/aboutdialog.cpp
  src/ui/dialogs/addmathchanneldialog.cpp
  src/ui/dialogs/adduserchanneldialog.cpp
  src/ui/dialogs/addviewdialog.cpp
  src/ui/dialogs/addviewdialog.cpp
  src/ui/dialogs/connectdialog.cpp
  src/ui/dialogs/plotconfigdialog.cpp
  src/ui/dialogs/plotcurveconfigdialog.cpp
  src/ui/dialogs/plotdiffmarkerdialog.cpp
  src/ui/dialogs/savedialog.cpp
  src/ui/dialogs/selectsignaldialog.cpp
  src/ui/processing/processingwidget.cpp
  src/ui/processing/processthreadwidget.cpp
  src/ui/processing/blocks/sequencesinblock.cpp
  src/ui/tabs/basetab.cpp
  src/ui/tabs/devicetab.cpp
  src/ui/tabs/measurementtab.cpp
  src/ui/tabs/tabhelper.cpp
  src/ui/tabs/sourcesinktab.cpp
  src/ui/tabs/virtualtab.cpp
  src/ui/tabs/welcometab.cpp
  src/ui/views/baseview.cpp
  src/ui/views/dataview.cpp
  src/ui/views/demodmmcontrolview.cpp
  src/ui/views/measurementcontrolview.cpp
  src/ui/views/plotview.cpp
  src/ui/views/powerpanelview.cpp
  src/ui/views/sourcesinkcontrolview.cpp
  src/ui/views/valuepanelview.cpp
  src/ui/views/viewhelper.cpp
  src/ui/widgets/clickablelabel.cpp
  src/ui/widgets/colorbutton.cpp
  src/ui/widgets/lcddisplay.cpp
  src/ui/widgets/popup.cpp
  src/ui/widgets/plot/axislocklabel.cpp
  src/ui/widgets/plot/axispopup.cpp
  src/ui/widgets/plot/basecurvedata.cpp
  src/ui/widgets/plot/plot.cpp
  src/ui/widgets/plot/plotscalepicker.cpp
  src/ui/widgets/plot/timecurvedata.cpp
  src/ui/widgets/plot/xycurvedata.cpp
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

# Install the executable.
install(TARGETS ${PROJECT_NAME} DESTINATION bin/)

# Install the manpage.
#install(FILES doc/smuview.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)

# Install the desktop file.
install(FILES contrib/org.sigrok.SmuView.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

# Install the AppData/AppStream file.
install(FILES contrib/org.sigrok.SmuView.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)

# Install the SmuView icons.
install(FILES icons/smuview.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps)
install(FILES icons/smuview.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)

# Generate Windows installer script.
#configure_file(contrib/smuview_cross.nsi.in contrib/smuview_cross.nsi @ONLY)


#===============================================================================
#= Packaging (handled by CPack)
#-------------------------------------------------------------------------------


#===============================================================================
#= Tests
#-------------------------------------------------------------------------------
