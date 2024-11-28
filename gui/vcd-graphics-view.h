#pragma once
#include "../lib/vcd-parser/vcd-data.h"
#include <QGraphicsView>
#include <QWidget>
#include <qtreewidget.h>
#include <qwidget.h>
#include <QGraphicsScene>

class Waves;
class VCDPlotter;
class VCDGraphicsView : public QGraphicsView {

public:
  VCDGraphicsView(VCDPlotter*top, QList<VarData>& vars, QList<QTreeWidgetItem *>& list, VCDData *data,
                  QWidget *parent = nullptr);
  ~VCDGraphicsView();
  QList<QTreeWidgetItem *> &list;
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  

  int scrollHeight;
  QRect itemRect;
  QGraphicsScene *scene;

  VCDData *data;
  Waves * waves;
  QList<VarData>& vars;
  VCDPlotter*top;
};
