
#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <QObject>
#include <QQuickWindow>
#include "shaders.h"



// FORWARDS
class QVulkanDeviceFunctions;
class QVulkanFunctions;



//
class Renderer : public QObject
{
    Q_OBJECT

public:
    Renderer();
    ~Renderer();

    //
    void setViewportSize(const QPointF &pos, const QSizeF &size) { pos_ = pos; viewportSize_ = size; }
    void setWindow(QQuickWindow *window) { window_ = window; }

public slots:
    void frameStart();
    void mainPassRecordingStart();

private:
    VkInstance          instance_{};
    VkSurfaceKHR        surface_{};
    VkPhysicalDevice    physical_{};
    VkDevice            device_{};
    //
    QVulkanDeviceFunctions* deviceFuncs_{nullptr};
    QVulkanFunctions*       instanceFuncs_{nullptr};
    //
    VkBuffer            vbuf_{VK_NULL_HANDLE};
    VkDeviceMemory      vbufMem_{VK_NULL_HANDLE};
    //
    VkPipeline          pipeline_{VK_NULL_HANDLE};
    VkPipelineLayout    pipelineLayout_{VK_NULL_HANDLE};
    VkPipelineCache     pipelineCache_{VK_NULL_HANDLE};
    //
    QQuickWindow*   window_{nullptr};
    QSizeF          viewportSize_{};
    QPointF         pos_{};
    bool            initialized_{false};
    //
    Shaders     vulkanShaders_{};
    //
    void Init();
    void CreateVulkanBuffers();
    void CreateVulkanPipeline();
};

#endif // RENDERER_H
