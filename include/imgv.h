/// DearImGui, SDL, and Vulkan helper file
/// Based on DearImGui SDL3+Vulkan example at
/// vendor/ImGui/examples/example_sdl3_vulkan/
#pragma once
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <iostream>
#include "vulkRend.h"
#include "audio.h"
#include "freqs.h"

extern VkAllocationCallbacks *g_Allocator;
extern VkInstance g_Instance;
extern VkPhysicalDevice g_PhysicalDevice;
extern VkDevice g_Device;
extern uint32_t g_QueueFamily;
extern VkQueue g_Queue;
extern VkPipelineCache g_PipelineCache;
extern VkDescriptorPool g_DescriptorPool;

extern ImGui_ImplVulkanH_Window g_MainWindowData;
extern uint32_t g_MinImageCount;
extern bool g_SwapChainRebuild;

struct progState
{
    // ImGui data
    ImGui_ImplVulkanH_Window *wd;
    // SDL data
    SDL_Window *window;
    // Vulkan data
    VkResult err;
    int w, h;
    // Audio data
    audio myAudio;
    SDL_AudioDeviceID *devices;
    freqHolder myFreqs;
    int devCnt;
};

extern void check_vk_result(VkResult err);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
    (void)flags;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

extern bool IsExtensionAvailable(const ImVector<VkExtensionProperties> &properties, const char *extension);

extern void SetupVulkan(ImVector<const char *> instance_extensions);

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
extern void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height);

extern void CleanupVulkan();

extern void CleanupVulkanWindow(ImGui_ImplVulkanH_Window *wd);

extern void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);

extern void myFrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data, vulkRend &myRend);

extern void FramePresent(ImGui_ImplVulkanH_Window *wd);

// Custom program state setup
bool customSetup(progState &cState);

// Run main program loop
bool runLoop(progState &cState);

// SDL event polling
int eventPoll(SDL_Event &event, progState &cState);

// Display final shape
bool finalDisp();

// Display device selection prompt
bool devSelect(progState &cState, bool &devSel, int &devselected);

// Display main run state
bool runningState(progState &cState, ImVec2 volBarDim, float &lastMag, vulkRend &myRend, shapeGen &myShape, bool &displayShape);

// Draw volume bar
void volBar(ImVec2 volBarDim, float lastMag);

// Fetch audio device info for selection
bool audioDevFetch(progState &cState);