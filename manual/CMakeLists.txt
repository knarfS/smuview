##
## This file is part of the SmuView project.
##
## Copyright (C) 2018 Gerhard Sittig <gerhard.sittig@gmx.net>
## Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

cmake_minimum_required(VERSION 2.8.12)

# External dependencies, required and optional tools.
find_program(ASCIIDOCTOR_EXECUTABLE NAMES asciidoctor)
find_program(ASCIIDOCTOR_PDF_EXECUTABLE NAMES asciidoctor-pdf)

# Tunables.
set(STYLES_DIR "asciidoctor-stylesheet-factory/stylesheets")
set(STYLE_SHEET "readthedocs.css")

# Input files.
set(MANUAL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/manual.adoc")

# Output files, conversion results.
set(MANUAL_OUT_HTML "${CMAKE_CURRENT_BINARY_DIR}/manual.html")
set(MANUAL_OUT_PDF "${CMAKE_CURRENT_BINARY_DIR}/manual.pdf")

# Manual related make(1) targets.
add_custom_target(manual-html
	COMMAND ${ASCIIDOCTOR_EXECUTABLE}
		-a stylesheet=${STYLE_SHEET}
		-a stylesdir=${CMAKE_CURRENT_SOURCE_DIR}/${STYLES_DIR}
		-a toc=left
		-a sv_version=${SV_VERSION_STRING}
		-a sv_manual_version=${SV_MANUAL_VERSION}
		--destination-dir=${CMAKE_CURRENT_BINARY_DIR}
		${MANUAL_SRC}
	BYPRODUCTS ${MANUAL_OUT_HTML}
	DEPENDS ${MANUAL_SRC}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating manual, HTML output"
)
if (ASCIIDOCTOR_PDF_EXECUTABLE)
	add_custom_target(manual-pdf
		COMMAND ${ASCIIDOCTOR_PDF_EXECUTABLE}
			-a stylesheet=${STYLE_SHEET}
			-a stylesdir=${CMAKE_CURRENT_SOURCE_DIR}/${STYLES_DIR}
			-a sv_version=${SV_VERSION_STRING}
			-a sv_manual_version=${SV_MANUAL_VERSION}
			--destination-dir=${CMAKE_CURRENT_BINARY_DIR}
			${MANUAL_SRC}
		BYPRODUCTS ${MANUAL_OUT_PDF}
		DEPENDS ${MANUAL_SRC}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating manual, PDF output"
	)
else ()
	add_custom_target(manual-pdf
		COMMAND ${CMAKE_COMMAND} -E echo
			"asciidoctor-pdf executable is missing, NOT generating PDF output"
		DEPENDS ${MANUAL_SRC}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
endif ()
add_custom_target(manual)
add_dependencies(manual manual-html manual-pdf)


#===============================================================================
#= Installation
#-------------------------------------------------------------------------------

set(MANUAL_INST_SUBDIR "share/doc/smuview")
install(
	FILES ${MANUAL_OUT_HTML} ${MANUAL_OUT_PDF}
	DESTINATION ${MANUAL_INST_SUBDIR}
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	OPTIONAL
)
if (ASCIIDOCTOR_EXECUTABLE)
	install(
		DIRECTORY images
		DESTINATION ${MANUAL_INST_SUBDIR}
		FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
		PATTERN "*.xcf" EXCLUDE
	)
endif ()


#===============================================================================
#= Publish
#-------------------------------------------------------------------------------

set(MANUAL_PUP_DIR "${CMAKE_BINARY_DIR}/manual_publish/${SV_MANUAL_VERSION}")
add_custom_target(manual-publish)
add_custom_command(TARGET manual-publish PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${MANUAL_PUP_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_OUT_HTML} ${MANUAL_PUP_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_OUT_PDF} ${MANUAL_PUP_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${MANUAL_PUP_DIR}/images
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/images/*.png ${MANUAL_PUP_DIR}/images
	COMMAND ${CMAKE_COMMAND} -E make_directory ${MANUAL_PUP_DIR}/images/icons
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/images/icons/*.png ${MANUAL_PUP_DIR}/images/icons
	COMMAND ${CMAKE_COMMAND} -E make_directory ${MANUAL_PUP_DIR}/images/numbers
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/images/numbers/*.png ${MANUAL_PUP_DIR}/images/numbers)
