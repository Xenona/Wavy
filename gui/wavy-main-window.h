#pragma once

#include <QMainWindow>
#include <qcontainerfwd.h>
#include <qgraphicsscene.h>
#include "../lib/vcd-parser/vcd-data.h"
#include "../lib/vcd-parser/vcd-parser.h"
#include <qlistwidget.h>
#include <qscrollarea.h>
#include <qtabwidget.h>
#include <qtmetamacros.h>
#include <qtoolbutton.h>
#include <qtreeview.h>
#include <qboxlayout.h>
#include <qtreewidget.h>
#include <qwidget.h>
#include <QTreeWidget>
#include "info-table-widget.h"
#include "scope-tree-widget.h"
#include "vcd-plotter.h"
#include <QTableWidget>

struct GUIVCDInfo {
  VCDData* vcddata;
  VCDPlotter* tab;
  ScopeTreeWidget* scopeTree;
  InfoTableWidget* table;
};


QT_BEGIN_NAMESPACE
namespace Ui {
class WavyMainWindow;
}
QT_END_NAMESPACE

class WavyMainWindow : public QMainWindow {
  Q_OBJECT

public:
  WavyMainWindow();
  ~WavyMainWindow();
  
  QTabWidget *waveform_tabs;
  QTabWidget *sidebar;
  QToolButton *sidebar_objects_button_close;
  QToolButton *sidebar_scope_button_close;
  QToolButton *sidebar_scope_button_ok;
  QToolButton *sidebar_scope_button_save;
  QWidget *sidebar_objects_scroll_container;
  QWidget *sidebar_scope_scroll_container;
  QVBoxLayout *l_9;
  QVBoxLayout *l_10;
  QScrollArea *waveform_scroll;
  QAction *action_open;

  void loadVCDData(std::string path, VCDData*data=nullptr);
  void setVCDDataActive(QString path);
  QString VCDDataActive();
  void removeActiveVCD();
  void saveVCD();

  bool eventFilter(QObject *obj, QEvent *event);
  void addSelectedDumps();

  QTreeWidget* createTreeWidget(std::vector<ScopeData> data);
  void closeTab(int idx);
  Ui::WavyMainWindow *ui;

private:
  QMap<QString, GUIVCDInfo> vcdDataFiles;
  VCDParser* parser;
  QString _VCDDataActive;

};
