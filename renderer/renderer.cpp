
#include "renderer.h"
#include "shaders.h"
#include <vulkan/vulkan.h>
#include <QVulkanInstance>
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>
#include <QVector3D>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>




namespace{
    //
    struct GpuVertex{
        Vertex position;
        Vertex normal;
    };
    struct alignas(16) DrawConstants{
        float viewProjection[16];
        float lightDirection[4];
        float lightColor[4];
        float baseColor[4];
        float lighting[4];
    };
    static_assert( sizeof( DrawConstants ) == 128 );
    //
    struct GpuMesh{
        int             meshId{0};
        Color           color;
        //
        VkBuffer        vbuf_{VK_NULL_HANDLE};
        VkDeviceMemory  vbufMem_{VK_NULL_HANDLE};
        uint32_t        vertexCount{0};
    };
}

/*
 */
struct Renderer::RendererImpl{
    VkInstance          instance_{};
    VkSurfaceKHR        surface_{};
    VkPhysicalDevice    physical_{};
    VkDevice            device_{};
    //
    QVulkanDeviceFunctions* deviceFuncs_{nullptr};
    QVulkanFunctions*       instanceFuncs_{nullptr};
    //
    VkPipeline          pipeline_{VK_NULL_HANDLE};
    VkPipelineLayout    pipelineLayout_{VK_NULL_HANDLE};
    VkPipelineCache     pipelineCache_{VK_NULL_HANDLE};
    //
    Shaders     vulkanShaders_{};
    std::vector<GpuMesh>   meshes_{};

    //
    void Init( QQuickWindow *window );
    void CreateVulkanPipeline( QQuickWindow *window );
    void CreateMeshBuffers( std::vector<MeshPtr>const& meshes );
    //
    void Draw( QQuickWindow *window, QPointF const& pos, QSizeF const& viewport, const Scene& scene );
    //
    ~RendererImpl();
};
//
Renderer::RendererImpl::~RendererImpl(){
    if ( !deviceFuncs_ ){
        return;
    }
    //
    deviceFuncs_->vkDestroyPipeline( device_, pipeline_, nullptr );
    deviceFuncs_->vkDestroyPipelineLayout( device_, pipelineLayout_, nullptr );
    deviceFuncs_->vkDestroyPipelineCache( device_, pipelineCache_, nullptr );
    for ( auto& mesh : meshes_ ){
        deviceFuncs_->vkDestroyBuffer( device_, mesh.vbuf_, nullptr );
        deviceFuncs_->vkFreeMemory( device_, mesh.vbufMem_, nullptr );
    }
}

void Renderer::RendererImpl::Init( QQuickWindow *window ){
    Q_ASSERT( window );
    //
    vulkanShaders_.init();
    //
    QSGRendererInterface* rif = window->rendererInterface();
    QVulkanInstance* instance = reinterpret_cast<QVulkanInstance*>( rif->getResource(window, QSGRendererInterface::VulkanInstanceResource) );
    Q_ASSERT(instance && instance->isValid() );
    instance_ = instance->vkInstance();
    surface_  = QVulkanInstance::surfaceForWindow( window );
    //
    physical_   = *reinterpret_cast<VkPhysicalDevice*>( rif->getResource(window, QSGRendererInterface::PhysicalDeviceResource) );
    device_     = *reinterpret_cast<VkDevice*>( rif->getResource(window, QSGRendererInterface::DeviceResource) );
    Q_ASSERT(physical_ && device_);
    //
    deviceFuncs_ = instance->deviceFunctions( device_ );
    instanceFuncs_ = instance->functions();
    Q_ASSERT(deviceFuncs_ && instanceFuncs_);
    //
    CreateVulkanPipeline( window );
}

void Renderer::RendererImpl::CreateMeshBuffers( std::vector<MeshPtr> const& meshes ){
    for ( auto& mesh : meshes ){
        if ( !mesh || mesh->vertices().empty() ){
            continue;
        }
        const auto vertices = mesh->vertices();
        if ( vertices.size() % 3 || vertices.size() > std::numeric_limits<uint32_t>::max() ){
            qWarning( "Skipping mesh with an invalid triangle-list vertex count" );
            continue;
        }
        std::vector<GpuVertex> gpuVertices;
        gpuVertices.reserve( vertices.size() );
        for ( std::size_t i = 0; i < vertices.size(); i += 3 ){
            const auto& a = vertices[i];
            const auto& b = vertices[i + 1];
            const auto& c = vertices[i + 2];
            const auto normal = QVector3D::crossProduct(
                QVector3D( b.x - a.x, b.y - a.y, b.z - a.z ),
                QVector3D( c.x - a.x, c.y - a.y, c.z - a.z ) ).normalized();
            for ( std::size_t j = 0; j < 3; ++j ){
                gpuVertices.push_back( { vertices[i + j], { normal.x(), normal.y(), normal.z() } } );
            }
        }
        GpuMesh newGpuMesh;
        newGpuMesh.color = mesh->color();
        newGpuMesh.meshId = mesh->meshId();
        newGpuMesh.vertexCount = mesh->vertices().size();
        //
        VkPhysicalDeviceMemoryProperties physDevMemProps;
        instanceFuncs_->vkGetPhysicalDeviceMemoryProperties( physical_, &physDevMemProps );
        VkPhysicalDeviceProperties physDevProps;
        instanceFuncs_->vkGetPhysicalDeviceProperties( physical_, &physDevProps );
        //
        VkBufferCreateInfo bufferInfo;
        memset(&bufferInfo, 0, sizeof(bufferInfo));
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = gpuVertices.size() * sizeof( GpuVertex );
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkResult err = deviceFuncs_->vkCreateBuffer(device_, &bufferInfo, nullptr, &newGpuMesh.vbuf_);
        if (err != VK_SUCCESS)
            qFatal("Failed to create vertex buffer: %d", err);

        VkMemoryRequirements memReq;
        deviceFuncs_->vkGetBufferMemoryRequirements(device_, newGpuMesh.vbuf_, &memReq);
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
        err = deviceFuncs_->vkAllocateMemory(device_, &allocInfo, nullptr, &newGpuMesh.vbufMem_);
        if (err != VK_SUCCESS)
            qFatal("Failed to allocate vertex buffer memory of size %u: %d", uint(allocInfo.allocationSize), err);

        void *p = nullptr;
        err = deviceFuncs_->vkMapMemory(device_, newGpuMesh.vbufMem_, 0, allocInfo.allocationSize, 0, &p);
        if (err != VK_SUCCESS || !p)
            qFatal("Failed to map vertex buffer memory: %d", err);
        memcpy( p, gpuVertices.data(), gpuVertices.size() * sizeof( GpuVertex ) );
        deviceFuncs_->vkUnmapMemory(device_, newGpuMesh.vbufMem_);
        err = deviceFuncs_->vkBindBufferMemory(device_, newGpuMesh.vbuf_, newGpuMesh.vbufMem_, 0);
        if (err != VK_SUCCESS)
            qFatal("Failed to bind vertex buffer memory: %d", err);
        //
        meshes_.push_back( newGpuMesh );
    }
}

void Renderer::RendererImpl::CreateVulkanPipeline( QQuickWindow *window ){
    Q_ASSERT( window );
    //
    QSGRendererInterface *rif = window->rendererInterface();
    VkRenderPass rp = *reinterpret_cast<VkRenderPass *>( rif->getResource(window, QSGRendererInterface::RenderPassResource) );
    Q_ASSERT(rp);
    //
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = deviceFuncs_->vkCreatePipelineCache( device_, &pipelineCacheInfo, nullptr, &pipelineCache_);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline cache: %d", err);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkPushConstantRange pushConstants{};
    pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstants.size = sizeof( DrawConstants );
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstants;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    err = deviceFuncs_->vkCreatePipelineLayout( device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", err);

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
        sizeof(GpuVertex), // stride
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    VkVertexInputAttributeDescription vertexAttributes[] = {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( GpuVertex, position ) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( GpuVertex, normal ) }
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    memset(&vertexInputInfo, 0, sizeof(vertexInputInfo));
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBinding;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes;
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
    iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.pInputAssemblyState = &iaInfo;

    VkPipelineRasterizationStateCreateInfo rsInfo;
    memset(&rsInfo, 0, sizeof(rsInfo));
    rsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rsInfo.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rsInfo;

    VkPipelineMultisampleStateCreateInfo msInfo;
    memset(&msInfo, 0, sizeof(msInfo));
    msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msInfo.rasterizationSamples = static_cast<VkSampleCountFlagBits>( std::max( 1, window->format().samples() ) );
    pipelineInfo.pMultisampleState = &msInfo;

    VkPipelineDepthStencilStateCreateInfo dsInfo;
    memset(&dsInfo, 0, sizeof(dsInfo));
    dsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    dsInfo.depthTestEnable = VK_TRUE;
    dsInfo.depthWriteEnable = VK_TRUE;
    dsInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    pipelineInfo.pDepthStencilState = &dsInfo;

    // SrcAlpha, One
    VkPipelineColorBlendStateCreateInfo blendInfo;
    memset(&blendInfo, 0, sizeof(blendInfo));
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState blend;
    memset(&blend, 0, sizeof(blend));
    blend.blendEnable = false;
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

void Renderer::RendererImpl::Draw( QQuickWindow *window, QPointF const& pos, QSizeF const& viewport, const Scene& scene ){
    if ( viewport.width() <= 0 || viewport.height() <= 0 ){
        return;
    }
    QSGRendererInterface* rif = window->rendererInterface();
    Q_ASSERT( rif );
    //
    VkCommandBuffer cb = *reinterpret_cast<VkCommandBuffer*>( rif->getResource(window, QSGRendererInterface::CommandListResource) );
    Q_ASSERT(cb);
    //
    deviceFuncs_->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ );
    //
    VkViewport vp = { float(pos.x()), float(pos.y()), float(viewport.width()), float(viewport.height()), 0.0f, 1.0f };
    deviceFuncs_->vkCmdSetViewport(cb, 0,1, &vp);
    //
    const auto extent = window->size() * window->devicePixelRatio();
    const int left = std::max( 0, static_cast<int>( std::floor( pos.x() ) ) );
    const int top = std::max( 0, static_cast<int>( std::floor( pos.y() ) ) );
    const int right = std::min( extent.width(), static_cast<int>( std::ceil( pos.x() + viewport.width() ) ) );
    const int bottom = std::min( extent.height(), static_cast<int>( std::ceil( pos.y() + viewport.height() ) ) );
    if ( right <= left || bottom <= top ){
        return;
    }
    VkRect2D scissor{ { left, top }, { uint32_t( right - left ), uint32_t( bottom - top ) } };
    deviceFuncs_->vkCmdSetScissor( cb, 0, 1, &scissor );
    //
    // The custom overlay shares Qt's pass; clear only its own viewport's depth.
    VkClearAttachment depth{};
    depth.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth.clearValue.depthStencil.depth = 1.f;
    VkClearRect clearRect{ scissor, 0, 1 };
    deviceFuncs_->vkCmdClearAttachments( cb, 1, &depth, 1, &clearRect );
    //
    DrawConstants constants{};
    const auto viewProjection = scene.camera().viewProjection( viewport.width() / viewport.height() );
    std::memcpy( constants.viewProjection, viewProjection.constData(), sizeof( constants.viewProjection ) );
    const auto& light = scene.light();
    const auto direction = light._dir.isNull() ? QVector3D( 0.f, 0.f, -1.f ) : light._dir.normalized();
    constants.lightDirection[0] = direction.x();
    constants.lightDirection[1] = direction.y();
    constants.lightDirection[2] = direction.z();
    constants.lightColor[0] = light._col.r;
    constants.lightColor[1] = light._col.g;
    constants.lightColor[2] = light._col.b;
    constants.lighting[0] = std::max( 0.f, light._strength );
    constants.lighting[1] = std::max( 0.f, light._ambient );
    //
    for ( auto& mesh : meshes_ ){
        constants.baseColor[0] = mesh.color.r;
        constants.baseColor[1] = mesh.color.g;
        constants.baseColor[2] = mesh.color.b;
        constants.baseColor[3] = 1.f;
        deviceFuncs_->vkCmdPushConstants( cb, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( constants ), &constants );
        VkDeviceSize vbufOffset = 0;
        deviceFuncs_->vkCmdBindVertexBuffers(cb, 0, 1, &mesh.vbuf_, &vbufOffset);
        //
        deviceFuncs_->vkCmdDraw(cb, mesh.vertexCount, 1, 0, 0 );
    }
}

/*
 */
void Renderer::frameStart(){
    QSGRendererInterface* rif = window_->rendererInterface();
    Q_ASSERT( rif->graphicsApi() == QSGRendererInterface::Vulkan );

    if ( !initialized_ ){
        impl_.reset( new Renderer::RendererImpl() );
        impl_->Init( window_ );
        //
        scene_.Init();
        impl_->CreateMeshBuffers( scene_.Meshes() );
        initialized_ = true;
    }
}

void Renderer::mainPassRecordingStart(){
    const QQuickWindow::GraphicsStateInfo &stateInfo( window_->graphicsStateInfo() );
    //
    window_->beginExternalCommands();
    //
    impl_->Draw( window_, pos_, viewportSize_, scene_ );
    //
    window_->endExternalCommands();
}
