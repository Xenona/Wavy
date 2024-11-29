#pragma once
#include "../lib/vcd-parser/vcd-data.h"
#include "vcd-graphics-view.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTreeWidget>
#include <qmap.h>
#include <qtmetamacros.h>
#include <qtreewidget.h>

class VCDPlotter : public QWidget {

public:
  VCDPlotter(VCDData *data, QWidget *parent);
  void wheelEvent(QWheelEvent *event) override;
  QTreeWidget *selected_dumps;
  QList<QTreeWidgetItem *> dumpsList;
  QList<VarData> varsList;

  void setHeightView(int height);
  void addSelectedDumps(QList<QTreeWidgetItem *> items, QList<VarData> varData);
  void zoomView(int delta);
  void sideShiftView(int delta);
  void plotUpdate();
  void drawWave();

  int marker = 0;
  long long leftFOVborder = 0;
  long long rightFOVborder = 0;
  long long minleftborder = 0;
  long long maxrightborder = 0;

  long long currentHeight = 0;

private:
  int MAX_ZOOM_DELTA = 30;

  VCDData *data;
  VCDGraphicsView *view;
  QScrollBar *horizScroll;
  QScrollBar *vertScroll;
};
