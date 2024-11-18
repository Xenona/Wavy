#pragma once
#include <QTreeWidget>

class ScopeTreeWidget: QTreeWidget {

public:
  ScopeTreeWidget();
  ~ScopeTreeWidget() = default;

  QTreeWidgetItem *currentItem;
  
};
