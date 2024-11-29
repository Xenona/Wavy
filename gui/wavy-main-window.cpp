#include "wavy-main-window.h"
#include "../lib/vcd-parser/vcd-char-stream.h"
#include "../lib/vcd-parser/vcd-token-stream.h"
#include "scope-tree-widget.h"
#include "ui_wavy-main-window.h"
#include "vcd-plotter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPair>
#include <exception>
#include <qabstractitemmodel.h>
#include <qabstractscrollarea.h>
#include <qaction.h>
#include <qdebug.h>
#include <qdialog.h>
#include <qfont.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qstyle.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qtreewidget.h>
#include <qwidget.h>
#include <string>

WavyMainWindow::WavyMainWindow() : ui(new Ui::WavyMainWindow) {
  this->ui->setupUi(this);
  this->waveform_tabs = ui->waveform_tabs;
  this->sidebar = ui->sidebar;
  this->sidebar_objects_button_close = ui->sidebar_objects_button_close;
  this->sidebar_scope_button_close = ui->sidebar_scope_button_close;
  this->sidebar_scope_button_ok = ui->sidebar_scope_button_ok;
  this->objects_list = ui->objects_list;
  this->sidebar_scope_scroll_container = ui->sidebar_scope_scroll_container;
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

  QObject::connect(
      ui->waveform_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        QWidget *tab = this->waveform_tabs->widget(index);

        for (auto [key, value] : this->vcdDataFiles.asKeyValueRange()) {
          if (value.tab == tab) {
            this->setVCDDataActive(key);
          }
        }
      });

  QObject::connect(
      this->sidebar_scope_button_ok, &QToolButton::clicked, this, [this]() {
        QList<QTreeWidgetItem *> selectedElements =
            this->vcdDataFiles.value(this->_VCDDataActive)
                .scopeTree->selectedItems();

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
      });

          QObject::connect(this->waveform_tabs, &QTabWidget::tabCloseRequested, this,
                     &WavyMainWindow::closeTab);
}

void WavyMainWindow::closeTab(int idx) {
  qDebug() << "idx" << idx;
  // this->waveform_tabs->removeTab(idx);
  VCDPlotter* tab = static_cast<VCDPlotter*>(this->waveform_tabs->widget(idx));

  this->_VCDDataActive = tab->path;
  this->removeActiveVCD();

};

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
    VCDPlotter *tab = new VCDPlotter(qstring_path, VCDData, this);
    this->waveform_tabs->addTab(tab,
                                QString::fromStdString(path).split("/").last());

    // creating scope tree
    ScopeTreeWidget *treeWidget = new ScopeTreeWidget(
        VCDData->scopes, this->ui->sidebar_scope_scroll_container);


    this->vcdDataFiles.insert(qstring_path, {VCDData, tab, treeWidget});

    QObject::connect(treeWidget, &QTreeWidget::itemSelectionChanged, this,
                     [this]() {
                       QList<QTreeWidgetItem *> selectedElements =
                           this->vcdDataFiles.value(this->_VCDDataActive)
                               .scopeTree->selectedItems();
                       this->sidebar_scope_button_ok->setDisabled(
                           selectedElements.size() == 0);
                     });

    this->_VCDDataActive = qstring_path;
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
  this->vcdDataFiles.value(path).scopeTree->setParent(NULL);
  this->vcdDataFiles.value(path).scopeTree->setParent(
      this->sidebar_scope_scroll_container);
  this->vcdDataFiles.value(path).scopeTree->setGeometry({0, 0, 1000, 1000});
  this->vcdDataFiles.value(path).scopeTree->show();

  // todo
  // make scrollbar appear when length is too much
  this->vcdDataFiles.value(path).scopeTree->resizeColumnToContents(0);

  // setting current file focused;
  this->_VCDDataActive = path;
}

void WavyMainWindow::removeActiveVCD() {
  if (!this->vcdDataFiles.empty()) {
    GUIVCDInfo info = this->vcdDataFiles.value(this->_VCDDataActive);
    info.scopeTree->clearSelection();
    this->vcdDataFiles.remove(this->_VCDDataActive);

    if (!this->vcdDataFiles.empty()) {

    QString next = this->vcdDataFiles.cbegin().key();
    this->setVCDDataActive(next);
    } 

    info.scopeTree->setParent(NULL);
    delete info.vcddata;
    delete info.tab;
    delete info.scopeTree;
  }
  this->sidebar_scope_scroll_container->children();
}

WavyMainWindow::~WavyMainWindow() {
  while (!this->vcdDataFiles.empty()) {
    removeActiveVCD();
  }
  delete this->parser;
  delete this->ui;
}
