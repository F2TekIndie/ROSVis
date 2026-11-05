
#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>
#include <QQuickWindow>
#include "scene.h"



//
class Renderer : public QObject
{
    Q_OBJECT

public:
    Renderer() = default;
    ~Renderer() = default;

    //
    void setViewportSize(const QPointF &pos, const QSizeF &size) { pos_ = pos; viewportSize_ = size; }
    void setWindow(QQuickWindow *window) { window_ = window; }

public slots:
    void frameStart();
    void mainPassRecordingStart();

private:
    struct RendererImpl;
    // QT
    QQuickWindow*   window_{nullptr};
    QSizeF          viewportSize_{};
    QPointF         pos_{};
    bool            initialized_{false};
    // RENDERER
    std::shared_ptr<RendererImpl> impl_{nullptr};
    Scene       scene_{};
};

#endif // RENDERER_H
