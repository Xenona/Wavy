#include "vcd-plotter.h"
#include "ui_capture_device_dialog.h"
#include "ui_wavy-main-window.h"
#include "vcd-graphics-view.h"
#include "waves.h"
#include "wavy-main-window.h"
#include <QGraphicsScene>
#include <QHeaderView>
#include <QPainterPath>
#include <QScrollBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QTreeWidget>
#include <QWheelEvent>
#include <cmath>
#include <qabstractitemview.h>
#include <qabstractspinbox.h>
#include <qboxlayout.h>
#include <qdebug.h>
#include <qgraphicsview.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qscrollbar.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qtabwidget.h>
#include <qtreewidget.h>
#include <qwidget.h>
#include <sched.h>


VCDPlotter::VCDPlotter(QString path, VCDData *data, WavyMainWindow *top,
                       QWidget *parent)
    : QWidget(parent), path(path), top(top)

{
  this->data = data;
  QVBoxLayout *verticalLayout_12 = new QVBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);
  splitter->setOrientation(Qt::Horizontal);
  splitter->setChildrenCollapsible(false);
  QWidget *verticalLayoutWidget_4 = new QWidget(splitter);
  QHBoxLayout *horizontalLayout = new QHBoxLayout(verticalLayoutWidget_4);
  horizontalLayout->setSpacing(0);
  horizontalLayout->setContentsMargins(0, 0, 0, 0);
  this->selected_dumps = new QTreeWidget(verticalLayoutWidget_4);
  this->selected_dumps->header()->setVisible(false);
  this->selected_dumps->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  // this->selected_dumps_list->setMaximumWidth(600);
  // this->selected_dumps_list->setMinimumWidth(50);
  this->selected_dumps->setGeometry({0, 0, 1000, 1000});
  // this->selected_dumps->setSizePolicy(QSizePolicy::Policy::Expanding);
  this->selected_dumps->show();
  this->selected_dumps->setSelectionMode(QAbstractItemView::MultiSelection);
  horizontalLayout->addWidget(selected_dumps);
  this->vertScroll = this->selected_dumps->verticalScrollBar();
  splitter->addWidget(verticalLayoutWidget_4);
  QWidget *verticalLayoutWidget = new QWidget(splitter);
  QVBoxLayout *verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget);
  verticalLayout_2->setSpacing(0);
  verticalLayout_2->setContentsMargins(0, 0, 0, 0);

  this->view =
      new VCDGraphicsView(this, this->varsList, this->dumpsList, this->data,
                          this->waveStates, verticalLayoutWidget);

  verticalLayout_2->addWidget(this->view);
  this->horizScroll = new QScrollBar(verticalLayoutWidget);
  this->horizScroll->setOrientation(Qt::Horizontal);
  verticalLayout_2->addWidget(this->horizScroll);
  splitter->addWidget(verticalLayoutWidget);
  verticalLayout_12->addWidget(splitter);
  QObject::connect(this->vertScroll, &QScrollBar::actionTriggered, this,
                   [this](int value) {
                     // todo fix the magic number
                     if (value == 7) {
                       this->setHeightView(this->vertScroll->sliderPosition());
                     }
                   });

  QObject::connect(
      this->horizScroll, &QScrollBar::actionTriggered, this, [this](int value) {
        if (value == 7) {
          unsigned long long diff = this->rightFOVborder - this->leftFOVborder;
          this->leftFOVborder = this->horizScroll->sliderPosition();
          this->rightFOVborder =
              this->leftFOVborder + diff <= this->maxrightborder
                  ? this->leftFOVborder + diff
                  : this->maxrightborder;
        }
      });

  QObject::connect(this->top->ui->actionRed, &QAction::triggered, this,
                   [this]() { this->setCustomisations(Qt::red, base::NIL); });
  QObject::connect(
      this->top->ui->actionYellow, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::yellow, base::NIL); });
  QObject::connect(
      this->top->ui->actionOrang, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::darkYellow, base::NIL); });
  QObject::connect(this->top->ui->actionGreen, &QAction::triggered, this,
                   [this]() { this->setCustomisations(Qt::green, base::NIL); });
  QObject::connect(this->top->ui->actionLight_blue, &QAction::triggered, this,
                   [this]() { this->setCustomisations(Qt::cyan, base::NIL); });
  QObject::connect(this->top->ui->actionBlue, &QAction::triggered, this,
                   [this]() { this->setCustomisations(Qt::blue, base::NIL); });
  QObject::connect(
      this->top->ui->actionPurple, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::darkMagenta, base::NIL); });
  QObject::connect(
      this->top->ui->actionMagenta, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::magenta, base::NIL); });
  QObject::connect(this->top->ui->actionWhite, &QAction::triggered, this,
                   [this]() { this->setCustomisations(Qt::white, base::NIL); });
  QObject::connect(
      this->top->ui->actionBinary, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::transparent, base::bin); });
  QObject::connect(
      this->top->ui->actionOctal, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::transparent, base::oct); });
  QObject::connect(
      this->top->ui->actionDecimal, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::transparent, base::dec); });
  QObject::connect(
      this->top->ui->actionHexadecimal, &QAction::triggered, this,
      [this]() { this->setCustomisations(Qt::transparent, base::hex); });

  QObject::connect(this->selected_dumps, &QTreeWidget::itemSelectionChanged,
                   this, [this]() {
                     QList<QTreeWidgetItem *> selectedElements =
                         this->selected_dumps->selectedItems();
                     this->top->ui->menuSelected_dump->setDisabled(
                         selectedElements.size() == 0);
                   });

  if (!this->data->timepoints.size()) {
    this->data->warns.push_back("The timing diagram has no time points!");
  } else {
    this->leftFOVborder = this->data->timepoints[0].time;
    this->rightFOVborder = this->data->timepoints.back().time;
    this->minleftborder = this->leftFOVborder;
    this->maxrightborder = this->rightFOVborder;
    this->marker = this->leftFOVborder;
    this->horizScroll->setMinimum(this->leftFOVborder);
    this->horizScroll->setMaximum(this->rightFOVborder -
                                  (this->maxrightborder - this->minleftborder));
  }

  QSize a = this->view->size();
  this->view->setSceneRect(0, 0, a.width(), a.height());
  this->view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  qDebug() << this->view->sceneRect();
  this->view->centerOn(0, 0);
  this->plotUpdate();
}

void VCDPlotter::setCustomisations(Qt::GlobalColor color, base base) {
  auto selected = this->selected_dumps->selectedItems();
  auto all = this->dumpsList;
  for (int i = 0; i < all.size(); i++) {
    for (int j = i; j < selected.size(); j++) {

      if (all[i] == selected[j]) {
        if (base != base::NIL) {

          this->waveStates[i].base = base;
        }
        if (color != Qt::transparent) {

          this->waveStates[i].color = color;
        }
      }
    }
  }
};

VCDPlotter::~VCDPlotter() {
  for (int i = 0; i < this->dumpsList.size(); i++) {
    delete this->dumpsList[i];
  }
}

void VCDPlotter::setHeightView(int height) {
  this->currentHeight = height;
  this->plotUpdate();
};

void VCDPlotter::sideShiftView(int delta) {
  if (delta > 0) {
    if (this->leftFOVborder - delta >= 0) {
      this->leftFOVborder -= delta;
    } else {
      this->leftFOVborder = 0;
    }

    if (this->rightFOVborder - delta >= this->MAX_ZOOM_DELTA) {
      this->rightFOVborder -= delta;
    } else {
      this->rightFOVborder = this->MAX_ZOOM_DELTA;
    }
  } else {
    // todo
    if (this->leftFOVborder - delta <
        this->data->timepoints.back().time - this->MAX_ZOOM_DELTA) {
      this->leftFOVborder -= delta;
    } else {
      this->leftFOVborder =
          this->data->timepoints.back().time - this->MAX_ZOOM_DELTA;
    }

    if (this->rightFOVborder - delta <= this->data->timepoints.back().time) {
      this->rightFOVborder -= delta;
    } else {
      this->rightFOVborder = this->data->timepoints.back().time;
    }
  }

  this->plotUpdate();
};

void VCDPlotter::zoomView(int delta) {
  if (delta > 0) {

    if (this->rightFOVborder - this->leftFOVborder - 2 * delta >=
        this->MAX_ZOOM_DELTA) {
      this->rightFOVborder -= delta;
      this->leftFOVborder += delta;
    } else {
      this->leftFOVborder = (this->rightFOVborder + this->leftFOVborder) / 2 -
                            this->MAX_ZOOM_DELTA / 2;
      this->rightFOVborder = this->leftFOVborder + this->MAX_ZOOM_DELTA;
    }
  } else {
    if (this->leftFOVborder + delta >= 0) {
      this->leftFOVborder += delta;
    } else {
      this->leftFOVborder = 0;
    }

    if (!this->data->timepoints.size())
      return;

    if (this->rightFOVborder - delta <= this->data->timepoints.back().time) {
      this->rightFOVborder -= delta;
    } else {
      this->rightFOVborder = this->data->timepoints.back().time;
    }
  }
  this->plotUpdate();
}

void VCDPlotter::addSelectedDumps(QList<QTreeWidgetItem *> items,
                                  QList<VarData> varData) {
  this->selected_dumps->addTopLevelItems(items);
  this->dumpsList.append(items);
  this->varsList.append(varData);
  for (int i = 0; i < items.size(); i++) {

    this->waveStates.append({Qt::green, base::dec});
  }
  this->plotUpdate();
}

void VCDPlotter::wheelEvent(QWheelEvent *event) {
  // todo fix zoom to mouse (in the upper functions)
  // if shift, scroll left/right, if ctrl, zoom in out
  // where mouse points, if just like that, zoom in out
  // where marker is
  qDebug() << event->modifiers();
  int one = event->inverted() ? 1 : -1;

  bool shift = event->modifiers() & Qt::ShiftModifier;
  bool ctrl = event->modifiers() & Qt::ControlModifier;
  bool alt = event->modifiers() & Qt::AltModifier;

  if (shift && ctrl) {
    qDebug() << "marker";
  } else if (alt && ctrl) {
    qDebug() << "alt+ctrl";

    this->zoomView(event->angleDelta().x() > 0 ? (this->rightFOVborder-this->leftFOVborder)*0.1 : -(this->rightFOVborder-this->leftFOVborder)*0.1);
  } else if (alt) {
    qDebug() << "alt";

    this->sideShiftView(event->angleDelta().x() > 0 ? (this->rightFOVborder-this->leftFOVborder)*0.1 : -(this->rightFOVborder-this->leftFOVborder)*0.1);
  } else if (shift) {
    qDebug() << "shift";
    this->sideShiftView(event->angleDelta().y() > 0 ? 1 : -1);
  } else if (ctrl) {
    qDebug() << "mouse";
    this->zoomView(event->angleDelta().y() > 0 ? 1 : -1);
  } else {
    qDebug() << "height";
    // todo doesn't work
    this->vertScroll->setSliderPosition(
        this->vertScroll->sliderPosition() +
        one * (event->angleDelta().y() > 0 ? 1 : -1));
  }
  qDebug() << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
           << this->leftFOVborder + this->rightFOVborder << this->leftFOVborder
           << this->rightFOVborder;
}

void VCDPlotter::plotUpdate() {

  this->view->scrollHeight = this->currentHeight;
  if (this->dumpsList.length())
    this->view->itemRect =
        this->selected_dumps->visualItemRect(this->dumpsList[0]);

  this->view->waves->update();

  this->horizScroll->setMaximum(this->maxrightborder -
                                (this->rightFOVborder - this->leftFOVborder));
  this->horizScroll->setSliderPosition(this->leftFOVborder);
};
