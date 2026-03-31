#pragma once
#include <vector>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vulkan/vulkan.h>
// #include "imgv.h"
#include "shapeGen.h"

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

    /// @brief Number of vertecies indexed
    uint32_t indexCount = 0;

    /// @brief Error storage
    std::string errStr;

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
    /// @brief Draw with given command buffer. Does nothing if no mesh is available
    /// @param cmd
    void recorDraw(VkCommandBuffer cmd,uint32_t width, uint32_t height);
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
