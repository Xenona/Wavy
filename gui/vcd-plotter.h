#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <qtmetamacros.h>
#include "../lib/vcd-parser/vcd-data.h"
#include <QTreeWidget>

class VCDPlotter: public QWidget {

public:
  VCDPlotter(VCDData* data);
  void wheelEvent(QWheelEvent* event) override;
  QTreeWidget *selected_dumps;

private:

  VCDData* data;
  QGraphicsScene* scene;
  QGraphicsView* view;
  QScrollBar* horizScroll;
  QScrollBar* vertScroll;


  int marker;
  int leftFOVborder; 
  int rightFOVborder;


};
