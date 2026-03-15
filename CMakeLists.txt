cmake_minimum_required(VERSION 3.16)
project(AutoClicker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# SDL2 — forzar build estático para .exe sin DLL
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)

find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
	message(STATUS "SDL2 no encontrada. Descargando automáticamente...")
	FetchContent_Declare(SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG release-2.30.0
	)
	FetchContent_MakeAvailable(SDL2)
endif()

# ImGui
FetchContent_Declare(imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.91.0
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

find_package(OpenGL REQUIRED)

add_executable(AutoClicker WIN32
	src/main.cpp
	src/autoclicker.cpp
	src/ui.cpp
	${IMGUI_SOURCES}
)

target_include_directories(AutoClicker PRIVATE
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends
	${SDL2_SOURCE_DIR}/include
)

target_link_libraries(AutoClicker PRIVATE
	SDL2::SDL2main
	SDL2::SDL2-static
	OpenGL::GL
	# dependencias de sistema que SDL2 estático necesita en Windows
	winmm
	imm32
	version
	setupapi
)
