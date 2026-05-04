cmake_minimum_required(VERSION 3.22)

project(synthCoil
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR vendor/ImGui/)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends include)

set(SHADERDIR ${CMAKE_SOURCE_DIR}/shaders/)

set(SHADERS 
    ${SHADERDIR}coil.vert
    ${SHADERDIR}coil.frag
)

add_executable(synthDemo
    src/main.cpp
    src/audio.cpp
    src/imgv.cpp
    src/freqs.cpp
    src/shapeGen.cpp
    src/vulkRend.cpp
    ${IMGUI_DIR}imgui.cpp
    ${IMGUI_DIR}imgui_demo.cpp
    ${IMGUI_DIR}imgui_draw.cpp
    ${IMGUI_DIR}imgui_tables.cpp
    ${IMGUI_DIR}imgui_widgets.cpp
    ${IMGUI_DIR}backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}backends/imgui_impl_sdl3.cpp
)

# Grab SDL
add_subdirectory("vendor/SDL" "vendor/SDL" EXCLUDE_FROM_ALL)
target_link_libraries(synthDemo PRIVATE SDL3::SDL3)

# Grab Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(synthDemo PRIVATE Vulkan::Vulkan)

# Grab GLSLC for compiling shaders
find_program(GLSLC_EXECUTABLE glslc HINTS ${Vulkan_GLSLC_EXECUTABLE})

# Grab GLM for graphics
find_package(glm QUIET CONFIG)
if(glm_FOUND)
    message(STATUS "Using system GLM")
else()
    message(STATUS "Using vendored GLM")
    add_subdirectory("vendor/glm" "vendor/glm" EXCLUDE_FROM_ALL)
endif()

target_link_libraries(synthDemo PRIVATE glm::glm)

# Grab FFTW3
add_subdirectory("vendor/fftw3" "vendor/fftw3" EXCLUDE_FROM_ALL)

target_link_libraries(synthDemo PRIVATE fftw3f)

target_include_directories(synthDemo PRIVATE "vendor/fftw3/api" )

# Setup Shader compilation at build
set(SPIRV_OUTPUTS)

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
    )
    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(synthDemo Shaders) 

# If windows target, copy .dlls
if (WIN32)
    add_custom_command( TARGET synthDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:fftw3f>"
        "$<TARGET_FILE_DIR:synthDemo>"
    )
    add_custom_command( TARGET synthDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:synthDemo>/vendor/SDL/SDL3.dll"
        "$<TARGET_FILE_DIR:synthDemo>"
    )
endif()