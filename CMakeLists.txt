CMAKE_MINIMUM_REQUIRED(VERSION 4.1.2 FATAL_ERROR)
SET(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
PROJECT(HoloTable CXX C)

# Default runtime output directory: put binaries into build/bin for consistency if not already set
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

ADD_COMPILE_OPTIONS(
								-Wall
								-Wextra
								-Wpedantic
								-pedantic
								-O0
								-g
)


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

SET(CMAKE_CXX_SCAN_FOR_MODULES ON)
ADD_LIBRARY(game_lib SHARED)
SET_TARGET_PROPERTIES(game_lib PROPERTIES CXX_MODULE_STD ON)
TARGET_SOURCES(game_lib
	PUBLIC
		FILE_SET game_mxx TYPE CXX_MODULES FILES
			${CMAKE_CURRENT_SOURCE_DIR}/game_lib/game.cppm
			${CMAKE_CURRENT_SOURCE_DIR}/game_lib/renderer.cppm
)

ADD_EXECUTABLE(holotable ${CMAKE_CURRENT_SOURCE_DIR}/graphical_client/main.cpp)
SET_TARGET_PROPERTIES(holotable PROPERTIES CXX_MODULE_STD ON)
TARGET_LINK_LIBRARIES(holotable PRIVATE RGFW game_lib)

ADD_EXECUTABLE(tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp)
SET_TARGET_PROPERTIES(tests PROPERTIES CXX_MODULE_STD ON)

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

function(_copy_dir_to_target target srcdir destdirname)
	if(COPY_RUNTIME_RESOURCES AND EXISTS "${srcdir}")
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${target}>/${destdirname}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/${destdirname}"
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${srcdir}" "$<TARGET_FILE_DIR:${target}>/${destdirname}"
			COMMENT "Copying ${srcdir} -> $<TARGET_FILE_DIR:${target}>/${destdirname}"
		)
	endif()
endfunction()

_copy_dir_to_target(holotable "${RT_ASSETS_SRC}" assets)
_copy_dir_to_target(holotable "${RT_SHADERS_SRC}" shaders)
_copy_dir_to_target(holotable "${RT_SCRIPTS_SRC}" scripts)

# Optionally do same for tests so test binaries can access test assets if needed
_copy_dir_to_target(tests "${RT_ASSETS_SRC}" assets)
_copy_dir_to_target(tests "${RT_SHADERS_SRC}" shaders)
_copy_dir_to_target(tests "${RT_SCRIPTS_SRC}" scripts)

# Install targets and resource directories
install(TARGETS RGFW
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS game_lib
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS holotable
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(EXISTS "${RT_ASSETS_SRC}")
	# Install the `assets` directory into the exec dir (EX: bin/assets)
	install(DIRECTORY ${RT_ASSETS_SRC} DESTINATION ${CMAKE_INSTALL_BINDIR})
	if(INSTALL_DATA_IN_DATADIR)
		install(DIRECTORY ${RT_ASSETS_SRC} DESTINATION ${CMAKE_INSTALL_DATADIR}/holotable/assets)
	endif()
endif()

if(EXISTS "${RT_SHADERS_SRC}")
	# Install the `shaders` directory into the exec dir (EX: bin/shaders)
	install(DIRECTORY ${RT_SHADERS_SRC} DESTINATION ${CMAKE_INSTALL_BINDIR})
	if(INSTALL_DATA_IN_DATADIR)
		install(DIRECTORY ${RT_SHADERS_SRC} DESTINATION ${CMAKE_INSTALL_DATADIR}/holotable/shaders)
	endif()
endif()

if(EXISTS "${RT_SCRIPTS_SRC}")
	# Install the `scripts` directory into the exec dir (EX: bin/scripts)
	install(DIRECTORY ${RT_SCRIPTS_SRC} DESTINATION ${CMAKE_INSTALL_BINDIR})
	if(INSTALL_DATA_IN_DATADIR)
		install(DIRECTORY ${RT_SCRIPTS_SRC} DESTINATION ${CMAKE_INSTALL_DATADIR}/holotable/scripts)
	endif()
endif()

# End resource copy & install rules
