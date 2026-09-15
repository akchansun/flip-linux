#include "ImageView.h"

#include "FitZoom.h"
#include "I18n.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QUrl>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>

namespace {
constexpr double kMinScale = 0.05;
constexpr double kMaxScale = 16.0;
constexpr double kZoomStep = 1.25;
} // namespace

ImageView::ImageView(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
}

void ImageView::setImage(const QImage& image)
{
    m_image = image;
    m_pixmap = QPixmap::fromImage(m_image);
    m_pan = QPointF();
    zoomFitDefault();
    update();
}

void ImageView::clearImage()
{
    m_image = QImage();
    m_pixmap = QPixmap();
    m_scale = 1.0;
    m_fitMode = true;
    m_pan = QPointF();
    update();
    emit scaleChanged(m_scale);
}

bool ImageView::hasImage() const
{
    return !m_image.isNull();
}

void ImageView::zoomIn()
{
    applyScale(m_scale * kZoomStep, false);
}

void ImageView::zoomOut()
{
    applyScale(m_scale / kZoomStep, false);
}

void ImageView::zoomActual()
{
    applyScale(1.0, false);
}

void ImageView::zoomFitDefault()
{
    applyScale(fitScale(), true);
}

double ImageView::scale() const
{
    return m_scale;
}

QSize ImageView::imagePixelSize() const
{
    return m_image.size();
}

double ImageView::fitScale() const
{
    return defaultFitScale(m_image.size(), size());
}

QRectF ImageView::drawnRect() const
{
    if (m_image.isNull())
        return {};
    const QSizeF drawn(m_image.width() * m_scale, m_image.height() * m_scale);
    QPointF topLeft((width() - drawn.width()) / 2.0, (height() - drawn.height()) / 2.0);
    topLeft += m_pan;
    return QRectF(topLeft, drawn);
}

void ImageView::applyScale(double scale, bool fitMode)
{
    m_fitMode = fitMode;
    m_scale = std::clamp(scale, kMinScale, kMaxScale);
    if (m_fitMode)
        m_pan = QPointF();
    update();
    emit scaleChanged(m_scale);
}

int ImageView::edgeZoneWidth() const
{
    return std::max(48, int(width() * 0.16));
}

ImageView::HoverEdge ImageView::edgeAt(const QPoint& pos) const
{
    if (!hasImage() || m_image.isNull())
        return HoverEdge::None;
    const int zone = edgeZoneWidth();
    if (pos.x() <= zone)
        return HoverEdge::Left;
    if (pos.x() >= width() - zone)
        return HoverEdge::Right;
    return HoverEdge::None;
}

void ImageView::updateHover(const QPoint& pos)
{
    const HoverEdge edge = edgeAt(pos);
    if (edge != m_hover) {
        m_hover = edge;
        if (m_hover == HoverEdge::None)
            unsetCursor();
        else
            setCursor(Qt::PointingHandCursor);
        update();
    }
}

QStringList ImageView::droppedPaths(const QMimeData* mime) const
{
    QStringList paths;
    if (!mime || !mime->hasUrls())
        return paths;
    for (const QUrl& url : mime->urls()) {
        if (url.isLocalFile())
            paths << url.toLocalFile();
    }
    return paths;
}

void ImageView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(24, 24, 24));

    if (m_image.isNull()) {
        p.setPen(QColor(210, 210, 210));
        QFont title = font();
        title.setPointSize(title.pointSize() + 4);
        title.setBold(true);
        p.setFont(title);
        const QRect hintRect = rect().adjusted(24, 0, -24, -12);
        p.drawText(hintRect, Qt::AlignCenter, I18n::t("empty.hint"));
        QFont sub = font();
        p.setFont(sub);
        p.setPen(QColor(150, 150, 150));
        p.drawText(rect().adjusted(24, 28, -24, 0), Qt::AlignHCenter | Qt::AlignVCenter,
                   I18n::t("empty.sub"));
        return;
    }

    p.setRenderHint(QPainter::SmoothPixmapTransform, m_scale < 0.999 || m_scale > 1.001);
    p.drawPixmap(drawnRect().toRect(), m_pixmap);

    if (m_hover != HoverEdge::None) {
        const int zone = edgeZoneWidth();
        QRect band = m_hover == HoverEdge::Left ? QRect(0, 0, zone, height())
                                                : QRect(width() - zone, 0, zone, height());
        p.fillRect(band, QColor(255, 255, 255, 22));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 200));
        const qreal cx = m_hover == HoverEdge::Left ? zone * 0.45 : width() - zone * 0.45;
        const qreal cy = height() / 2.0;
        QPainterPath chevron;
        if (m_hover == HoverEdge::Left) {
            chevron.moveTo(cx + 8, cy - 18);
            chevron.lineTo(cx - 10, cy);
            chevron.lineTo(cx + 8, cy + 18);
            chevron.lineTo(cx + 14, cy + 10);
            chevron.lineTo(cx + 2, cy);
            chevron.lineTo(cx + 14, cy - 10);
        } else {
            chevron.moveTo(cx - 8, cy - 18);
            chevron.lineTo(cx + 10, cy);
            chevron.lineTo(cx - 8, cy + 18);
            chevron.lineTo(cx - 14, cy + 10);
            chevron.lineTo(cx - 2, cy);
            chevron.lineTo(cx - 14, cy - 10);
        }
        chevron.closeSubpath();
        p.drawPath(chevron);
    }
}

void ImageView::resizeEvent(QResizeEvent*)
{
    if (m_fitMode && hasImage())
        applyScale(fitScale(), true);
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;
    m_pressPos = event->pos();
    m_pressPan = m_pan;
    m_moved = false;
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
    updateHover(event->pos());
    if (!(event->buttons() & Qt::LeftButton))
        return;
    const QPoint delta = event->pos() - m_pressPos;
    if (delta.manhattanLength() < 4)
        return;
    m_moved = true;
    if (!m_fitMode && hasImage()) {
        m_pan = m_pressPan + delta;
        setCursor(Qt::ClosedHandCursor);
        update();
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;
    updateHover(event->pos());
    if (m_moved)
        return;
    const HoverEdge edge = edgeAt(event->pos());
    if (edge == HoverEdge::Left)
        emit previousRequested();
    else if (edge == HoverEdge::Right)
        emit nextRequested();
}

void ImageView::mouseDoubleClickEvent(QMouseEvent*)
{
    if (!hasImage())
        return;
    if (m_fitMode && qAbs(m_scale - 1.0) > 0.001)
        zoomActual();
    else
        zoomFitDefault();
}

void ImageView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (!hasImage())
            return;
        if (event->angleDelta().y() > 0)
            zoomIn();
        else if (event->angleDelta().y() < 0)
            zoomOut();
        return;
    }
    if (event->angleDelta().y() > 0)
        emit previousRequested();
    else if (event->angleDelta().y() < 0)
        emit nextRequested();
}

void ImageView::dragEnterEvent(QDragEnterEvent* event)
{
    if (!droppedPaths(event->mimeData()).isEmpty())
        event->acceptProposedAction();
}

void ImageView::dropEvent(QDropEvent* event)
{
    const QStringList paths = droppedPaths(event->mimeData());
    if (!paths.isEmpty()) {
        event->acceptProposedAction();
        emit filesDropped(paths);
    }
}

void ImageView::leaveEvent(QEvent*)
{
    if (m_hover != HoverEdge::None) {
        m_hover = HoverEdge::None;
        unsetCursor();
        update();
    }
}
