
#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <QObject>
#include <QQuickWindow>



//
class Renderer : public QObject
{
    Q_OBJECT

public:
    Renderer();
    ~Renderer();

    //
    void setViewportSize(const QSize &size) { viewportSize_ = size; }
    void setWindow(QQuickWindow *window) { window_ = window; }

public slots:
    void frameStart();
    void mainPassRecordingStart();

private:
    VkInstance          instance_{};
    VkSurfaceKHR        surface_{};
    //
    VkPhysicalDevice    physical_{};
    VkDevice            device_{};
    //
    QQuickWindow*   window_{nullptr};
    QSize           viewportSize_{};
    //
    void Init();
    void CreateDevice();
    void CreateCommandBuffers();
    void CreateGeometry();
};

#endif // RENDERER_H
