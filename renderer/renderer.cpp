
#include "renderer.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>



namespace{
    //
    std::filesystem::path executablePath() {
        return std::filesystem::read_symlink("/proc/self/exe");
    }
    //
    template<class T>
    [[nodiscard]] T structure( VkStructureType type ){
        T value{};
        value.sType = type;
        return value;
    }
    //
    inline void check(VkResult result, const char* operation) {
        if ( result != VK_SUCCESS ){
            throw std::runtime_error(std::string(operation) + " failed (VkResult " + std::to_string(result) + ")");
        }
    }
}

Renderer::Renderer()
    :QObject()
    ,instance_()
    ,surface_()
{
}

Renderer::~Renderer(){

}

void Renderer::frameStart(){

}
void Renderer::mainPassRecordingStart(){

}

void Renderer::Init(){
    /*
    //
    instance_.setApiVersion(    QVersionNumber(1, 3, 0) );
    instance_.setFlags(         QVulkanInstance::NoDebugOutputRedirect );
    instance_.setExtensions(    {VK_EXT_DEBUG_UTILS_EXTENSION_NAME} );
    if ( !instance_.create() ){
        std::cout << "Could not create vulkan instance" << std::endl;
    }
    //
    setSurfaceType(VulkanSurface);
    setVulkanInstance( &instance_ );
    //
    const auto surface = QVulkanInstance::surfaceForWindow(this);
    const auto shaders = executablePath().parent_path() / "shaders";

    renderer_ = std::make_unique<Renderer>( vulkanInstance()->vkInstance(), surface, shaders );
    */
}

void Renderer::CreateDevice(){
    std::uint32_t count{};
    check( vkEnumeratePhysicalDevices( instance_, &count, nullptr ), "Enumerate GPUs" );
    std::vector<VkPhysicalDevice> devices(count);
    check( vkEnumeratePhysicalDevices( instance_, &count, devices.data() ), "Enumerate GPUs" );
    int best = -1;
    for (const auto candidate : devices) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties( candidate, &properties );
        if ( properties.apiVersion < VK_API_VERSION_1_3 ){
            std::cout << "No sufficient Vulkan support." << std::endl;
            continue;
        }
        //
        std::uint32_t extensionCount{};
        check( vkEnumerateDeviceExtensionProperties( candidate, nullptr, &extensionCount, nullptr ), "Enumerate GPU extensions" );
        std::vector<VkExtensionProperties> extensions(extensionCount);
        check( vkEnumerateDeviceExtensionProperties( candidate, nullptr, &extensionCount, extensions.data() ), "Enumerate GPU extensions" );
        //
        const auto has = [&](const char* propertyName){
            return std::any_of( extensions.begin(), extensions.end(), [propertyName](const auto& extension) { return std::strcmp(extension.extensionName, propertyName) == 0; });
        };
        //
        if ( !has(VK_KHR_SWAPCHAIN_EXTENSION_NAME) || !has(VK_EXT_MESH_SHADER_EXTENSION_NAME) ){
            std::cout << "requires VK_KHR_swapchain and VK_EXT_mesh_shader; no legacy fallback" << std::endl;
            continue;
        }
        //
        auto v11    = structure<VkPhysicalDeviceVulkan11Features        >(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES);
        auto v12    = structure<VkPhysicalDeviceVulkan12Features        >(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES);
        auto v13    = structure<VkPhysicalDeviceVulkan13Features        >(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES);
        auto mesh   = structure<VkPhysicalDeviceMeshShaderFeaturesEXT   >(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT);
        auto features = structure<VkPhysicalDeviceFeatures2>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
        features.pNext = &v11; v11.pNext = &v12; v12.pNext = &v13; v13.pNext = &mesh;
        vkGetPhysicalDeviceFeatures2( candidate, &features );
        if ( !mesh.meshShader || !mesh.taskShader || !v11.shaderDrawParameters || !v13.dynamicRendering || !v13.synchronization2 || !v13.shaderDemoteToHelperInvocation || !v12.timelineSemaphore || !v12.runtimeDescriptorArray || !v12.descriptorBindingPartiallyBound || !v12.descriptorBindingSampledImageUpdateAfterBind || !v12.shaderSampledImageArrayNonUniformIndexing ){
            std::cout << "requires task/mesh shaders, dynamic rendering, sync2, timeline and bindless sampled-image indexing" << std::endl;
            continue;
        }
        // MORE EVALUATIONS FOR REQUIRED FEATURES
        ;
        //
        const int score = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 100 : 10;
        if ( score > best) {
            best = score;
            physical_ = candidate;
            //families = *queues;
            //maxTaskGroupsTotal = meshLimits.maxTaskWorkGroupTotalCount;
            //maxMeshGroups = std::min( {meshLimits.maxMeshWorkGroupCount[0], meshLimits.maxTaskWorkGroupCount[0], meshLimits.maxTaskWorkGroupTotalCount} );
            //maxMeshInstances = std::min( meshLimits.maxMeshWorkGroupCount[1], meshLimits.maxTaskWorkGroupCount[1] );

            //
            std::cout << "Enumerated candidate device " << properties.deviceName << std::endl;
            //
            for ( const auto extension : extensions ){
                std::cout << " >> " << extension.extensionName << " << ";
            }
        }
    }
    // If we have a physical candidate, we now create the vulkan device
    ;
}

void Renderer::CreateCommandBuffers(){

}

void Renderer::CreateGeometry(){

}
