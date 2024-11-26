#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <qmap.h>
#include <qtmetamacros.h>
#include "../lib/vcd-parser/vcd-data.h"
#include "vcd-graphics-view.h"
#include <QTreeWidget>
#include <qtreewidget.h>
#include "waves.h"

class VCDPlotter: public QWidget {

public:
  VCDPlotter(VCDData* data, QWidget* parent);
  void wheelEvent(QWheelEvent* event) override;
  QTreeWidget *selected_dumps;
  QList<QTreeWidgetItem*> dumpsList;
  QList<VarData> varsList;

  void setHeightView(int height);
  void addSelectedDumps(QList<QTreeWidgetItem*> items, QList<VarData> varData);
  void zoomView(int delta);
  void sideShiftView(int delta);
  void plotUpdate();
  void drawWave();


private:

  int MAX_ZOOM_DELTA = 10;

  VCDData* data;
  VCDGraphicsView* view;
  QScrollBar* horizScroll;
  QScrollBar* vertScroll;


  int marker;
  int leftFOVborder; 
  int rightFOVborder;

  int currentHeight;

};
