#include "path.h"
#include <QPainter>
#include <qnamespace.h>


Path::Path(Qt::GlobalColor color) : color(color) {
  this->main = new QPainterPath();
  this->x = new QPainterPath();
  this->z = new QPainterPath();
}

void Path::lineTo(float x, float y, Qt::GlobalColor color) {
  if (color == Qt::red) {
    this->x->lineTo(x, y);
    this->z->moveTo(x, y);
    this->main->moveTo(x, y);
  } else if (color == this->color) {
    this->x->moveTo(x, y);
    this->z->moveTo(x, y);
    this->main->lineTo(x, y);
  } else if (color == Qt::blue) {
    this->x->moveTo(x, y);
    this->z->lineTo(x, y);
    this->main->moveTo(x, y);
  }
}

void Path::drawPath(QPainter *painter) {
  // painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
  painter->save();
  painter->setPen(QPen(this->color));
  painter->drawPath(*this->main);
  painter->setPen(QPen(Qt::red));
  painter->drawPath(*this->x);
  painter->setPen(QPen(Qt::blue));
  painter->drawPath(*this->z);
  painter->restore();
}

void Path::moveTo(float x, float y) {
  this->x->moveTo(x, y);
  this->z->moveTo(x, y);
  this->main->moveTo(x, y);
};
