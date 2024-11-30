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
  QWidget *sidebar_objects_scroll_container;
  QWidget *sidebar_scope_scroll_container;
  QVBoxLayout *l_9;
  QVBoxLayout *l_10;
  QScrollArea *waveform_scroll;
  QAction *action_open;
  QAction *action_save;

  void loadVCDData(std::string path);
  void setVCDDataActive(QString path);
  QString VCDDataActive();
  void removeActiveVCD();

  QTreeWidget* createTreeWidget(std::vector<ScopeData> data);
  void closeTab(int idx);

private:
  QMap<QString, GUIVCDInfo> vcdDataFiles;
  VCDParser* parser;
  QString _VCDDataActive;

protected:
  Ui::WavyMainWindow *ui;

};
