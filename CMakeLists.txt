cmake_minimum_required(VERSION 3.20)
project(ogle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

if(MSVC)
    add_compile_options(/utf-8 /W4 /MP)
    add_compile_definitions(UNICODE _UNICODE)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

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

target_include_directories(ogle PRIVATE src)

target_link_libraries(ogle PRIVATE
    glad_gl_core_46
    glm::glm
    opengl32          # системная библиотека OpenGL
    user32
    gdi32
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