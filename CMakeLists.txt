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

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/App.cpp

    src/config/AppConfig.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
    src/core/FileSystem.cpp
    src/core/FileSystem.h
    src/editor/Editor.cpp
    src/editor/Editor.h
    src/editor/EditorAssetHelpers.cpp
    src/editor/EditorAssetHelpers.h
    src/editor/EditorContentBrowserPanel.cpp
    src/editor/EditorContentBrowserPanel.h
    src/editor/EditorCreationPanel.cpp
    src/editor/EditorCreationPanel.h
    src/editor/EditorProceduralTexturePanel.cpp
    src/editor/EditorProceduralTexturePanel.h
    src/editor/EditorHierarchyPanel.cpp
    src/editor/EditorHierarchyPanel.h
    src/editor/EditorMeshEditPanel.cpp
    src/editor/EditorMeshEditPanel.h
    src/editor/EditorInspectorPanel.cpp
    src/editor/EditorInspectorPanel.h
    src/editor/EditorAnimationPanel.cpp
    src/editor/EditorAnimationPanel.h

    src/ui/Win32Window.cpp
    src/ui/Win32Window.h
    src/ui/IWindow.h

    src/opengl/OpenGLInitializer.cpp
    src/opengl/OpenGLInitializer.h
    src/opengl/OpenGLDebug.cpp
    src/opengl/OpenGLDebug.h
    src/opengl/GLFunctions.cpp
    src/opengl/GLFunctions.h
    src/opengl/ShaderManager.cpp
    src/opengl/ShaderManager.h
    src/opengl/OpenGLRenderer.cpp
    src/opengl/OpenGLRenderer.h
    src/opengl/Camera.cpp
    src/opengl/Camera.h

    src/Logger.cpp
    src/Logger.h
    src/WorldGenerator.cpp
    src/WorldGenerator.h


    src/models/PrimitiveFactory.cpp
    src/models/PrimitiveFactory.h

    src/models/MeshBuffer.cpp
    src/models/MeshBuffer.h
    src/models/BaseModel.cpp
    src/models/BaseModel.h
    src/models/ModelEntity.cpp
    src/models/ModelEntity.h
    src/render/Texture2D.cpp
    src/render/Texture2D.h
    src/render/Material.cpp
    src/render/Material.h
    src/render/MaterialLibrary.cpp
    src/render/MaterialLibrary.h
    src/render/AnimationLibrary.cpp
    src/render/AnimationLibrary.h
    src/render/ProceduralTexture.cpp
    src/render/ProceduralTexture.h
    src/render/TextureGenerator.cpp
    src/render/TextureGenerator.h

    src/input/AxisBinding.cpp
    src/input/AxisBinding.h
    src/input/InputAction.cpp
    src/input/InputAction.h
    src/input/InputController.cpp
    src/input/InputController.h
    src/input/InputTypes.h

    src/managers/InputManager.cpp
    src/managers/InputManager.h
    src/managers/ImGuiManager.cpp
    src/managers/ImGuiManager.h
    src/managers/PhysicsManager.cpp
    src/managers/PhysicsManager.h
    src/physics/BulletPhysicsEngine.cpp
    src/physics/BulletPhysicsEngine.h
    src/managers/CameraManager.cpp
    src/managers/CameraManager.h
    src/managers/InputActionsManager.cpp
    src/managers/InputActionsManager.h
    src/managers/RenderManager.cpp
    src/managers/RenderManager.h
    src/managers/ScriptManager.cpp
    src/managers/ScriptManager.h
    src/managers/TimeManager.cpp
    src/managers/TimeManager.h
    src/managers/WorldManager.cpp
    src/managers/WorldManager.h

    src/scripting/ScriptBindings.cpp
    src/scripting/ScriptBindings.h
    src/scripting/ScriptEngine.cpp
    src/scripting/ScriptEngine.h
    src/scripting/Player.cpp
    src/scripting/Player.h

    src/scripting/bindings/Common.cpp
    src/scripting/bindings/Common.h
    src/scripting/bindings/LogBindings.cpp
    src/scripting/bindings/LogBindings.h
    src/scripting/bindings/WorldBindings.cpp
    src/scripting/bindings/WorldBindings.h
    src/scripting/bindings/EntityBindings.cpp
    src/scripting/bindings/EntityBindings.h
    src/scripting/bindings/MaterialBindings.cpp
    src/scripting/bindings/MaterialBindings.h
    src/scripting/bindings/LightBindings.cpp
    src/scripting/bindings/LightBindings.h
    src/scripting/bindings/PhysicsBindings.cpp
    src/scripting/bindings/PhysicsBindings.h

    src/world/World.cpp
    src/world/World.h
    src/world/WorldComponents.h
    src/world/WorldObject.cpp
    src/world/WorldObject.h
)

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

# Подключаем библиотеки assimp nlohmann/json и OpenMesh 
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