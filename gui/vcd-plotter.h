#pragma once
#include "../lib/vcd-parser/vcd-data.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTreeWidget>
#include <qmap.h>
#include <qnamespace.h>
#include <qtmetamacros.h>
#include <qtreewidget.h>

class VCDGraphicsView;
class WavyMainWindow;

enum class base {
  NIL,
  bin,
  oct,
  dec,
  sig,
  hex,
};
struct WaveState {
  Qt::GlobalColor color;
  base base;
};
class VCDPlotter : public QWidget {

public:
  VCDPlotter(QString path, VCDData *data, WavyMainWindow *top, QWidget *parent);
  ~VCDPlotter();
  void wheelEvent(QWheelEvent *event) override;

  QTreeWidget *selected_dumps;
  QList<QTreeWidgetItem *> dumpsList;
  QList<VarData> varsList;
  QList<WaveState> waveStates;
  WavyMainWindow *top;

  void setCustomisations(Qt::GlobalColor color, base base);
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

  QString path;
  VCDGraphicsView *view;

private:
  int MAX_ZOOM_DELTA = 20;

  VCDData *data;
  QScrollBar *horizScroll;
  QScrollBar *vertScroll;
};
