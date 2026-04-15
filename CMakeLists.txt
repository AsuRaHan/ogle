cmake_minimum_required(VERSION 4.2)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()
set(PROJECT_NAME "OGLE3D")
project(${PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

# =============================================================================
# Автоматический сбор всех исходных файлов (рекомендуется на текущем этапе)
# =============================================================================

# Собираем все .cpp файлы рекурсивно из папки src и всех её подпапок
file(GLOB_RECURSE PROJECT_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Собираем все заголовочные файлы (.h и .hpp) — это нужно для удобства в IDE 
# (Visual Studio, CLion и т.д. будут правильно показывать структуру проекта)
file(GLOB_RECURSE PROJECT_HEADERS 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

# Создаём исполняемый файл
add_executable(${PROJECT_NAME} WIN32
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

# Указываем, где искать заголовочные файлы (очень важно!)
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
# =============================================================================

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glm)

# Подключаем библиотеку Duktape
FetchContent_Declare(
    duktape
    URL https://duktape.org/duktape-2.7.0.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(duktape)
if(NOT duktape_POPULATED)
    FetchContent_MakeAvailable(duktape)
    add_library(duktape STATIC ${duktape_SOURCE_DIR}/src/duktape.c)
    target_include_directories(duktape PUBLIC ${duktape_SOURCE_DIR}/src)
endif()

FetchContent_Declare(
    dukglue
    GIT_REPOSITORY https://github.com/Aloshi/dukglue.git
    GIT_TAG master
    GIT_PROGRESS   TRUE
)
FetchContent_GetProperties(dukglue)
if(NOT dukglue_POPULATED)
    FetchContent_MakeAvailable(dukglue)
endif()

# Подключаем библиотеку stb_image для загрузки изображений
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Подключаем библиотеки assimp nlohmann/json и OpenMesh
# Отключаем тесты, инструменты и лишние части Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG master
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
    openmesh
    GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh.git
    GIT_TAG master
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(openmesh)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.16.0
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(entt)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG develop
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Подключаем библиотеки ImGui и ImGuizmo
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
    GIT_PROGRESS TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
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
    target_link_libraries(imgui PUBLIC opengl32 user32 gdi32)
endif()

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG master
    GIT_PROGRESS   TRUE
)
FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_MakeAvailable(imguizmo)
    add_library(imguizmo STATIC
        ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
    )
    target_include_directories(imguizmo PUBLIC
        ${imguizmo_SOURCE_DIR}
    )
    target_link_libraries(imguizmo PUBLIC imgui)
endif()

set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    bullet
    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
    GIT_TAG master
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(bullet)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src 
    ${bullet_SOURCE_DIR}/src
    ${dukglue_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    assimp::assimp 
    nlohmann_json::nlohmann_json
    EnTT::EnTT
    glm::glm
    OpenMeshCore 
    OpenMeshTools
    duktape 
    imgui 
    imguizmo 
    BulletDynamics
    BulletCollision
    LinearMath
    shlwapi
    user32 gdi32 opengl32 ole32 windowscodecs
)



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)