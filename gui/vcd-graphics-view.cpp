#include "vcd-graphics-view.h"
#include "waves.h"
#include <QDebug>
#include <QPainterPath>
#include <qdebug.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qtreewidget.h>


VCDGraphicsView::VCDGraphicsView(VCDPlotter*top, QList<VarData> &vars,
                                 QList<QTreeWidgetItem *> &list, VCDData *data, QList<WaveState> &waveStates,
                                 QWidget *parent)
    : QGraphicsView(parent), data(data), list(list), vars(vars),
      scene(new QGraphicsScene(this)), waves(new Waves(this)), top(top), waveStates(waveStates) {
  this->setResizeAnchor(NoAnchor);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  this->scene->addItem(this->waves);
  this->setScene(this->scene);

}

VCDGraphicsView::~VCDGraphicsView() {
  delete this->scene;
};

void VCDGraphicsView::resizeEvent(QResizeEvent *event) {

  QSize a = this->size();
  this->setSceneRect(0, 0, a.width(), a.height());

  // this->repaint();
  this->top->plotUpdate();
  QGraphicsView::resizeEvent(event);
}

void VCDGraphicsView::paintEvent(QPaintEvent *event) {
  QPainter painter(this->viewport());
  painter.drawRect(0, 0, this->viewport()->size().width(),
                   this->viewport()->size().height());
   
  QGraphicsView::paintEvent(event);
};
