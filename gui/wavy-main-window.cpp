#include "wavy-main-window.h"
#include "../lib/vcd-parser/vcd-char-stream.h"
#include "../lib/vcd-parser/vcd-token-stream.h"
#include "ui_wavy-main-window.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPair>
#include <exception>
#include <qaction.h>
#include <qdebug.h>
#include <qdialog.h>
#include <qlogging.h>
#include <qobject.h>
#include <qstyle.h>
#include <qtabwidget.h>
#include <qtreewidget.h>
#include <qwidget.h>
#include <string>

WavyMainWindow::WavyMainWindow() : ui(new Ui::WavyMainWindow) {
  this->ui->setupUi(this);
  this->selected_dumps_list = ui->selected_dumps_list;
  this->waveform_tabs = ui->waveform_tabs;
  this->sidebar = ui->sidebar;
  this->sidebar_objects_button_close = ui->sidebar_objects_button_close;
  this->sidebar_scope_button_close = ui->sidebar_scope_button_close;
  this->objects_list = ui->objects_list;
  this->dumps_list = ui->dumps_list;
  this->sidebar_scope_scroll_container = ui->sidebar_scope_scroll_container;
  this->waveform_scroll = ui->waveform_scroll;
  this->action_open = ui->action_open;
  this->action_save = ui->action_save;

  this->parser = new VCDParser();

  QObject::connect(ui->action_open, &QAction::triggered, this, [this]() {
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open VCD file"), "", tr("VCD files (*.vcd);;All Files (*)"));
    if (filename.isEmpty())
      return;
    this->loadVCDData(filename.toStdString());
  });

  QObject::connect(ui->waveform_tabs, &QTabWidget::currentChanged, this, [this](int index) {
    QWidget* tab = this->waveform_tabs->widget(index);

    for (auto [key, value] : this->vcdDataFiles.asKeyValueRange()) {
      if (value.tab == tab) {
        this->setVCDDataActive(key);
      }
    }
  });
}

void WavyMainWindow::loadVCDData(std::string path) {
  // load data
  VCDData *VCDData;

  try {
    VCDData =
        this->parser->getVCDData(new VCDTokenStream(new VCDCharStream(path)));
  } catch (const std::exception &e) {
    QMessageBox msgBox;
    msgBox.setText(
        QString::fromStdString(std::string{"Error happened: "} + e.what()));
    msgBox.exec();
  }

  QString qstring_path = QString::fromStdString(path);

  if (!this->vcdDataFiles.contains(qstring_path)) {
    // creating tab
    QWidget *tab = new QWidget();
    tab->setObjectName(path+"_tab");
    this->waveform_tabs->addTab(tab,
                                QString::fromStdString(path).split("/").last());

    // creating scope tree
    QTreeWidget* treeWidget = this->createTreeWidget(VCDData->scopes);
    treeWidget->setObjectName(path+"_tree");

    this->vcdDataFiles.insert(qstring_path, {VCDData, tab, treeWidget});
    this->_VCDDataActive = qstring_path;
  }
    // TODO callsetVCDDataActive;
  this->setVCDDataActive(qstring_path);
}

QTreeWidget *WavyMainWindow::createTreeWidget(std::vector<ScopeData> data) {

  QTreeWidget *scopeTree =
      new QTreeWidget(this->ui->sidebar_scope_scroll_container);

  struct TreeNodeInfo {
    QTreeWidgetItem *self;
  };

  QMap<QString, TreeNodeInfo> treeMap;

  int i = 0;
  int created = 0;
  while (i < data.size() && created < data.size()) {
    auto &datum = data[i];
    if (datum.ID == "") {
      i++;
    } else {

      // root
      if (datum.parentScopeID == "") {

        QTreeWidgetItem *item = new QTreeWidgetItem(scopeTree);
        item->setText(0, QString::fromStdString(datum.name));
        // todo switch by datum.type
        // item->setIcon(int column, const QIcon &aicon)

        treeMap.insert(QString::fromStdString(datum.ID), {item});
        created++;
        datum.ID = "";
      } else {
        QString keyParentID = QString::fromStdString(datum.parentScopeID);
        if (treeMap.contains(keyParentID)) {

          auto parent = treeMap.value(keyParentID);

          QTreeWidgetItem *item = new QTreeWidgetItem(parent.self);
          item->setText(0, QString::fromStdString(datum.name));

          treeMap.insert(QString::fromStdString(datum.ID), {item});
          datum.ID = "";
          created++;

          for (auto &var : datum.vars) {
            QTreeWidgetItem *varItem = new QTreeWidgetItem(item);
            varItem->setText(0, QString::fromStdString(var.trueName));
            // todo 
            // set a proper icon depending on a var.type
            if (var.size > 1) {
              for (int i = 0; i < var.size; i++) {
                QTreeWidgetItem *varItemVector = new QTreeWidgetItem(varItem);
                varItemVector->setText(0, QString::fromStdString(var.trueName + " [" + std::to_string(i) + "]"));

              }
            }

          }

        } else {
          i++;
          continue;
        }
      };
    }
    if (i >= data.size()) i = 0;
  }

  return scopeTree;
}

void WavyMainWindow::setVCDDataActive(QString path) {

  if (!this->vcdDataFiles.contains(path)) {
    throw "One should not set active non-existing file";
    return;
  }


  // setting tab
  this->waveform_tabs->setCurrentWidget(this->vcdDataFiles.value(path).tab);

  // filling scope tab
  this->vcdDataFiles.value(this->_VCDDataActive).scopeTree->setParent(NULL);
  this->vcdDataFiles.value(path).scopeTree->setParent(this->sidebar_scope_scroll_container);
  this->vcdDataFiles.value(path).scopeTree->setGeometry({0, 0, 1000, 1000});
  this->vcdDataFiles.value(path).scopeTree->setHeaderHidden(true);
  this->vcdDataFiles.value(path).scopeTree->show();
  // todo 
  // make scrollbar appear when length is too much
  this->vcdDataFiles.value(path).scopeTree->resizeColumnToContents(0);

  // setting current file focused;
  this->_VCDDataActive = path;
}
