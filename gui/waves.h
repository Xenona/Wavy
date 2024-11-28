#pragma once
#include "vcd-graphics-view.h"
#include <QGraphicsObject>
#include <QWidget>
#include "vcd-plotter.h"


class Waves : public QGraphicsObject {

public:
  Waves(VCDGraphicsView *top);

  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget = nullptr) override;

  int test;
  VCDGraphicsView *top;
  int isScalar(DumpData data, std::string id);
  int isVector(DumpData data, std::string id);

};
