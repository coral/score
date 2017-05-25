#include <Scenario/Document/Minimap/Minimap.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QWidget>
#include <ossia/detail/math.hpp>
#include <QApplication>
#include <QDesktopWidget>
namespace Scenario
{
static const constexpr auto min_dist = 10;
Minimap::Minimap(QWidget* vp):
  m_viewport{vp}
{
}

void Minimap::setWidth(double d)
{
  prepareGeometryChange();
  m_width = d;
  update();
  m_viewport->update();
}

void Minimap::setLeftHandle(double l)
{
  m_leftHandle = ossia::clamp(l, 0., m_rightHandle - min_dist);
  update();
  m_viewport->update();
}

void Minimap::setRightHandle(double r)
{
  m_rightHandle = ossia::clamp(r, m_leftHandle + min_dist, m_width);
  update();
  m_viewport->update();
}

void Minimap::setHandles(double l, double r)
{
  m_leftHandle = ossia::clamp(l, 0., m_rightHandle - min_dist);
  m_rightHandle = ossia::clamp(r, m_leftHandle + min_dist, m_width);

  update();
  m_viewport->update();
}

void Minimap::modifyHandles(double l, double r)
{
  setHandles(l, r);
  emit visibleRectChanged(m_leftHandle, m_rightHandle);
}

void Minimap::setLargeView()
{
  modifyHandles(0., m_width);
}

void Minimap::zoomIn()
{
  modifyHandles(m_leftHandle + 10, m_rightHandle - 10);
}

void Minimap::zoomOut()
{
  modifyHandles(m_leftHandle - 10, m_rightHandle + 10);
}

QRectF Minimap::boundingRect() const
{
  return {0., 0., m_width, m_height};
}

void Minimap::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  painter->setPen(QPen(QColor(0, 255, 0, 255), 2));
  painter->drawRect(QRectF{m_leftHandle, 2., m_rightHandle - m_leftHandle, m_height - 3.});
}

void Minimap::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
  m_gripLeft = false;
  m_gripRight = false;
  m_gripMid = false;

  const auto pos_x = ev->pos().x();

  if(std::abs(pos_x - m_leftHandle) < 3.)
    m_gripLeft = true;
  else if(std::abs(pos_x - m_rightHandle) < 3.)
    m_gripRight = true;
  else if(pos_x > m_leftHandle && pos_x < m_rightHandle)
    m_gripMid = true;

  m_startPos = ev->screenPos();
  m_lastPos = m_startPos;

  QCursor::setPos(100, 100);
  QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
  ev->accept();
}

void Minimap::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
{
  const auto pos = ev->screenPos();
  if(m_gripLeft || m_gripRight || m_gripMid)
  {
    auto dx = pos.x() - 100;
    if(m_gripLeft)
    {
      setLeftHandle(m_leftHandle + dx);
    }
    else if(m_gripRight)
    {
      setRightHandle(m_rightHandle + dx);
    }
    else if(m_gripMid)
    {
      auto dy = pos.y() - 100;

      setHandles(
            m_leftHandle  + dx - dy,
            m_rightHandle + dx + dy);
    }

    QCursor::setPos(100, 100);
    emit visibleRectChanged(m_leftHandle, m_rightHandle);
  }
  ev->accept();
}

void Minimap::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
  m_gripLeft = false;
  m_gripRight = false;
  m_gripMid = false;
  QCursor::setPos(m_startPos);
  QApplication::restoreOverrideCursor();
  ev->accept();
}

}

