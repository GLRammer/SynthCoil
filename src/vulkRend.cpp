#include "vulkRend.h"

std::vector<char> readFile(const std::string &filename)
{
    // Construct filestream
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    // Check if file will open
    if (!file.is_open())
        return {};

    // Check filesize to pre-allocate memory
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    // Return to head of file and read data
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule createShaderModule(VkDevice dev, const std::vector<char> &code)
{
    // Check for empty vector
    if (code.empty() || code.size() % 4 != 0)
        return VK_NULL_HANDLE;

    // Create module info
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    // Build module with info and check for failure
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(dev, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return shaderModule;
}

vulkRend::vulkRend(
    VkPhysicalDevice physdev,
    VkDevice dev,
    VkQueue q,
    uint32_t qFamily,
    VkRenderPass rp)
{
    physDev = physdev;
    device = dev;
    graphQ = q;
    qFam = qFamily;
    rendPass = rp;
}

int vulkRend::findMemType(uint32_t filter, VkMemoryPropertyFlags properties)
{
    // fetch device mem properties
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDev, &memProperties);

    // iterate through device memory types
    for (int i = 0; i < memProperties.memoryTypeCount; i++)
    {
        // if this memtype is in the filter
        if ((filter & (1 << i))
            // and this type's properties match our given properties
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            // return current index
            return (uint32_t)i;
        }
    }

    // Fail condition
    errStr = "Failed Vulkan memory search.";
    return -1;
}

bool vulkRend::createBuffer(
    VkDeviceSize sz,
    VkBufferUsageFlags use,
    VkMemoryPropertyFlags prop,
    VkBuffer &buff,
    VkDeviceMemory &mem)
{
    VkBufferCreateInfo buffInfo{};
    buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffInfo.size = sz;
    buffInfo.usage = use;
    buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Check for failure while generating
    if (vkCreateBuffer(device, &buffInfo, nullptr, &buff) != VK_SUCCESS)
    {
        errStr = "Failed buffer creation.";
        return false;
    }

    // Catch buffer reqs
    VkMemoryRequirements memreqs;
    vkGetBufferMemoryRequirements(device, buff, &memreqs);

    // Build mem info
    VkMemoryAllocateInfo memInfo{};
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = memreqs.size;
    int temp = findMemType(memreqs.memoryTypeBits, prop);
    if (temp == -1)
    {
        return false;
    }
    memInfo.memoryTypeIndex = (uint32_t)temp;

    // Check for failure while allocating
    if (vkAllocateMemory(device, &memInfo, nullptr, &mem) != VK_SUCCESS)
    {
        vkFreeMemory(device, mem, nullptr);
        errStr = "Failed memory allocation.";
        return false;
    }

    // Check for failure while binding memory and buffer
    if (vkBindBufferMemory(device, buff, mem, 0))
    {
        vkFreeMemory(device, mem, nullptr);
        errStr = "Failed memory buffer binding.";
        return false;
    }
    return true;
}

void vulkRend::destroyMesh()
{

    // Clear index info
    if (indBuff != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, indBuff, nullptr);
        indBuff = VK_NULL_HANDLE;
    }
    if (indMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, indMem, nullptr);
        indMem = VK_NULL_HANDLE;
    }

    // Clear vertex info
    if (vertBuff != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, vertBuff, nullptr);
        vertBuff = VK_NULL_HANDLE;
    }
    if (vertMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, vertMem, nullptr);
        vertMem = VK_NULL_HANDLE;
    }

    // Empty count
    indexCount = 0;
}

bool vulkRend::uploadVert(const std::vector<shapeVertex> &verts)
{
    // Check for empty vertex
    if (verts.empty())
    {
        errStr = "Vertex upload called with empty vector.";
        return false;
    }

    // Grab verts size in memory
    VkDeviceSize sz = sizeof(shapeVertex) * verts.size();

    // Call buffer creation. If it fails, pass its error along
    if (!createBuffer(
            sz,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertBuff,
            vertMem))
    {
        return false;
    }

    // temp data pointer
    void *data = nullptr;

    // Map data to vertMem and pipe verts into memory
    if (vkMapMemory(device, vertMem, 0, sz, 0, &data) == VK_ERROR_MEMORY_MAP_FAILED)
    {
        errStr = "Failed to map vertecies to memory";
        return false;
    }
    memcpy(data, verts.data(), sz);

    // Unmap vertMem
    vkUnmapMemory(device, vertMem);

    return true;
}

bool vulkRend::uploadInd(const std::vector<int> &ind)
{
    // Check for empty vertex
    if (ind.empty())
    {
        errStr = "Index upload called with empty vector.";
        return false;
    }

    // Grab ind size in memory
    VkDeviceSize sz = sizeof(int) * ind.size();

    // Call buffer creation. If it fails, pass its error along
    if (!createBuffer(
            sz,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indBuff,
            indMem))
    {
        return false;
    }

    // Temp data pointer
    void *data = nullptr;

    // Map data to indMem and pipe verts into memory
    if (vkMapMemory(device, indMem, 0, sz, 0, &data) == VK_ERROR_MEMORY_MAP_FAILED)
    {
        errStr = "Failed to map indexes to memory";
        return false;
    }
    memcpy(data, ind.data(), sz);

    // Unmap indMem
    vkUnmapMemory(device, indMem);

    return true;
}

bool vulkRend::initPipe(const std::string &vertPath, const std::string &fragPath)
{
    // Read Spv files
    std::vector<char> vertCode = readFile(vertPath);
    std::vector<char> fragCode = readFile(fragPath);

    // If either read failed, error out
    if (vertCode.empty() || fragCode.empty())
    {
        errStr = "Unable to load 1 or more shader files";
        return false;
    }

    // Use loaded code to write shaders
    VkShaderModule vertShader = createShaderModule(device, vertCode);
    VkShaderModule fragShader = createShaderModule(device, fragCode);

    // Check modules were actually made
    if (vertShader == VK_NULL_HANDLE || fragShader == VK_NULL_HANDLE)
    {
        errStr = "Failed to create shader modules.";
        return false;
    }

    // Prepare shader staging
    // shaderStages[0]=vert, shaderStages[1]=frag
    VkPipelineShaderStageCreateInfo shaderStages[2]{};

    // Vert first
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShader;
    shaderStages[0].pName = "main";

    // Now frag
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShader;
    shaderStages[1].pName = "main";

    // Setup vertex binding
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(shapeVertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Setup vertex attributes
    std::vector<VkVertexInputAttributeDescription> attr(2);
    // pos -> attr[0]
    attr[0].binding = 0;
    attr[0].location = 0;
    attr[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attr[0].offset = offsetof(shapeVertex, pos);

    // alpha -> attr[1]
    attr[1].binding = 0;
    attr[1].location = 1;
    attr[1].format = VK_FORMAT_R32_SFLOAT;
    attr[1].offset = offsetof(shapeVertex, alpha);

    // Pipe attr into an input state
    VkPipelineVertexInputStateCreateInfo vertIn{};
    vertIn.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertIn.vertexBindingDescriptionCount = 1;
    vertIn.pVertexBindingDescriptions = &binding;
    vertIn.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
    vertIn.pVertexAttributeDescriptions = attr.data();

    // Build input assembly
    VkPipelineInputAssemblyStateCreateInfo inAssembly{};
    inAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inAssembly.primitiveRestartEnable = VK_FALSE;

    // Build viewport state
    VkPipelineViewportStateCreateInfo viewState{};
    viewState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewState.viewportCount = 1;
    viewState.scissorCount = 1;

    // Build rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Build multisampler
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Build color blend
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Build dynamic states
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Build pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    // Clear modules and report error on failure
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeLayout) != VK_SUCCESS)
    {
        errStr = "Failed to create pipeline layout.";
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        return false;
    }

    // Build pipeline info
    VkGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = 2;
    pipeInfo.pStages = shaderStages;
    pipeInfo.pVertexInputState = &vertIn;
    pipeInfo.pInputAssemblyState = &inAssembly;
    pipeInfo.pViewportState = &viewState;
    pipeInfo.pRasterizationState = &rasterizer;
    pipeInfo.pMultisampleState = &multisampling;
    pipeInfo.pColorBlendState = &colorBlending;
    pipeInfo.pDynamicState = &dynamicState;
    pipeInfo.layout = pipeLayout;
    pipeInfo.renderPass = rendPass;
    pipeInfo.subpass = 0;

    // Build graphics and catch and report errors
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeInfo, nullptr, &graphicsPipe) != VK_SUCCESS)
    {
        errStr = "Failed to create graphics pipeline.";
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        vkDestroyPipelineLayout(device, pipeLayout, nullptr);
        return false;
    }

    // Cleanup
    vkDestroyShaderModule(device, vertShader, nullptr);
    vkDestroyShaderModule(device, fragShader, nullptr);
    return true;
}

bool vulkRend::updateMesh(const std::vector<shapeVertex> &verts, const std::vector<int> &inds)
{
    // Check for empty inputs
    if (verts.empty() || inds.empty())
    {
        errStr = "Empty vertex/index data.";
        return false;
    }

    // Clear old mesh
    destroyMesh();

    // Upload inputs
    if (!uploadVert(verts))
    {
        destroyMesh();
        return false;
    }

    if (!uploadInd(inds))
    {
        destroyMesh();
        return false;
    }

    indexCount = inds.size();

    return true;
}

void vulkRend::recorDraw(VkCommandBuffer cmd, uint32_t width, uint32_t height)
{
    // If nothing to draw, do nothing
    if (graphicsPipe == VK_NULL_HANDLE || vertBuff == VK_NULL_HANDLE || indBuff == VK_NULL_HANDLE || indexCount == 0)
        return;

    // Setup viewports for custom renders
    VkViewport port{};
    port.x = 0.0f;
    port.y = 0.0f;
    port.width = static_cast<float>(width);
    port.height = static_cast<float>(height);
    port.minDepth = 0.0f;
    port.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {width, height};

    // Offset dummy
    VkDeviceSize offsets[] = {0};

    // Bind it all together
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipe);
    vkCmdSetViewport(cmd, 0, 1, &port);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertBuff, offsets);
    vkCmdBindIndexBuffer(cmd, indBuff, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}

void vulkRend::cleanup()
{
    // Clear mesh
    destroyMesh();

    // Clear graphics
    if (graphicsPipe != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, graphicsPipe, nullptr);
        graphicsPipe = VK_NULL_HANDLE;
    }

    // Clear layout
    if (pipeLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, pipeLayout, nullptr);
        pipeLayout = VK_NULL_HANDLE;
    }
}

std::string vulkRend::getErr() { return errStr; }

vulkRend::~vulkRend() { cleanup(); }
