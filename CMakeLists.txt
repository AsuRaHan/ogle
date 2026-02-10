cmake_minimum_required(VERSION 3.20)
project(ogle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Путь к ресурсам (иконка)
set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc")

# ────────────────────────────────────────────────────────────────
# Зависимости через FetchContent
# ────────────────────────────────────────────────────────────────

include(FetchContent)

# GLAD 2
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    GIT_PROGRESS   TRUE
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_46 STATIC REPRODUCIBLE LOADER API gl:core=4.6)
set_target_properties(glad_gl_core_46 PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    POSITION_INDEPENDENT_CODE   OFF
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)

# EnTT
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.16.0
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(entt)

# ImGui — master (docking теперь встроен)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        master
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Для Win32 бэкенда
if(WIN32)
    target_link_libraries(imgui PRIVATE imm32)
endif()

# stb_image
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(stb)

add_library(stb_interface INTERFACE)
target_include_directories(stb_interface INTERFACE ${stb_SOURCE_DIR})

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# assimp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v6.0.4
    GIT_PROGRESS   TRUE
)

set(ASSIMP_BUILD_TESTS              OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL                  OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT                ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS       OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

# ────────────────────────────────────────────────────────────────
# Основной проект
# ────────────────────────────────────────────────────────────────

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"    # если используешь .hpp
)

add_executable(ogle WIN32
    ${SOURCES}
    ${APP_ICON_RESOURCE}
)

# Флаги компиляции
if(MSVC)
    target_compile_options(ogle PRIVATE /utf-8 /W4 /MP)
    target_compile_definitions(ogle PRIVATE UNICODE _UNICODE)
else()
    target_compile_options(ogle PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Линковка
target_link_libraries(ogle PRIVATE
    glad_gl_core_46
    stb_interface
    glm::glm
    imgui
    EnTT::EnTT               # ← автоматом подтянет include
    nlohmann_json::nlohmann_json
    assimp::assimp
    opengl32
    user32
    gdi32
    imm32
)

# Include-директории (assimp уже включён в target)
target_include_directories(ogle PRIVATE
    src
    ${stb_SOURCE_DIR}
    # ${entt_SOURCE_DIR}/single_include   ← не нужен, если EnTT::EnTT
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

set_target_properties(ogle PROPERTIES
    WIN32_EXECUTABLE            TRUE
    VS_GLOBAL_CharacterSet      "Unicode"
    DEBUG_POSTFIX               d
)

set_source_files_properties(${APP_ICON_RESOURCE} PROPERTIES LANGUAGE RC)

# Копирование .exe в корень (Debug + Release)
add_custom_command(TARGET ogle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ogle>
        ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:ogle>
    COMMENT "Копируем исполняемый файл в корень проекта"
)

# Полезно для отладки
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")