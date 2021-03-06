#include "coloritem.h"
#include <QtGui>

ColorItem::ColorItem(string mask, QColor clr, int id)
    :  Tile(mask), _active(false), _color(clr), _id(id)
{
    setCursor(Qt::OpenHandCursor);
}

QRectF ColorItem::boundingRect() const
{
    return QRectF(0, 0, width() * 20, height() * 20);
}

void ColorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(QBrush(_color));
    int h, s, v;
    _color.getHsv(&h, &s, &v);
    QColor clr = _color;
    if (!_active)
    {
        clr.setHsv(h, s/2, v);
    }

    for (int i = 0; i < height(); ++i)
    {
        for (int j = 0; j < width(); ++j)
        {
            if (Tile::data[i][j] != '0')
            {
                painter->fillRect(j * 20 + 1, i * 20 + 1, 18, 18, clr);
            }
        }
    }
}

void ColorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!_active)
    {
        return;
    }

    switch(event->button())
    {
    case(Qt::LeftButton):
        setCursor(Qt::ClosedHandCursor);
        break;
    case(Qt::RightButton):
        prepareGeometryChange();
        Tile::rotate();
        update();
        break;
    default:
        event->ignore();
    }
}

void ColorItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (!_active)
    {
        return;
    }

    reflect();
    update();
}

void ColorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!_active)
    {
        return;
    }

    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() < QApplication::startDragDistance())
    {
        return;
    }

    QDrag *drag = new QDrag(event->widget());
    QMimeData *mime = new QMimeData;
    drag->setMimeData(mime);

    mime->setColorData(_color);
    mime->setText(QString::number(_id) + QString(':') + QString(getAsText().c_str()));

    QPixmap pixmap(width() * 20, height() * 20);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    paint(&painter, 0, 0);
    painter.end();

    pixmap.setMask(pixmap.createHeuristicMask());

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(5, 5));

    drag->exec();
    setCursor(Qt::OpenHandCursor);
}

void ColorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    setCursor(Qt::OpenHandCursor);
}

void ColorItem::activate()
{
    _active = true;
}

void ColorItem::deactivate()
{
    _active = false;
}
