#include "audio.h"
#include "freqs.h"
#include <string>
#include <iostream>

// Dear ImGui stuffs
#include "imgui.h"
#include "imgv.h"

void catchStream(audio myAudio, freqHolder myfreqs, std::string &errorstr);

int main(int argc, char *argv[])
{
    // SDL init
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cerr << "Error: SDL_Init(): "<<SDL_GetError()<<std::endl;
        return 1;
    }
    std::string errorstr = "";
    audio myAudio;
    freqHolder myFreqs;

    //get devices info for menu
    int devCnt;
    SDL_AudioDeviceID* devices=SDL_GetAudioRecordingDevices(&devCnt);
    if(devCnt==0 || devices==NULL){
        std::cerr << "No devices found\n";
        return -1;
    }
    // std::cout << SDL_GetAudioDeviceName(devices[0]) << std::endl;
    //////  The following section is modified code from ImGui/examples/example_sdl3_vulkan/

    
    // Create window with Vulkan graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("SynthCoil", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        std::cerr << "Error: SDL_CreateWindow(): "<<SDL_GetError()<<std::endl;
        return 1;
    }

    ImVector<const char*> extensions;
    {
        uint32_t sdl_extensions_count = 0;
        const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
        for (uint32_t n = 0; n < sdl_extensions_count; n++)
            extensions.push_back(sdl_extensions[n]);
    }
    SetupVulkan(extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err;
    if (SDL_Vulkan_CreateSurface(window, g_Instance, g_Allocator, &surface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        return 1;
    }

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.Allocator = g_Allocator;
    init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    bool devSel = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        {
            if(!devSel){

                ImGui::Begin("Select Device");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("Please select an input device.");               // Display some text (you can use a format strings too)
                ImGui::Text("%d devices detected.", devCnt);
                int devselected=0;
                const char* previewVal = SDL_GetAudioDeviceName(devices[devselected]);
                if(ImGui::BeginCombo("Select Device",previewVal)){
                    for (int i=0;i<devCnt;i++){
                        const bool is_sel=(devselected==i);
                        std::string tempstr=std::to_string(i)+": "+std::string(SDL_GetAudioDeviceName(devices[i]));
                        if(ImGui::Selectable(tempstr.c_str(),is_sel))
                            devselected=i;
                    }
                    ImGui::EndCombo();
                }
                if(ImGui::Button("Select")){
                    myAudio.selectDev(devices[devselected]);
                    // std::cout<< SDL_GetCurrentAudioDriver()<<std::endl;
                    devSel=true;
                    if(myAudio.startStream()==-1){
                        std::cerr << myAudio.getErr() << std::endl;
                        done=true;
                        SDL_free(devices);
                    }
                    
                }
                
            }else{
                ImGui::Begin("Current Volume");
                if(myAudio.available()==0){
                    ImGui::Text("I hear %.3f", myAudio.currVol());
                    myFreqs.freqGet(myAudio);
                }
                if(ImGui::Button("End Stream")){
                    catchStream(myAudio,myFreqs,errorstr);
                    done=true;
                }
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }
    }
    /////// End modified example code

    // if (myAudio.startStream() == 0)
    // {
    //     SAMPLE max = 0;
    //     for (int i = 0; i < (int)data.recordedSamples.size(); i++)
    //     {
    //         if (data.recordedSamples[i] > max)
    //             max = data.recordedSamples[i];
    //     }

    //     // freq sample
    //     freqHolder tempout = freqGet(data.recordedSamples);
    //     std::vector<SAMPLE> freqs = tempout.frequencies, mags = tempout.magnitudes;
    //     for (int i = FFTSZ; i + FFTSZ < data.recordedSamples.size(); i += FFTSZ)
    //     {
    //         tempout = freqGet(data.recordedSamples, i);
    //         freqs.insert(freqs.end(), tempout.frequencies.begin(), tempout.frequencies.end());
    //         mags.insert(mags.end(), tempout.magnitudes.begin(), tempout.magnitudes.end());
    //     }
    //     // analyze sample
    //     max = 0;
    //     SAMPLE maxFreq;
    //     for (int i = 0; i < freqs.size(); i++)
    //     {
    //         if (mags[i] > max)
    //         {
    //             max = mags[i];
    //             maxFreq = freqs[i];
    //         }
    //     }
    //     std::cout << "Max freq= " << maxFreq << std::endl;
    // }
    // else
    // {
    //     std::cout << errorstr << std::endl;
    // }
    return 0;
}

void catchStream(audio myAudio,freqHolder myfreqs, std::string &errorstr){
    // analyze sample
    SAMPLE max = 0;
    SAMPLE maxFreq;
    for (int i = 0; i < myfreqs.frequencies.size(); i++)
    {
        if (myfreqs.magnitudes[i] > max)
        {
            max = myfreqs.magnitudes[i];
            maxFreq = myfreqs.frequencies[i];
        }
    }
    std::cout << "Max freq= " << maxFreq << " at " << max << std::endl;
    std::cout << "num of freqs sampled: " << myfreqs.frequencies.size() << std::endl;
}