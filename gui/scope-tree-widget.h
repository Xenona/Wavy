#pragma once
#include <QTreeWidget>
#include <qtmetamacros.h>
#include <qtreewidget.h>
#include "../lib/vcd-parser/vcd-data.h"
#include <QMainWindow>

class ScopeTreeWidget: public QTreeWidget {

public:
  ScopeTreeWidget(std::vector<ScopeData> data, QWidget *parent = nullptr);
  ~ScopeTreeWidget();

  QMap<QTreeWidgetItem*, VarData> varData;

};
