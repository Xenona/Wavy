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

struct GUIVCDInfo {
  VCDData* vcddata;
  QWidget* tab;
  QTreeWidget* scopeTree;
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
  
  QVBoxLayout *selected_dumps_list;
  QTabWidget *waveform_tabs;
  QTabWidget *sidebar;
  QToolButton *sidebar_objects_button_close;
  QToolButton *sidebar_scope_button_close;
  QListWidget *objects_list;
  QListWidget *dumps_list;
  QWidget *sidebar_scope_scroll_container;
  QScrollArea *waveform_scroll;
  QAction *action_open;
  QAction *action_save;

  void loadVCDData(std::string path);
  void setVCDDataActive(QString path);
  QString VCDDataActive();

  QTreeWidget* createTreeWidget(std::vector<ScopeData> data);

private:
  QMap<QString, GUIVCDInfo> vcdDataFiles;
  VCDParser* parser;
  QString _VCDDataActive;

protected:
  Ui::WavyMainWindow *ui;

};
