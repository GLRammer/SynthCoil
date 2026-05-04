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
    updateUB(defaultUB());
    std::vector<std::pair<float, float>> tempVec;
    tempVec.push_back(std::pair<float, float>(0.0f, 0.0f));
    updateFreqs(tempVec);
    initChroma();
}

bool vulkRend::chromaPass()
{
    // Check for color transition
    transition = false;
    for (int i = 0; i < 3; i++)
    {
        if (chroma[i] != targChroma[i])
        {
            transition = true;
            chroma[i] = chromaMath(chroma[i], targChroma[i]);
        }
    }

    // grab size in memory
    VkDeviceSize sz = sizeof(float) * 3;

    // temp memory mapping
    void *data = nullptr;
    if (vkMapMemory(device, chromaMem, 0, sz, 0, &data) != VK_SUCCESS)
    {
        errStr = "Failed to map color to memory";
        return false;
    }

    // copy data over
    memcpy(data, chroma, sz);

    // unmap and exit
    vkUnmapMemory(device, chromaMem);
    return true;
}

float vulkRend::chromaMath(float a, float b)
{
    float diff = fabs(a - b);
    float max = std::max(a, b);
    // If difference is less than 1%, just set them equal
    if (diff / max < 0.01)
    {
        return b;
    }
    // Set to average of values
    return (a + b) / 2.0;
}

bool vulkRend::initChroma()
{
    // Grab buffer size
    VkDeviceSize sz = sizeof(float) * 3;

    // Check for nulls
    if (chromaBuff != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, chromaBuff, nullptr);
        chromaBuff = VK_NULL_HANDLE;
    }
    if (chromaMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, chromaMem, nullptr);
        chromaMem = VK_NULL_HANDLE;
    }

    // Create buffer
    if (!createBuffer(sz, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, chromaBuff, chromaMem))
    {
        return false;
    }

    // temp memory mapping
    void *data = nullptr;
    if (vkMapMemory(device, chromaMem, 0, sz, 0, &data) != VK_SUCCESS)
    {
        errStr = "Failed to map color to memory";
        return false;
    }

    // copy data over
    memcpy(data, chroma, sz);

    // unmap and exit
    vkUnmapMemory(device, chromaMem);
    return true;
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
        vkDestroyBuffer(device, buff, nullptr);
        mem = VK_NULL_HANDLE;
        buff = VK_NULL_HANDLE;
        errStr = "Failed memory allocation.";
        return false;
    }

    // Check for failure while binding memory and buffer
    if (vkBindBufferMemory(device, buff, mem, 0))
    {
        vkFreeMemory(device, mem, nullptr);
        vkDestroyBuffer(device, buff, nullptr);
        mem = VK_NULL_HANDLE;
        buff = VK_NULL_HANDLE;
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
    if (vkMapMemory(device, vertMem, 0, sz, 0, &data) != VK_SUCCESS)
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
    if (vkMapMemory(device, indMem, 0, sz, 0, &data) != VK_SUCCESS)
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

    VkDescriptorSetLayoutBinding bindings[3];

    // Uniform buffer setup for 3d
    VkDescriptorSetLayoutBinding ubBinding{};
    ubBinding.binding = 0;
    ubBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubBinding.descriptorCount = 1;
    ubBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubBinding.pImmutableSamplers = nullptr;
    bindings[0] = ubBinding;

    // Frequency buffer setup for GPU side generation
    VkDescriptorSetLayoutBinding freqBind{};
    freqBind.binding = 1;
    freqBind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    freqBind.descriptorCount = 1;
    freqBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    freqBind.pImmutableSamplers = nullptr;
    bindings[1] = freqBind;

    // Color buffer
    VkDescriptorSetLayoutBinding chromaBind{};
    chromaBind.binding = 2;
    chromaBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    chromaBind.descriptorCount = 1;
    chromaBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    chromaBind.pImmutableSamplers = nullptr;
    bindings[2] = chromaBind;

    VkDescriptorSetLayoutCreateInfo descLayoutInfo{};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.bindingCount = 3;
    descLayoutInfo.pBindings = bindings;

    // Clear modules and report error on failure
    if (vkCreateDescriptorSetLayout(device, &descLayoutInfo, nullptr, &descLayout) != VK_SUCCESS)
    {
        errStr = "Failed to create descriptor layout.";
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        return false;
    }

    // Descriptor pool setup
    VkDescriptorPoolSize poolSize[3]{};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = 1;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize[1].descriptorCount = 1;
    poolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[2].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 3;
    poolInfo.pPoolSizes = poolSize;
    poolInfo.maxSets = 1;

    // Clear modules and report error on failure
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descPool) != VK_SUCCESS)
    {
        errStr = "Failed to create descriptor pool.";
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        return false;
    }

    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descLayout;

    // Clear modules and report error on failure
    if (vkAllocateDescriptorSets(device, &allocInfo, &descSet) != VK_SUCCESS)
    {
        errStr = "Failed to allocate descriptor set.";
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        return false;
    }

    // Write buffers into descriptor set
    VkDescriptorBufferInfo uBInfo{};
    uBInfo.buffer = uB;
    uBInfo.offset = 0;
    uBInfo.range = sizeof(uniformBuffer);

    VkDescriptorBufferInfo chromaInfo{};
    chromaInfo.buffer = chromaBuff;
    chromaInfo.offset = 0;
    chromaInfo.range = sizeof(float) * 3;

    VkDescriptorBufferInfo freqBInfo{};
    freqBInfo.buffer = freqBuff;
    freqBInfo.offset = 0;
    freqBInfo.range = sizeof(float) * (maxFreqCnt * 2 + 2);

    VkWriteDescriptorSet descWrite[3]{};
    descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[0].dstSet = descSet;
    descWrite[0].dstBinding = 0;
    descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descWrite[0].descriptorCount = 1;
    descWrite[0].pBufferInfo = &uBInfo;

    descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[1].dstSet = descSet;
    descWrite[1].dstBinding = 1;
    descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descWrite[1].descriptorCount = 1;
    descWrite[1].pBufferInfo = &freqBInfo;

    descWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[2].dstSet = descSet;
    descWrite[2].dstBinding = 2;
    descWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descWrite[2].descriptorCount = 1;
    descWrite[2].pBufferInfo = &chromaInfo;

    // void return, so no safety here
    vkUpdateDescriptorSets(device, 3, descWrite, 0, nullptr);

    // Build pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descLayout;

    // Clear modules and report error on failure
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeLayout) != VK_SUCCESS)
    {
        errStr = "Failed to create pipeline layout.";
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        vkDestroyDescriptorSetLayout(device, descLayout, nullptr);
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

bool vulkRend::updateFreqs(const std::vector<std::pair<float, float>> &freqs)
{
    // Handle empty input
    if (freqs.size() == 0)
    {
        errStr = "Empty frequency vector provided";
        return false;
    }

    // Grab freqs size in memory
    VkDeviceSize sz = sizeof(float) * (freqs.size() * 2 + 2);

    // Call buffer creation. If it fails, pass its error along
    if (freqBuff == VK_NULL_HANDLE || freqMem == VK_NULL_HANDLE || freqs.size() > maxFreqCnt)
    {
        if (freqs.size() > maxFreqCnt)
        {
            errStr = "Provided frequency vector exceeds max";
            return false;
        }
        if (!initFreqs())
        {
            return false;
        }
    }

    // temp data pointer
    void *data = nullptr;

    // Map data to freqMem and pipe freqs into memory
    if (vkMapMemory(device, freqMem, 0, sz, 0, &data) != VK_SUCCESS)
    {
        errStr = "Failed to map frequencies to memory";
        return false;
    }

    // Temp buffer
    std::vector<float> tempVec;
    tempVec.reserve(2 * freqs.size() + 2);

    // Store smoothing value and buffer size
    tempVec.push_back(smooth);
    tempVec.push_back(freqs.size() * 2 + 2);

    // Store inputs
    for (int i = 0; i < freqs.size(); i++)
    {
        tempVec.push_back(freqs[i].first);
        tempVec.push_back(freqs[i].second);
    }

    //  Copy buffer to memory
    memcpy(data, tempVec.data(), sz);

    // Unmap freqMem
    vkUnmapMemory(device, freqMem);

    // return false;
    return true;
}

bool vulkRend::initFreqs()
{
    // Check for nulls
    if (freqBuff != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, freqBuff, nullptr);
        freqBuff = VK_NULL_HANDLE;
    }
    if (freqMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, freqMem, nullptr);
        freqMem = VK_NULL_HANDLE;
    }

    // Store buffer size
    int sz = sizeof(float) * ((2 * maxFreqCnt) + 2);

    // Make buffer
    if (!createBuffer(
            sz,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            freqBuff,
            freqMem))
    {
        return false;
    }
    return true;
}

bool vulkRend::initUB()
{
    // Store buffer size
    VkDeviceSize sz = sizeof(uniformBuffer);

    // Check for nulls
    if (uBMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, uBMem, nullptr);
        uBMem = VK_NULL_HANDLE;
    }
    if (uB != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, uB, nullptr);
        uB = VK_NULL_HANDLE;
    }

    // Create buffer
    if (!createBuffer(
            sz,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uB,
            uBMem))
    {
        return false;
    }
    return true;
}

void vulkRend::updateSmooth(float newSmooth)
{
    smooth = newSmooth;
    if (smooth > 1)
        smooth = 1;
    if (smooth < 0)
        smooth = 0;
}

bool vulkRend::updateUB(const uniformBuffer &uBuff)
{
    // Check for empty buffer
    if (uBuff.isEmpty())
    {
        errStr = "Empty Uniform buffer provided.";
        return false;
    }

    // Grab ind size in memory
    VkDeviceSize sz = sizeof(uBuff);

    // Call buffer creation. If it fails, pass its error along
    if (uB == VK_NULL_HANDLE || uBMem == VK_NULL_HANDLE)
    {
        if (!initUB())
        {
            return false;
        }
    }

    // Temp data pointer
    void *data = nullptr;

    // Map data to uBMem and pipe buffer into memory
    if (vkMapMemory(device, uBMem, 0, sz, 0, &data) != VK_SUCCESS)
    {
        errStr = "Failed to map uniform buffer to memory";
        return false;
    }
    memcpy(data, &uBuff, sz);

    // Unmap indMem
    vkUnmapMemory(device, uBMem);

    // Store buffer for later comparisons
    lastUB = uBuff;

    return true;
}

const uniformBuffer vulkRend::getUB() { return lastUB; }

bool vulkRend::updateColor(float r, float g, float b)
{
    float temp[3] = {r, g, b};
    return updateColor(temp);
}

bool vulkRend::updateColor(float rgb[3])
{
    // Check if data is new
    bool eq = true;
    for (int i = 0; i < 3; i++)
    {
        if (rgb[i] != targChroma[i])
        {
            eq = false;
            // Set target color
            targChroma[i] = rgb[i];
        }
    }

    // Handle empty buffer
    if (chromaBuff == VK_NULL_HANDLE | chromaMem == VK_NULL_HANDLE)
    {
        if (!initChroma())
        {
            return false;
        }
        eq = false;
    }

    if (!eq)
    {
        if (!chromaPass())
        {
            return false;
        }
        transition = true;
    }
    return true;
}

void vulkRend::recorDraw(VkCommandBuffer cmd, uint32_t width, uint32_t height)
{
    // If nothing to draw, do nothing
    if (graphicsPipe == VK_NULL_HANDLE || vertBuff == VK_NULL_HANDLE || indBuff == VK_NULL_HANDLE || indexCount == 0)
        return;

    // Check for chroma transition
    if (transition)
    {
        chromaPass();
    }

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
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeLayout, 0, 1, &descSet, 0, nullptr);
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
    if (descLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, descLayout, nullptr);
        descLayout = VK_NULL_HANDLE;
    }
    if (freqBuff != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, freqBuff, nullptr);
        freqBuff = VK_NULL_HANDLE;
    }
    if (uB != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, uB, nullptr);
        uB = VK_NULL_HANDLE;
    }
    if (freqMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, freqMem, nullptr);
        freqMem = VK_NULL_HANDLE;
    }
    if (uBMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, uBMem, nullptr);
        uBMem = VK_NULL_HANDLE;
    }
    if (descPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descPool, nullptr);
        descPool = VK_NULL_HANDLE;
    }
    if (chromaBuff != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, chromaBuff, nullptr);
        chromaBuff = VK_NULL_HANDLE;
    }
    if (chromaMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, chromaMem, nullptr);
        chromaMem = VK_NULL_HANDLE;
    }
}

std::string vulkRend::getErr() { return errStr; }

vulkRend::~vulkRend() { cleanup(); }

uniformBuffer defaultUB()
{
    uniformBuffer ub{};
    ub.model = glm::mat4(1.0f);
    ub.model = glm::rotate(ub.model, 0.75f, glm::vec3(1, 0, 0));
    ub.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 15.0f),
                          glm::vec3(0.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));
    ub.proj = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
    ub.scale = 5.0;
    return ub;
}