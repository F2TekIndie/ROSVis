
#include "vulkanview.h"
#include <QPlatformSurfaceEvent>
#include "renderer/renderer.h"




VulkanView::VulkanView()
    :QQuickItem()
{
    connect( this, &QQuickItem::windowChanged, this, &VulkanView::handleWindowChanged );
}

void VulkanView::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &VulkanView::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &VulkanView::cleanup, Qt::DirectConnection);

        // Ensure we start with cleared to black. The squircle's blend mode relies on this.
        win->setColor(Qt::black);
    }
}

void VulkanView::cleanup(){
    renderer_ = nullptr;
}

void VulkanView::sync(){
    if ( !renderer_ ){
        renderer_ = new Renderer();
        // Initializing resources is done before starting to record the
        // renderpass, regardless of wanting an underlay or overlay.
        connect(window(), &QQuickWindow::beforeRendering, renderer_, &Renderer::frameStart, Qt::DirectConnection);
        // Here we want an underlay and therefore connect to
        // beforeRenderPassRecording. Changing to afterRenderPassRecording
        // would render the squircle on top (overlay).
        connect(window(), &QQuickWindow::beforeRenderPassRecording, renderer_, &Renderer::mainPassRecordingStart, Qt::DirectConnection);
    }
    renderer_->setViewportSize(window()->size() * window()->devicePixelRatio());
    renderer_->setWindow(window());
}

void VulkanView::releaseResources(){
    delete renderer_;
    renderer_ = nullptr;
}
