cmake_minimum_required(VERSION 3.22)

project(synthCoil
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR vendor/ImGui/)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends include)

add_executable(synthDemo
    src/main.cpp
    src/audio.cpp
    src/imgv.cpp
    src/freqs.cpp
    src/shapeGen.cpp
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
# TODO figure out how to vendor Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(synthDemo PRIVATE Vulkan::Vulkan)

# Grab portaudio
# add_subdirectory("vendor/portaudio" "vendor/portaudio" EXCLUDE_FROM_ALL)

# target_link_libraries(synthDemo PRIVATE portaudio)


# Grab FFTW3
add_subdirectory("vendor/fftw3" "vendor/fftw3" EXCLUDE_FROM_ALL)

target_link_libraries(synthDemo PRIVATE fftw3f)

target_include_directories(synthDemo PRIVATE "vendor/fftw3/api" )

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
    add_custom_command( TARGET synthDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/vulkan-1.dll"
        "$<TARGET_FILE_DIR:synthDemo>"
    )
endif()