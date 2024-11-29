#pragma once
#include "vcd-graphics-view.h"
#include "vcd-plotter.h"
#include <QGraphicsObject>
#include <QWidget>

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
  void addText(float scenePos, float prevScenePos, QPainter *painter,
               float lineHeight, int WAVES_GAP, long long prev,
               long long prevFloat, DumpData dump, int idx, float y,
               std::string prevString);
};
