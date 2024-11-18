#pragma once
#include <QTreeWidget>
#include "../lib/vcd-parser/vcd-data.h"

class ScopeTreeWidget: public QTreeWidget {

public:
  ScopeTreeWidget(std::vector<ScopeData> data, QWidget *parent = nullptr);
  ~ScopeTreeWidget() = default;

  QTreeWidgetItem *currentItem;
  
};
