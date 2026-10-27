cmake_minimum_required(VERSION 3.24)
project(Sandbox)

include(FetchContent)

option(BUILD_SHARED_LIBS "Build Using Shared Libraries" ON)

# SDL3
# We want to build SDL3 as a shared library (DLL), and we don't need the static library, tests, or examples.
set(SDL_SHARED ON CACHE BOOL "Build shared SDL3" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Do not build static SDL3" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Do not build SDL3 test support" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Do not build SDL3 tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Do not build SDL3 examples" FORCE)

# FetchContent will download SDL3 from the specified Git repository and tag, and make it available for linking in our project.
# Give it a meaningful name, tell us where to find it and which version, and don’t get the full commit history
FetchContent_Declare(
	SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-3.4.16
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL3)

add_library(Frost SHARED Frost/src/Application.cpp  Frost/src/WindowsWindow.cpp)
add_executable(Sandbox Sandbox/src/Sandbox.cpp)
target_include_directories(Frost PUBLIC Frost/headers)

target_link_libraries(Frost PRIVATE SDL3::SDL3)
target_link_libraries(Sandbox PUBLIC Frost)

set_target_properties(Frost PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(WIN32)
	add_custom_target(StageRuntime
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:Sandbox>
		VERBATIM)
	add_dependencies(Sandbox StageRuntime)
endif()