CMAKE_MINIMUM_REQUIRED(VERSION 4.1.2 FATAL_ERROR)
SET(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_SCAN_FOR_MODULES ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
PROJECT(HoloTable CXX)

ADD_COMPILE_OPTIONS(
								-stdlib=libc++
								-Weverything
								-Wno-weak-vtables
								-Wno-covered-switch-default
								-Wno-missing-prototypes
								-Wno-unsafe-buffer-usage
								-Wno-padded
								-Wno-c++98-compat
								-Wno-c++98-compat-pedantic
								-Wno-pre-c++20-compat
								-O0
								-g
)

INCLUDE_DIRECTORIES(SYSTEM third_party)

ADD_EXECUTABLE(holotable src/main.cxx)
SET_TARGET_PROPERTIES(holotable PROPERTIES CXX_MODULE_STD ON)
IF (WIN32)
	TARGET_LINK_LIBRARIES(holotable PUBLIC gdi32)
ELSE()
	find_package(X11 REQUIRED)
	find_package(XRandR REQUIRED)

	target_link_libraries(holotable ${X11_LIBRARIES} ${XRANDR_LIBRARIES})
ENDIF()

ADD_EXECUTABLE(tests tests/main.cxx)
SET_TARGET_PROPERTIES(tests PROPERTIES CXX_MODULE_STD ON)
