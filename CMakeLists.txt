cmake_minimum_required(VERSION 3.20)
project(ogle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Путь к ресурсам (иконка)
set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc")

# FetchContent для GLAD 2
include(FetchContent)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8           # или более свежий тег, если есть
    GIT_PROGRESS   TRUE
    SOURCE_SUBDIR  cmake            # очень важно! CMakeLists.txt лежит в поддиректории cmake/
)

FetchContent_MakeAvailable(glad)

# Генерируем GLAD для OpenGL Core 4.6 (можно изменить версию)
glad_add_library(glad_gl_core_46 STATIC REPRODUCIBLE LOADER API gl:core=4.6)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3            # или более свежий тег, если есть
)

FetchContent_MakeAvailable(glm)

# Добавляем EnTT (header-only библиотека для ECS)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.16.0          # свежая версия на момент написания; проверь на GitHub
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(entt)

# Добавляем ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking  # Ветка с поддержкой докинга
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(imgui)

# Создаем библиотеку ImGui
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

target_compile_definitions(imgui PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD2  # Используем GLAD2
)

# Добавляем stb_image (stb) для загрузки текстур
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master  # конкретный коммит для стабильности
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(stb)

# Создаем интерфейсную библиотеку для stb
add_library(stb_interface INTERFACE)
target_include_directories(stb_interface INTERFACE 
    ${stb_SOURCE_DIR}
)

# Добавляем nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.4
    GIT_PROGRESS TRUE
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

# Для Win32 бэкенда
if(WIN32)
    target_link_libraries(imgui PRIVATE imm32)
endif()

# Автоматический сбор всех .cpp и .h файлов из src/
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Основной исполняемый файл (все исходники автоматически добавлены)
add_executable(ogle WIN32
    ${SOURCES}
    ${APP_ICON_RESOURCE}
)

if(MSVC)
    add_compile_options(/utf-8 /W4 /MP)
    add_compile_definitions(UNICODE _UNICODE)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# В target_link_libraries добавь imgui:
target_link_libraries(ogle PRIVATE
    glad_gl_core_46
    stb_interface
    glm::glm
    imgui
    nlohmann_json::nlohmann_json
    assimp::assimp
    opengl32
    user32
    gdi32
    imm32                    # Для Win32 input (может потребоваться)
)

# Добавляем include директории
target_include_directories(ogle PRIVATE 
    src
    ${stb_SOURCE_DIR}
    ${entt_SOURCE_DIR}/single_include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${assimp_SOURCE_DIR}/include
)


set_target_properties(ogle PROPERTIES
    WIN32_EXECUTABLE            TRUE
    VS_GLOBAL_CharacterSet      "Unicode"
    DEBUG_POSTFIX               d
)

set_source_files_properties(${APP_ICON_RESOURCE} PROPERTIES LANGUAGE RC)

# ────────────────────────────────────────────────────────────────
# Копирование исполняемого файла в корень проекта после сборки
# ────────────────────────────────────────────────────────────────

# Для Release
add_custom_command(TARGET ogle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ogle>
        ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:ogle>
    COMMENT "Копируем ogle.exe в корень проекта (Release)"
    CONFIG Release
)

# Для Debug
add_custom_command(TARGET ogle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ogle>
        ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:ogle>
    COMMENT "Копируем ogled.exe в корень проекта (Debug)"
    CONFIG Debug
)