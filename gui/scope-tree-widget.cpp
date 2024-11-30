#include "scope-tree-widget.h"
#include "wavy-main-window.h"
#include <qboxlayout.h>
#include <qobject.h>
#include <qtreewidget.h>
#include <string>

ScopeTreeWidget::ScopeTreeWidget(std::vector<ScopeData> data) {

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

      QTreeWidgetItem *item;
      // root
      if (datum.parentScopeID == "") {

        item = new QTreeWidgetItem(this);
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

          item = new QTreeWidgetItem(parent.self);
          item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
          item->setText(0, QString::fromStdString(datum.name));

          treeMap.insert(QString::fromStdString(datum.ID), {item});
          datum.ID = "";
          created++;

        } else {
          i++;
          continue;
        }
      };

      for (auto &var : datum.vars) {
        QTreeWidgetItem *varItem = new QTreeWidgetItem(item);

        this->varData.insert(varItem, var);

        varItem->setText(0, QString::fromStdString(var.trueName + " [" + std::to_string(var.size) + ":0]"));
        // todo
        // set a proper icon depending on a var.type
        // if (var.size > 1) {
        //   for (int i = 0; i < var.size; i++) {
        //     QTreeWidgetItem *varItemVector = new QTreeWidgetItem(varItem);
        //     varItemVector->setText(
        //         0, QString::fromStdString(var.trueName + " [" +
        //                                   std::to_string(i) + "]"));
        //   }
        // }
      }
    }
    if (i >= data.size())
      i = 0;
  }

  this->setHeaderHidden(true);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  // layout = new QVBoxLayout();
  // parent->setLayout(layout);
  // layout->setContentsMargins(0, 0, 0, 0);
  // l->addWidget(this);

}

ScopeTreeWidget::~ScopeTreeWidget() {
  while (!this->varData.isEmpty()) {
    auto key = this->varData.cbegin().key();
    this->varData.remove(key);
    delete key;
  }
}
