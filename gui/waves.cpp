#include "waves.h"
#include <QPainter>
#include <QPainterPath>
#include <qgraphicsitem.h>
#include <qlogging.h>


void Waves::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                 QWidget *widget) {
  QPainterPath path;

  qDebug() << "test" << test;
  path.moveTo(0,  test  +0);
  path.lineTo(20, test  + 0);
  path.lineTo(40, test  + 0);
  path.lineTo(40, test  + 20);
  path.lineTo(60, test  + 20);
  path.lineTo(60, test  + 0);
  path.lineTo(80, test  + 0);
  path.lineTo(80, test  + 20);
  path.lineTo(100, test +20);
  path.lineTo(100, test +0);
  path.lineTo(120, test +0);
  path.lineTo(120, test +0);

  painter->drawPath(path);
  // path.closeSubpath();
};

QRectF Waves::boundingRect() const {
  return QRectF(0, 0, 10, 10);
};

