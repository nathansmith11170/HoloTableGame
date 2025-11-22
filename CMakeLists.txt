CMAKE_MINIMUM_REQUIRED(VERSION 3.28)
PROJECT(HoloTable CXX C)

if(MSVC)
	string(APPEND CMAKE_CXX_FLAGS " /D_HAS_EXCEPTIONS=0 /EHs-c- /GR-")
  add_compile_options(
    /W4
    /permissive-
  )

  add_compile_options(
    "$<$<CONFIG:Debug>:/Od>"
    "$<$<CONFIG:Debug>:/Zi>"
    "$<$<CONFIG:Release>:/O2>"
    "$<$<CONFIG:RelWithDebInfo>:/O2>" 
  )
else()
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -pedantic
  )

	string(APPEND CMAKE_CXX_FLAGS " -fno-exceptions -fno-rtti")

  add_compile_options(
    "$<$<CONFIG:Debug>:-O0>"
    "$<$<CONFIG:Debug>:-g>"
    "$<$<CONFIG:Release>:-O2>"
    "$<$<CONFIG:RelWithDebInfo>:-O2>"
  )
endif()
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

#FIND_PACKAGE(Vulkan REQUIRED)
#INCLUDE_DIRECTORIES(SYSTEM ${Vulkan_INCLUDE_DIRS})

ADD_LIBRARY(RGFW SHARED ${CMAKE_CURRENT_SOURCE_DIR}/graphical_client/rgfw.c)
SET_TARGET_PROPERTIES(RGFW PROPERTIES LINKER_LANGUAGE C)
#TARGET_LINK_LIBRARIES(RGFW PUBLIC ${Vulkan_LIBRARIES})
TARGET_COMPILE_DEFINITIONS(RGFW PRIVATE RGFW_EXPORT RGFW_NO_API RGFW_IMPLEMENTATION)
IF (WIN32)
	target_link_libraries(RGFW PUBLIC gdi32 user32)
ELSE()
	MESSAGE(STATUS "Platform: Unix-like. Using X11.")
	FIND_PACKAGE(X11 REQUIRED)
	FIND_PACKAGE(PkgConfig REQUIRED)
	PKG_CHECK_MODULES(XRANDR REQUIRED xrandr)
	TARGET_LINK_LIBRARIES(RGFW PRIVATE ${XRANDR_LIBRARIES})
	TARGET_INCLUDE_DIRECTORIES(RGFW PRIVATE ${XRANDR_INCLUDE_DIRS})
	IF(X11_FOUND)
		MESSAGE(STATUS "Configuring RGFW with X11 support.")
		TARGET_COMPILE_DEFINITIONS(RGFW PUBLIC RGFW_X11 RGFW_NO_WAYLAND)
		TARGET_LINK_LIBRARIES(RGFW PRIVATE ${X11_LIBRARIES})
	ELSE()
		MESSAGE(FATAL_ERROR "X11 not found.")
	ENDIF()
ENDIF()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    message(STATUS "Clang-Tidy found: ${CLANG_TIDY_EXE}")
    
    set(CLANG_TIDY_CHECKS "-*,bugprone-*,modernize-*,performance-*")
    
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS}")
else()
    message(WARNING "Clang-Tidy not found! Code analysis will be disabled.")
endif()

SET(CMAKE_CXX_SCAN_FOR_MODULES ON)

ADD_LIBRARY(game SHARED)
SET_TARGET_PROPERTIES(game
	PROPERTIES
		WINDOWS_EXPORT_ALL_SYMBOLS ON
)
TARGET_SOURCES(game
	PUBLIC
		FILE_SET game_mxx TYPE CXX_MODULES FILES
			${CMAKE_CURRENT_SOURCE_DIR}/game_lib/game.cppm
			${CMAKE_CURRENT_SOURCE_DIR}/game_lib/renderer.cppm
)

ADD_EXECUTABLE(holotable ${CMAKE_CURRENT_SOURCE_DIR}/graphical_client/main.cpp)
target_compile_definitions(holotable PRIVATE RGFW_IMPORT)
TARGET_LINK_LIBRARIES(holotable PRIVATE RGFW game)

ADD_EXECUTABLE(tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp)

# ---- Resource copy & install rules ----
include(GNUInstallDirs)

option(COPY_RUNTIME_RESOURCES "Copy runtime resource directories (assets/shaders/scripts) into the build output and install" ON)
option(INSTALL_DATA_IN_DATADIR "Also install resources into ${CMAKE_INSTALL_DATADIR}/holotable in addition to the executable location" OFF)

# Default runtime output directory for town: put binaries into build/bin for consistency if not already set
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

set(RT_ASSETS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/graphical_client/assets)
set(RT_SHADERS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/graphical_client/shaders)
set(RT_SCRIPTS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/game_lib/scripts)

# ---- SVG to PNG conversion using msdfgen (reworked to produce separate Ninja steps) ----
set(RT_STAMPS_DIR ${CMAKE_BINARY_DIR}/stamps)
file(MAKE_DIRECTORY ${RT_STAMPS_DIR})

function(_create_copy_target target srcdir destdirname)
	if(COPY_RUNTIME_RESOURCES AND EXISTS "${srcdir}")
		set(STAMP ${RT_STAMPS_DIR}/copy_${target}_${destdirname}.stamp)
		add_custom_command(
			OUTPUT ${STAMP}
			COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${target}>/${destdirname}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/${destdirname}"
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${srcdir}" "$<TARGET_FILE_DIR:${target}>/${destdirname}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${RT_STAMPS_DIR}"
			COMMAND ${CMAKE_COMMAND} -E touch ${STAMP}
			COMMENT "Copying ${srcdir} -> $<TARGET_FILE_DIR:${target}>/${destdirname}"
			VERBATIM
		)
		add_custom_target(copy_${target}_${destdirname} DEPENDS ${STAMP})
		add_dependencies(${target} copy_${target}_${destdirname})
	endif()
endfunction()

function(_create_convert_svgs_target target)
	if(EXISTS "${RT_ASSETS_SRC}")
		file(GLOB SVG_FILES "${RT_ASSETS_SRC}/*.svg")
		set(STAMPS)
		foreach(SVG_FILE ${SVG_FILES})
			get_filename_component(SVG_BASENAME ${SVG_FILE} NAME_WE)
			set(STAMP ${RT_STAMPS_DIR}/convert_${target}_${SVG_BASENAME}.stamp)
			add_custom_command(
				OUTPUT ${STAMP}
				COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/assets"
				COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/third_party/msdfgen.exe" msdf -svg "${SVG_FILE}" -autoframe -o "$<TARGET_FILE_DIR:${target}>/assets/${SVG_BASENAME}.png" -size 64 64 -pxrange 8
				COMMAND ${CMAKE_COMMAND} -E touch ${STAMP}
				COMMENT "Converting ${SVG_BASENAME}.svg -> ${SVG_BASENAME}.png for ${target}"
				VERBATIM
			)
			list(APPEND STAMPS ${STAMP})
		endforeach()
		if(STAMPS)
			add_custom_target(convert_svgs_${target} DEPENDS ${STAMPS})
			# Ensure conversion happens after the copy step so output files aren't blocked
			if(TARGET copy_${target}_assets)
				add_dependencies(convert_svgs_${target} copy_${target}_assets)
			endif()
			add_dependencies(${target} convert_svgs_${target})
		endif()
	endif()
endfunction()

function(_create_remove_svgs_target target)
	if(EXISTS "${RT_ASSETS_SRC}")
		file(GLOB SVG_FILES "${RT_ASSETS_SRC}/*.svg")
		set(STAMP ${RT_STAMPS_DIR}/remove_svgs_${target}.stamp)
		# populate the command list by assembling remove commands for each SVG
		set(_cmds)
		foreach(SVG_FILE ${SVG_FILES})
			get_filename_component(SVG_NAME ${SVG_FILE} NAME)
			list(APPEND _cmds "COMMAND" "${CMAKE_COMMAND}" "-E" "remove" "$<TARGET_FILE_DIR:${target}>/assets/${SVG_NAME}")
		endforeach()
		list(APPEND _cmds "COMMAND" "${CMAKE_COMMAND}" "-E" "touch" "${STAMP}")
		# Now create a custom command using the assembled command list
		add_custom_command(
			OUTPUT ${STAMP}
			${_cmds}
			COMMENT "Removing SVG files from output assets for ${target}"
			VERBATIM
		)
		add_custom_target(remove_svgs_${target} DEPENDS ${STAMP})
		# Ensure removal happens after copy and conversion
		if(TARGET convert_svgs_${target})
			add_dependencies(remove_svgs_${target} convert_svgs_${target})
		endif()
		if(TARGET copy_${target}_assets)
			add_dependencies(remove_svgs_${target} copy_${target}_assets)
		endif()
		add_dependencies(${target} remove_svgs_${target})
	endif()
endfunction()

# Replace previous per-target copy/convert/remove calls with the new separate-step targets
_create_copy_target(holotable "${RT_ASSETS_SRC}" assets)
_create_copy_target(holotable "${RT_SHADERS_SRC}" shaders)
_create_copy_target(holotable "${RT_SCRIPTS_SRC}" scripts)
_create_convert_svgs_target(holotable)
_create_remove_svgs_target(holotable)
# End of reworked resource steps

