#pragma once
#include <QPainterPath>
#include <qnamespace.h>

class Path {
  public:

  Path(Qt::GlobalColor color);
  ~Path() = default;
  void drawPath(QPainter*painter);
  void lineTo(float x, float y, Qt::GlobalColor color = Qt::green);

  void moveTo(float x, float y);
private:
  QPainterPath* main;
  QPainterPath* x;
  QPainterPath* z;

  Qt::GlobalColor color = Qt::green;


};
