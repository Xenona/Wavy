#include "vcd-plotter.h"
#include <QGraphicsScene>
#include <qboxlayout.h>
#include <qgraphicsview.h>
#include <QScrollBar>
#include <qwidget.h>
#include <QSplitter>
#include <QTreeWidget>
#include <QHeaderView>

VCDPlotter::VCDPlotter(VCDData* data) {

  this->data = data;
  this->scene = new QGraphicsScene(this);

  QVBoxLayout* verticalLayout_12 = new QVBoxLayout(this);

  QSplitter * splitter = new QSplitter(this);
  splitter->setOrientation(Qt::Horizontal);
  splitter->setChildrenCollapsible(false);
  QWidget* verticalLayoutWidget_4 = new QWidget(splitter);
  QHBoxLayout* horizontalLayout = new QHBoxLayout(verticalLayoutWidget_4);
  horizontalLayout->setSpacing(0);
  horizontalLayout->setContentsMargins(0, 0, 0, 0);

  this->selected_dumps = new QTreeWidget(verticalLayoutWidget_4);
  this->selected_dumps->header()->setVisible(false);
  this->selected_dumps->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  // this->selected_dumps_list->setMaximumWidth(600);
  // this->selected_dumps_list->setMinimumWidth(50);
  this->selected_dumps->setGeometry({0, 0, 1000, 1000});
  this->selected_dumps->show();

  horizontalLayout->addWidget(selected_dumps);

  this->vertScroll = new QScrollBar(verticalLayoutWidget_4);
  this->vertScroll->setOrientation(Qt::Vertical);

  horizontalLayout->addWidget(this->vertScroll);

  splitter->addWidget(verticalLayoutWidget_4);
  QWidget * verticalLayoutWidget = new QWidget(splitter);
  QVBoxLayout * verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget);
  verticalLayout_2->setSpacing(0);
  verticalLayout_2->setContentsMargins(0, 0, 0, 0);
  this->view = new QGraphicsView(verticalLayoutWidget);

  verticalLayout_2->addWidget(this->view);

  this->horizScroll = new QScrollBar(verticalLayoutWidget);
  this->horizScroll->setOrientation(Qt::Horizontal);

  verticalLayout_2->addWidget(this->horizScroll);

  splitter->addWidget(verticalLayoutWidget);

  verticalLayout_12->addWidget(splitter);


  // this->leftFOVborder = this->data->timepoints[0];

}

void VCDPlotter::wheelEvent(QWheelEvent* event) {
  // if shift, scroll left/right, if ctrl, zoom in out 
  // where mouse points, if just like that, zoom in out 
  // where marker is  
}
