
#include "renderer.h"
#include <QVulkanInstance>
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>




namespace{
    //
    static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
    {
        return (v + byteAlign - 1) & ~(byteAlign - 1);
    }
    //
    static const float vertices[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };
    //
    const int UBUF_SIZE = 4;
}

Renderer::Renderer()
    :QObject()
    ,instance_()
    ,surface_()
{
}

Renderer::~Renderer(){
    if ( !deviceFuncs_ ){
        return;
    }
    //
    deviceFuncs_->vkDestroyPipeline( device_, pipeline_, nullptr );
    deviceFuncs_->vkDestroyPipelineLayout( device_, pipelineLayout_, nullptr );
    deviceFuncs_->vkDestroyDescriptorSetLayout( device_, descriptorSetLayout_, nullptr );
    deviceFuncs_->vkDestroyDescriptorPool( device_, descriptorPool_, nullptr );
    deviceFuncs_->vkDestroyPipelineCache( device_, pipelineCache_, nullptr );
    deviceFuncs_->vkDestroyBuffer( device_, vbuf_, nullptr );
    deviceFuncs_->vkFreeMemory( device_, vbufMem_, nullptr );
    deviceFuncs_->vkDestroyBuffer( device_, ubuf_, nullptr );
    deviceFuncs_->vkFreeMemory( device_, ubufMem_, nullptr );
}

void Renderer::frameStart(){
    QSGRendererInterface* rif = window_->rendererInterface();
    Q_ASSERT( rif->graphicsApi() == QSGRendererInterface::Vulkan );

    if ( !initialized_ ){
        vulkanShaders_.init();
        Init( window_->graphicsStateInfo().framesInFlight );
    }
}

void Renderer::mainPassRecordingStart(){
    const QQuickWindow::GraphicsStateInfo &stateInfo( window_->graphicsStateInfo() );
    QSGRendererInterface* rif = window_->rendererInterface();
    //
    VkDeviceSize ubufOffset = stateInfo.currentFrameSlot * allocPerUBuf_;
    void *p = nullptr;
    VkResult err = deviceFuncs_->vkMapMemory( device_, ubufMem_, ubufOffset, allocPerUBuf_, 0, &p);
    if (err != VK_SUCCESS || !p)
        qFatal("Failed to map uniform buffer memory: %d", err);
    deviceFuncs_->vkUnmapMemory( device_, ubufMem_ );

    /***********/
    window_->beginExternalCommands();
    //
    VkCommandBuffer cb = *reinterpret_cast<VkCommandBuffer*>( rif->getResource(window_, QSGRendererInterface::CommandListResource) );
    Q_ASSERT(cb);
    //
    deviceFuncs_->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ );
    //
    VkDeviceSize vbufOffset = 0;
    deviceFuncs_->vkCmdBindVertexBuffers(cb, 0, 1, &vbuf_, &vbufOffset);
    //
    uint32_t dynamicOffset = allocPerUBuf_ * stateInfo.currentFrameSlot;
    deviceFuncs_->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &descriptorSet_, 1, &dynamicOffset);
    //
    VkViewport vp = {0, 0, float(viewportSize_.width()), float(viewportSize_.height()), 0.0f, 1.0f};
    deviceFuncs_->vkCmdSetViewport(cb, 0,1, &vp);
    VkRect2D scissor = { {0,0}, {uint32_t(viewportSize_.width()), uint32_t(viewportSize_.height())} };
    deviceFuncs_->vkCmdSetScissor(cb, 0, 1, &scissor);
    //
    deviceFuncs_->vkCmdDraw(cb, 4, 1, 0, 0 );
    //
    window_->endExternalCommands();
    /***********/
}

void Renderer::Init(int framesInFlight){
    //
    initialized_ = true;
    QSGRendererInterface* rif = window_->rendererInterface();
    QVulkanInstance* instance = reinterpret_cast<QVulkanInstance*>( rif->getResource(window_, QSGRendererInterface::VulkanInstanceResource) );
    Q_ASSERT(instance && instance->isValid() );
    instance_ = instance->vkInstance();
    surface_  = QVulkanInstance::surfaceForWindow( window_ );
    //
    physical_   = *reinterpret_cast<VkPhysicalDevice*>( rif->getResource(window_, QSGRendererInterface::PhysicalDeviceResource) );
    device_     = *reinterpret_cast<VkDevice*>( rif->getResource(window_, QSGRendererInterface::DeviceResource) );
    Q_ASSERT(physical_ && device_);
    //
    deviceFuncs_ = instance->deviceFunctions( device_ );
    instanceFuncs_ = instance->functions();
    Q_ASSERT(deviceFuncs_ && instanceFuncs_);
    //
    CreateVulkanBuffers( framesInFlight );
    CreateVulkanPipeline();
}

void Renderer::CreateVulkanBuffers(int framesInFlight){
    //
    VkPhysicalDeviceMemoryProperties physDevMemProps;
    instanceFuncs_->vkGetPhysicalDeviceMemoryProperties( physical_, &physDevMemProps );
    VkPhysicalDeviceProperties physDevProps;
    instanceFuncs_->vkGetPhysicalDeviceProperties( physical_, &physDevProps );
    //
    allocPerUBuf_ = aligned(UBUF_SIZE, physDevProps.limits.minUniformBufferOffsetAlignment);

    VkBufferCreateInfo bufferInfo;
    memset(&bufferInfo, 0, sizeof(bufferInfo));
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkResult err = deviceFuncs_->vkCreateBuffer(device_, &bufferInfo, nullptr, &vbuf_);
    if (err != VK_SUCCESS)
        qFatal("Failed to create vertex buffer: %d", err);

    VkMemoryRequirements memReq;
    deviceFuncs_->vkGetBufferMemoryRequirements(device_, vbuf_, &memReq);
    VkMemoryAllocateInfo allocInfo;
    memset(&allocInfo, 0, sizeof(allocInfo));
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;

    uint32_t memTypeIndex = uint32_t(-1);
    const VkMemoryType *memType = physDevMemProps.memoryTypes;
    for (uint32_t i = 0; i < physDevMemProps.memoryTypeCount; ++i) {
        if (memReq.memoryTypeBits & (1 << i)) {
            if ((memType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                && (memType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                memTypeIndex = i;
                break;
            }
        }
    }
    if (memTypeIndex == uint32_t(-1))
        qFatal("Failed to find host visible and coherent memory type");

    allocInfo.memoryTypeIndex = memTypeIndex;
    err = deviceFuncs_->vkAllocateMemory(device_, &allocInfo, nullptr, &vbufMem_);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate vertex buffer memory of size %u: %d", uint(allocInfo.allocationSize), err);

    void *p = nullptr;
    err = deviceFuncs_->vkMapMemory(device_, vbufMem_, 0, allocInfo.allocationSize, 0, &p);
    if (err != VK_SUCCESS || !p)
        qFatal("Failed to map vertex buffer memory: %d", err);
    memcpy(p, vertices, sizeof(vertices));
    deviceFuncs_->vkUnmapMemory(device_, vbufMem_);
    err = deviceFuncs_->vkBindBufferMemory(device_, vbuf_, vbufMem_, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind vertex buffer memory: %d", err);


    //
    memset(&bufferInfo, 0, sizeof(bufferInfo));
    bufferInfo.size = framesInFlight * allocPerUBuf_;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    err = deviceFuncs_->vkCreateBuffer( device_, &bufferInfo, nullptr, &ubuf_);
    if (err != VK_SUCCESS)
        qFatal("Failed to create uniform buffer: %d", err);
    deviceFuncs_->vkGetBufferMemoryRequirements( device_, ubuf_, &memReq);
    memTypeIndex = uint32_t(-1);
    for (uint32_t i = 0; i < physDevMemProps.memoryTypeCount; ++i) {
        if (memReq.memoryTypeBits & (1 << i)) {
            if ((memType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                && (memType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                memTypeIndex = i;
                break;
            }
        }
    }
    if (memTypeIndex == uint32_t(-1))
        qFatal("Failed to find host visible and coherent memory type");

    memset(&allocInfo, 0, sizeof(allocInfo));
    allocInfo.allocationSize = framesInFlight * allocPerUBuf_;
    allocInfo.memoryTypeIndex = memTypeIndex;
    err = deviceFuncs_->vkAllocateMemory( device_, &allocInfo, nullptr, &ubufMem_);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate uniform buffer memory of size %u: %d", uint(allocInfo.allocationSize), err);

    err = deviceFuncs_->vkBindBufferMemory( device_, ubuf_, ubufMem_, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind uniform buffer memory: %d", err);
}

void Renderer::CreateVulkanPipeline(){
    //
    QSGRendererInterface *rif = window_->rendererInterface();
    VkRenderPass rp = *reinterpret_cast<VkRenderPass *>( rif->getResource(window_, QSGRendererInterface::RenderPassResource) );
    Q_ASSERT(rp);
    //
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = deviceFuncs_->vkCreatePipelineCache( device_, &pipelineCacheInfo, nullptr, &pipelineCache_);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline cache: %d", err);

    VkDescriptorSetLayoutBinding descLayoutBinding;
    memset(&descLayoutBinding, 0, sizeof(descLayoutBinding));
    descLayoutBinding.binding = 0;
    descLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descLayoutBinding.descriptorCount = 1;
    descLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo layoutInfo;
    memset(&layoutInfo, 0, sizeof(layoutInfo));
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &descLayoutBinding;
    err = deviceFuncs_->vkCreateDescriptorSetLayout( device_, &layoutInfo, nullptr, &descriptorSetLayout_);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor set layout: %d", err);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    err = deviceFuncs_->vkCreatePipelineLayout( device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_);
    if (err != VK_SUCCESS)
        qWarning("Failed to create pipeline layout: %d", err);

    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    //
    Shader vertexShader = vulkanShaders_.shader( Shaders::VERTEX, 0 );
    VkShaderModuleCreateInfo shaderInfo;
    memset(&shaderInfo, 0, sizeof(shaderInfo));
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = vertexShader.size();
    shaderInfo.pCode = reinterpret_cast<const quint32 *>(vertexShader.constData());
    VkShaderModule vertShaderModule;
    err = deviceFuncs_->vkCreateShaderModule( device_, &shaderInfo, nullptr, &vertShaderModule);
    if (err != VK_SUCCESS)
        qFatal("Failed to create vertex shader module: %d", err);

    //
    Shader fragmentShader = vulkanShaders_.shader( Shaders::FRAG, 0 );
    shaderInfo.codeSize = fragmentShader.size();
    shaderInfo.pCode = reinterpret_cast<const quint32 *>(fragmentShader.constData());
    VkShaderModule fragShaderModule;
    err = deviceFuncs_->vkCreateShaderModule( device_, &shaderInfo, nullptr, &fragShaderModule);
    if (err != VK_SUCCESS)
        qFatal("Failed to create fragment shader module: %d", err);

    VkPipelineShaderStageCreateInfo stageInfo[2];
    memset(&stageInfo, 0, sizeof(stageInfo));
    stageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stageInfo[0].module = vertShaderModule;
    stageInfo[0].pName = "main";
    stageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stageInfo[1].module = fragShaderModule;
    stageInfo[1].pName = "main";
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stageInfo;

    VkVertexInputBindingDescription vertexBinding = {
        0, // binding
        2 * sizeof(float), // stride
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    VkVertexInputAttributeDescription vertexAttr = {
        0, // location
        0, // binding
        VK_FORMAT_R32G32_SFLOAT, // 'vertices' only has 2 floats per vertex
        0 // offset
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    memset(&vertexInputInfo, 0, sizeof(vertexInputInfo));
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBinding;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &vertexAttr;
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    memset(&dynamicInfo, 0, sizeof(dynamicInfo));
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = 2;
    dynamicInfo.pDynamicStates = dynStates;
    pipelineInfo.pDynamicState = &dynamicInfo;

    VkPipelineViewportStateCreateInfo viewportInfo;
    memset(&viewportInfo, 0, sizeof(viewportInfo));
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = viewportInfo.scissorCount = 1;
    pipelineInfo.pViewportState = &viewportInfo;

    VkPipelineInputAssemblyStateCreateInfo iaInfo;
    memset(&iaInfo, 0, sizeof(iaInfo));
    iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pipelineInfo.pInputAssemblyState = &iaInfo;

    VkPipelineRasterizationStateCreateInfo rsInfo;
    memset(&rsInfo, 0, sizeof(rsInfo));
    rsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rsInfo.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rsInfo;

    VkPipelineMultisampleStateCreateInfo msInfo;
    memset(&msInfo, 0, sizeof(msInfo));
    msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &msInfo;

    VkPipelineDepthStencilStateCreateInfo dsInfo;
    memset(&dsInfo, 0, sizeof(dsInfo));
    dsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineInfo.pDepthStencilState = &dsInfo;

    // SrcAlpha, One
    VkPipelineColorBlendStateCreateInfo blendInfo;
    memset(&blendInfo, 0, sizeof(blendInfo));
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState blend;
    memset(&blend, 0, sizeof(blend));
    blend.blendEnable = true;
    blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blend.colorBlendOp = VK_BLEND_OP_ADD;
    blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend.alphaBlendOp = VK_BLEND_OP_ADD;
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendInfo.attachmentCount = 1;
    blendInfo.pAttachments = &blend;
    pipelineInfo.pColorBlendState = &blendInfo;
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = rp;

    err = deviceFuncs_->vkCreateGraphicsPipelines( device_, pipelineCache_, 1, &pipelineInfo, nullptr, &pipeline_);
    //
    deviceFuncs_->vkDestroyShaderModule( device_, vertShaderModule, nullptr);
    deviceFuncs_->vkDestroyShaderModule( device_, fragShaderModule, nullptr);

    if (err != VK_SUCCESS)
        qFatal("Failed to create graphics pipeline: %d", err);
}
