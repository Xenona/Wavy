#include "wavy-main-window.h"
#include "../lib/vcd-parser/vcd-char-stream.h"
#include "../lib/vcd-parser/vcd-token-stream.h"
#include "info-table-widget.h"
#include "scope-tree-widget.h"
#include "ui_capture_device_dialog.h"
#include "ui_wavy-main-window.h"
#include "vcd-plotter.h"
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPair>
#include <QSizePolicy>
#include <cstddef>
#include <exception>
#include <qabstractitemmodel.h>
#include <qabstractscrollarea.h>
#include <qaction.h>
#include <qdebug.h>
#include <qdialog.h>
#include <qfont.h>
#include <qlogging.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsizepolicy.h>
#include <qstyle.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qtreewidget.h>
#include <qwidget.h>
#include <string>
#include "capture_device_dialog.h"
#include <fcntl.h>


WavyMainWindow::WavyMainWindow() : ui(new Ui::WavyMainWindow) {
  this->ui->setupUi(this);
  this->waveform_tabs = ui->waveform_tabs;
  this->sidebar = ui->sidebar;
  this->sidebar_scope_button_ok = ui->sidebar_scope_button_ok;
  this->sidebar_objects_scroll_container = ui->sidebar_objects_scroll_container;
  this->sidebar_scope_scroll_container = ui->sidebar_scope_scroll_container;
  this->l_9 = ui->l_9;
  this->l_10 = ui->l_10;
  this->action_open = ui->action_open;

  this->parser = new VCDParser();
  qApp->installEventFilter(this);

  QObject::connect(this->action_open, &QAction::triggered, this, [this]() {
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open VCD file"), "", tr("VCD files (*.vcd);;All Files (*)"));
    if (filename.isEmpty())
      return;
    this->loadVCDData(filename.toStdString());
  });

  QObject::connect(
      ui->waveform_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        QWidget *tab = this->waveform_tabs->widget(index);

        for (auto [key, value] : this->vcdDataFiles.asKeyValueRange()) {
          if (value.tab == tab) {
            this->setVCDDataActive(key);
          }
        }
      });

  QObject::connect(this->sidebar_scope_button_ok, &QToolButton::clicked, this,
                   &WavyMainWindow::addSelectedDumps);

  QObject::connect(this->waveform_tabs, &QTabWidget::tabCloseRequested, this,
                   &WavyMainWindow::closeTab);


  QObject::connect(this->ui->actionCapture_device, &QAction::triggered, this, [this]() {
    
    int fd = open("/dev/pikernely0", O_RDWR);
    if (fd < 0) {
      QMessageBox msgBox;
      msgBox.setText("Could not open device. Did you load a module?");
      msgBox.exec();
      return;
    }
    
    auto d = new CaprureDeviceDialog(fd, this);
    // a->setupUi(d);
    if (d->exec() != QDialog::Rejected) {

    std::string name = "Pico";
    this->loadVCDData(name, d->data);
    }
    // if d->e();

    
  });
}

void WavyMainWindow::addSelectedDumps() {
  QList<QTreeWidgetItem *> selectedElements =
      this->vcdDataFiles.value(this->_VCDDataActive).scopeTree->selectedItems();

  QList<QTreeWidgetItem *> items;
  QList<VarData> varData;
  for (auto it : selectedElements) {
    auto newItem = it->clone();
    auto f = QFont(newItem->font(0));
    f.setPointSize(16);
    newItem->setFont(0, f);
    items.append(newItem);
    varData.append(this->vcdDataFiles.value(this->_VCDDataActive)
                       .scopeTree->varData.value(it));
  }

  this->vcdDataFiles.value(this->_VCDDataActive)
      .tab->addSelectedDumps(items, varData);
}

bool WavyMainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    if (this->vcdDataFiles.count(_VCDDataActive)) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (obj == this->vcdDataFiles.value(_VCDDataActive).tab->selected_dumps) {

        if (keyEvent->key() == Qt::Key_Delete) {

          auto &list = this->vcdDataFiles.value(_VCDDataActive).tab->dumpsList;
          auto &tree =
              this->vcdDataFiles.value(_VCDDataActive).tab->selected_dumps;
          auto items = tree->selectedItems();
          for (int i = 0; i < items.size(); i++) {
            for (int j = 0; j < list.size();) {
              if (items[i] == list[j]) {
                list.erase(list.begin() + j);
                this->vcdDataFiles.value(_VCDDataActive)
                    .tab->varsList.erase(
                        this->vcdDataFiles.value(_VCDDataActive)
                            .tab->varsList.begin() +
                        j);
                this->vcdDataFiles.value(_VCDDataActive)
                    .tab->waveStates.erase(
                        this->vcdDataFiles.value(_VCDDataActive)
                            .tab->waveStates.begin() +
                        j);
                break;
              } else {
                j++;
              }
            }
          }

          tree->clearSelection();
          for (auto item : items) {
            delete item;
          }
          this->vcdDataFiles.value(_VCDDataActive).tab->plotUpdate();
        }
      } else if (obj == this->vcdDataFiles.value(_VCDDataActive).scopeTree) {
        if (keyEvent->key() == Qt::Key_Return) {
          this->addSelectedDumps();
        }
      }
    }
  }
  return QObject::eventFilter(obj, event);
}

void WavyMainWindow::closeTab(int idx) {
  // this->waveform_tabs->removeTab(idx);
  VCDPlotter *tab = static_cast<VCDPlotter *>(this->waveform_tabs->widget(idx));

  this->_VCDDataActive = tab->path;
  this->removeActiveVCD();
};

void WavyMainWindow::loadVCDData(std::string path, VCDData*data) {
  // load data
  VCDData *VCDData;

  try {
    if (data==nullptr) {

    VCDData =
        this->parser->getVCDData(new VCDTokenStream(new VCDCharStream(path)));
    } else {
      VCDData=data;
    }

    if (VCDData->errors.size()) {
      QMessageBox msgBox;
      msgBox.setText(QString::fromStdString(std::string{
          VCDData->warns.size()
              ? "Found " + std::to_string(VCDData->warns.size()) +
                    " warnings. Also "
              : "" + std::to_string(VCDData->errors.size()) +
                    " errors found. File can't be processed. Please, address "
                    "the issues on the 'Details' tab and try again."}));
      msgBox.exec();
    } else if (VCDData->warns.size()) {
      QMessageBox msgBox;
      msgBox.setText(QString::fromStdString(
          std::string{"Found " + std::to_string(VCDData->warns.size()) +
                      " warnings. Please, proceed to the 'Details' tab "}));
      msgBox.exec();
    }
  } catch (const std::exception &e) {
    QMessageBox msgBox;
    msgBox.setText(
        QString::fromStdString(std::string{"Error happened: "} + e.what()));
    msgBox.exec();
  }

  QString qstring_path = QString::fromStdString(path);

  if (!this->vcdDataFiles.contains(qstring_path)) {
    // creating tab
    VCDPlotter *tab = new VCDPlotter(qstring_path, VCDData, this, this);
    this->waveform_tabs->addTab(tab,
                                QString::fromStdString(path).split("/").last());

    // creating scope tree
    ScopeTreeWidget *treeWidget = new ScopeTreeWidget(VCDData->scopes);

    InfoTableWidget *table = new InfoTableWidget(VCDData);

    this->vcdDataFiles.insert(qstring_path, {VCDData, tab, treeWidget, table});

    QObject::connect(treeWidget, &QTreeWidget::itemSelectionChanged, this,
                     [this]() {
                       QList<QTreeWidgetItem *> selectedElements =
                           this->vcdDataFiles.value(this->_VCDDataActive)
                               .scopeTree->selectedItems();
                       this->sidebar_scope_button_ok->setDisabled(
                           selectedElements.size() == 0);
                     });

    // this->_VCDDataActive = qstring_path;
  }
  this->setVCDDataActive(qstring_path);
}

void WavyMainWindow::setVCDDataActive(QString path) {

  if (!this->vcdDataFiles.contains(path)) {
    throw "One should not set active non-existing file";
    return;
  }

  // setting tab
  this->waveform_tabs->setCurrentWidget(this->vcdDataFiles.value(path).tab);

  // filling scope tab
  if (this->vcdDataFiles.contains(_VCDDataActive)) {
    this->vcdDataFiles.value(_VCDDataActive).scopeTree->setParent(NULL);
    this->vcdDataFiles.value(_VCDDataActive).table->setParent(NULL);
  }

  this->vcdDataFiles.value(path).scopeTree->setParent(
      this->sidebar_scope_scroll_container);
  this->vcdDataFiles.value(path).table->setParent(
      this->sidebar_objects_scroll_container);

  this->l_9->removeWidget(this->vcdDataFiles.value(_VCDDataActive).scopeTree);
  this->l_9->addWidget(this->vcdDataFiles.value(path).scopeTree);

  this->l_10->removeWidget(this->vcdDataFiles.value(_VCDDataActive).table);
  this->l_10->addWidget(this->vcdDataFiles.value(path).table);

  this->vcdDataFiles.value(path).scopeTree->show();
  this->vcdDataFiles.value(path).table->show();

  // this->vcdDataFiles.value(_VCDDataActive).scopeTree->layout->replaceWidget(this->vcdDataFiles.value(_VCDDataActive).scopeTree,
  // this->vcdDataFiles.value(path).scopeTree);
  // this->vcdDataFiles.value(path).scopeTree->layout->setParent(this->sidebar_scope_scroll_container);
  // auto res =
  // this->l_9->replaceWidget(this->vcdDataFiles.value(_VCDDataActive).scopeTree,
  // this->vcdDataFiles.value(path).scopeTree); if (res == nullptr ){ qDebug()
  // << "JHE";

  // }

  // todo
  // make scrollbar appear when length is too much
  this->vcdDataFiles.value(path).scopeTree->resizeColumnToContents(0);

  // setting current file focused;
  this->_VCDDataActive = path;
}

void WavyMainWindow::removeActiveVCD() {
  if (!this->vcdDataFiles.empty()) {
    GUIVCDInfo info = this->vcdDataFiles.value(this->_VCDDataActive);
    if (info.scopeTree->selectedItems().size())
      info.scopeTree->clearSelection();

    QMap<QString, GUIVCDInfo>::iterator it;
    QString next;
    QString prevPath;
    for (it = this->vcdDataFiles.begin(); it != this->vcdDataFiles.end();
         ++it) {
      if (it.key() != _VCDDataActive) {
        next = it.key();
        break;
      }
    }

    prevPath = this->_VCDDataActive;
    if (next != "") {
      this->setVCDDataActive(next);
    } else {
      this->_VCDDataActive = "";
    }
    this->vcdDataFiles.remove(prevPath);

    info.scopeTree->setParent(NULL);
    info.table->setParent(NULL);
    delete info.vcddata;
    delete info.tab;
    delete info.scopeTree;
  }
  // this->sidebar_scope_scroll_container->children();
}

WavyMainWindow::~WavyMainWindow() {
  while (!this->vcdDataFiles.empty()) {
    removeActiveVCD();
  }
  delete this->parser;
  delete this->ui;
}
