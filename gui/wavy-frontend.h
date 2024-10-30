#pragma once
#include <qboxlayout.h>
#include <qscrollarea.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qtreeview.h>
#include <string>
#include <ui_mainwindow.h>
#include <vector>
#include "../lib/vcd-parser/vcd-data.h"

class WavyFrontend {

public:
  WavyFrontend(Ui::MainWindow *ui);

  QVBoxLayout *selected_dumps_list;
  QTabWidget *waveform_tabs;
  QTabWidget *sidebar;
  QToolButton *sidebar_objects_button_close;
  QToolButton *sidebar_scope_button_close;
  QListWidget *objects_list;
  QListWidget *dumps_list;
  QTreeView *scope_tree;
  QScrollArea *waveform_scroll;
  QAction *action_open;
  QAction *action_save;

  void loadVCDData(std::string path);


private:
  Ui::MainWindow *ui;
  std::vector<VCDData *> vcdDatas;
};
