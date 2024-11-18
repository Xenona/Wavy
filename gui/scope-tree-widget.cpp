#include "scope-tree-widget.h"
#include "wavy-main-window.h"

ScopeTreeWidget::ScopeTreeWidget(std::vector<ScopeData> data, QWidget *parent) {

  QTreeWidget *scopeTree = new QTreeWidget(parent);

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
        item->setFlags(item->flags() & (~Qt::ItemIsSelectable));

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
          item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
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
                varItemVector->setText(
                    0, QString::fromStdString(var.trueName + " [" +
                                              std::to_string(i) + "]"));
              }
            }
          }

        } else {
          i++;
          continue;
        }
      };
    }
    if (i >= data.size())
      i = 0;
  }

  scopeTree->setHeaderHidden(true);
  scopeTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
  scopeTree->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
}
