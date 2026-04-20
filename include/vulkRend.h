#pragma once
#include <vector>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #include "imgv.h"
#include "shapeGen.h"

// Uniform buffer for 3d rendering
struct uniformBuffer
{
    glm::mat4 model{0.0f};
    glm::mat4 view{0.0f};
    glm::mat4 proj{0.0f};
    float scale = 1.0;
    bool operator==(const uniformBuffer &other) const
    {
        return (model == other.model && view == other.view && proj == other.proj && scale == other.scale);
    }
    bool isEmpty() const
    {
        return (model == glm::mat4(0.0f) && view == glm::mat4(0.0f) && proj == glm::mat4(0.0f));
    }
};

class vulkRend
{
private:
    // Vulkan values
    VkPhysicalDevice physDev = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphQ = VK_NULL_HANDLE;
    uint32_t qFam = 0;
    VkRenderPass rendPass = VK_NULL_HANDLE;
    VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipe = VK_NULL_HANDLE;
    VkBuffer vertBuff = VK_NULL_HANDLE;
    VkDeviceMemory vertMem = VK_NULL_HANDLE;
    VkBuffer indBuff = VK_NULL_HANDLE;
    VkDeviceMemory indMem = VK_NULL_HANDLE;
    VkBuffer freqBuff = VK_NULL_HANDLE;
    VkDeviceMemory freqMem = VK_NULL_HANDLE;
    VkDescriptorSetLayout descLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    VkBuffer uB = VK_NULL_HANDLE;
    VkDeviceMemory uBMem = VK_NULL_HANDLE;

    float smooth = 0.5;
    int maxFreqCnt = 50;

    // Color stuffs
    float chroma[3] = {1.0, 1.0, 1.0};
    float targChroma[3] = {1.0, 1.0, 1.0};
    bool transition = true;
    float chromaMath(float a, float b);
    bool chromaPass();
    bool initChroma();
    VkBuffer chromaBuff = VK_NULL_HANDLE;
    VkDeviceMemory chromaMem = VK_NULL_HANDLE;

    /// @brief Creats a buffer and binds it to the given memory
    /// @param sz       size of buffer
    /// @param use      Vulkan usage flags
    /// @param prop     Vulkan property flags
    /// @param buff     Pointer to fill with the generated buffer
    /// @param mem      Pointer to device memory to bind
    /// @return     False on failure, true on success
    bool createBuffer(
        VkDeviceSize sz,
        VkBufferUsageFlags use,
        VkMemoryPropertyFlags prop,
        VkBuffer &buff,
        VkDeviceMemory &mem);
    int findMemType(uint32_t filter, VkMemoryPropertyFlags properties);

    /// @brief Deletes all mesh information
    void destroyMesh();
    /// @brief Add vertecies to mesh and bind to memory
    /// @param verts    Vector of vertecies
    /// @return     False on failure, true on success
    bool uploadVert(const std::vector<shapeVertex> &verts);
    /// @brief Add indexes to mesh and bind to memory
    /// @param inds     Vector of indexes
    /// @return     False on failure, true on success
    bool uploadInd(const std::vector<int> &inds);
    // Deal with freqs and uB if not existing
    bool initFreqs();
    bool initUB();

    /// @brief Number of vertecies indexed
    uint32_t indexCount = 0;

    /// @brief Error storage
    std::string errStr;

    uniformBuffer lastUB;

public:
    /// @brief Constructed with Vulkan values
    vulkRend(
        VkPhysicalDevice physdev,
        VkDevice dev,
        VkQueue q,
        uint32_t qFamily,
        VkRenderPass rp);

    /// @brief Initialize the Vulkan pipeline
    /// @param vertPath     OS location of vertex SPV shader code
    /// @param fragPath     OS location of fragment SPV shader code
    /// @return             False on failure, true on success, use getErr() for more info.
    bool initPipe(const std::string &vertPath, const std::string &fragPath);

    /// @brief Update the mesh with fresh data
    /// @param verts    Vector of new vertecies
    /// @param ind      Vector of new indexes
    /// @return         False on failure, true on success, use getErr() for more info.
    bool updateMesh(const std::vector<shapeVertex> &verts, const std::vector<int> &ind);

    /// @brief Update frequency buffer with fresh data
    /// @param freqs    Vector of frequency/magnitude pairs where frequency is expressed as a ratio <=1
    /// @return         False on failure, true on success, use getErr() for more info.
    bool updateFreqs(const std::vector<std::pair<float, float>> &freqs);

    /// @brief Update smoothing value
    /// @param newSmooth new smoothing value. range [0-1]
    void updateSmooth(float newSmooth);

    /// @brief Bind new uniform buffer to memory
    /// @param uBuff    Uniform buffer input
    /// @return     False on failure, true on success, use getErr() for more info
    bool updateUB(const uniformBuffer &uBuff);

    const uniformBuffer getUB();

    // Update target color for the shape. Returns false on failure.
    bool updateColor(float r = 1.0, float g = 1.0, float b = 1.0);
    bool updateColor(float rgb[3]);

    /// @brief Draw with given command buffer. Does nothing if no mesh is available
    /// @param cmd
    void recorDraw(VkCommandBuffer cmd, uint32_t width, uint32_t height);

    /// @brief Destroy all pipelines and the current mesh
    void cleanup();

    /// @brief Fetch latest error
    /// @return Last triggered error
    std::string getErr();

    ~vulkRend();
};

/// @brief File reading helper for fetching SPV shader code
/// @param filename     Target file location
/// @return             File contents in a char vector
std::vector<char> readFile(const std::string &filename);

/// @brief Helper function to convert raw SPV shader code to a shader module
/// @param dev      Target device
/// @param code     Raw shader code input
/// @return         Vulkan shader module
VkShaderModule createShaderModule(VkDevice dev, const std::vector<char> &code);

/// @brief Function to quickly generate a default uniform buffer
/// @return Default uniform buffer instance
uniformBuffer defaultUB();