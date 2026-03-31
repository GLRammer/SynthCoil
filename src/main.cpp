#include "audio.h"
#include "freqs.h"
#include "shapeGen.h"
#include "vulkRend.h"
#include <string>
#include <iostream>

// Dear ImGui stuffs
#include "imgui.h"
#include "imgv.h"

int main(int argc, char *argv[])
{
    // SDL init
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cerr << "Error: SDL_Init(): " << SDL_GetError() << std::endl;
        return 1;
    }
    // Initialize state object
    progState cState{};

    // fetch audio devices
    if (!audioDevFetch(cState))
        return -1;

    // run custom setup function
    if (!customSetup(cState))
        return -1;

    // run program loop
    if (!runLoop(cState))
    {
        return -1;
    }

    // Cleanup
    SDL_free(cState.devices);
    check_vk_result(cState.err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow(&g_MainWindowData);
    CleanupVulkan();

    SDL_DestroyWindow(cState.window);
    SDL_Quit();

    return 0;
}
