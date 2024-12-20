# Top-level CMake project file, for global configuration and sub-projects.
cmake_minimum_required(VERSION 3.8)
project("CSE167_FA22_HW2")

# Add sources for the Model Viewer
file(
    GLOB SOURCES_MODEL_VIEWER
    src/*.cpp
)

# Include Dear ImGui sources
set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/include/img")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glut.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Combine all sources
list(APPEND SOURCES_MODEL_VIEWER ${IMGUI_SOURCES})

# Add executable target
add_executable(ModelViewer ${SOURCES_MODEL_VIEWER} "include/Frustum.h")

# Include headers
set(
    INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/GL
    ${PROJECT_SOURCE_DIR}/include/glm
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_include_directories(ModelViewer PRIVATE ${INCLUDE_DIRECTORIES})

# Link libraries
set(
    LINK_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/lib
)
target_link_directories(ModelViewer PRIVATE ${LINK_DIRECTORIES})
target_link_libraries(ModelViewer glew32 freeglut FreeImage opengl32)

# Ensure .dll is with .exe
file(COPY "${LINK_DIRECTORIES}/glew32.dll" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${LINK_DIRECTORIES}/freeglut.dll" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${LINK_DIRECTORIES}/FreeImage.dll" DESTINATION "${CMAKE_BINARY_DIR}")

# Move assets to .exe
add_custom_target(CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/shaders"
    "${CMAKE_BINARY_DIR}/shaders"
)
add_custom_target(CopyModels ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/models"
    "${CMAKE_BINARY_DIR}/models"
)
add_dependencies(ModelViewer CopyShaders)
add_dependencies(ModelViewer CopyModels)
