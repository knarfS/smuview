cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---------------------------------------------------------

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------

set(QtFindReplaceDialog_SHARED_HEADERS
	"finddialog.h"
	"findform.h"
	"findreplacedialog.h"
	"findreplaceform.h"
	"findreplace_global.h"
)

set(QtFindReplaceDialog_SOURCES
	"finddialog.cpp"
	"findform.cpp"
	"findreplacedialog.cpp"
	"findreplacedialog.ui"
	"findreplaceform.cpp"
	"findreplaceform.ui"
	${QtFindReplaceDialog_SHARED_HEADERS}
)

if(MSVC)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall")
endif()

# ---------------------------------------------------------

add_library(QtFindReplaceDialog STATIC ${QtFindReplaceDialog_SOURCES})
set_target_properties(QtFindReplaceDialog PROPERTIES PUBLIC_HEADER "${QtFindReplaceDialog_SHARED_HEADERS}")
target_compile_definitions(QtFindReplaceDialog PUBLIC "-DFINDREPLACESHARED_EXPORT=")
target_compile_definitions(QtFindReplaceDialog PRIVATE "-DFINDREPLACE_LIBRARY")
target_compile_options(QtFindReplaceDialog PUBLIC ${MY_COMPILE_OPTIONS})
target_link_libraries(QtFindReplaceDialog PRIVATE Qt5::Widgets)

target_include_directories(QtFindReplaceDialog SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:QtFindReplaceDialog/include>  # <prefix>/QtFindReplaceDialog/include
)
