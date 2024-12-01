#pragma once
#include <QTreeWidget>
#include <qtmetamacros.h>
#include <qtreewidget.h>
#include "../lib/vcd-parser/vcd-data.h"
#include <QVBoxLayout>


class ScopeTreeWidget: public QTreeWidget {

public:
  ScopeTreeWidget(std::vector<ScopeData> data);
  ~ScopeTreeWidget();

  QMap<QTreeWidgetItem*, VarData> varData;
  QVBoxLayout* layout;
};
