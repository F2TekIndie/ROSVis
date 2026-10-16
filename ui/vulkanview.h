#ifndef VULKANVIEW_H
#define VULKANVIEW_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>



// FORWADS
class Renderer;


//
class VulkanView : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit VulkanView();
    ~VulkanView() = default;

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow*win);

private:
    //
    Renderer*   renderer_{nullptr};
    //
    void releaseResources() override;
};

#endif // VULKANVIEW_H
