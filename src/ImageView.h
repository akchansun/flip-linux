#pragma once

#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QPointF>
#include <QWidget>

class QMimeData;

class ImageView : public QWidget
{
    Q_OBJECT

public:
    explicit ImageView(QWidget* parent = nullptr);

    void setImage(const QImage& image);
    void clearImage();
    bool hasImage() const;

    void zoomIn();
    void zoomOut();
    void zoomActual();
    void zoomFitDefault();

    double scale() const;
    QSize imagePixelSize() const;

signals:
    void previousRequested();
    void nextRequested();
    void scaleChanged(double scale);
    void filesDropped(const QStringList& paths);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    enum class HoverEdge { None, Left, Right };

    double fitScale() const;
    QRectF drawnRect() const;
    void applyScale(double scale, bool fitMode);
    int edgeZoneWidth() const;
    HoverEdge edgeAt(const QPoint& pos) const;
    void updateHover(const QPoint& pos);
    QStringList droppedPaths(const QMimeData* mime) const;

    QImage m_image;
    QPixmap m_pixmap;
    bool m_fitMode = true;
    double m_scale = 1.0;
    QPointF m_pan;
    QPoint m_pressPos;
    QPointF m_pressPan;
    bool m_moved = false;
    HoverEdge m_hover = HoverEdge::None;
};
