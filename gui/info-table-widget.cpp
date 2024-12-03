#include "info-table-widget.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QVBoxLayout>
#include <qtablewidget.h>
#include <string>

InfoTableWidget::InfoTableWidget(VCDData *data) {

  if (this->columnCount() < 1)
    this->setColumnCount(1);
  QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
  __qtablewidgetitem->setTextAlignment(Qt::AlignLeading | Qt::AlignVCenter);
  this->setHorizontalHeaderItem(0, __qtablewidgetitem);
  if (this->rowCount() < 7)
    this->setRowCount(7);
  QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
  this->setVerticalHeaderItem(0, __qtablewidgetitem1);
  QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
  this->setVerticalHeaderItem(1, __qtablewidgetitem2);
  QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
  this->setVerticalHeaderItem(2, __qtablewidgetitem3);
  QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
  this->setVerticalHeaderItem(3, __qtablewidgetitem4);
  QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
  this->setVerticalHeaderItem(4, __qtablewidgetitem5);
  QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
  this->setVerticalHeaderItem(5, __qtablewidgetitem6);
  QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
  this->setVerticalHeaderItem(6, __qtablewidgetitem7);
  QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
  __qtablewidgetitem8->setFlags(Qt::ItemIsEnabled);
  this->setItem(0, 0, __qtablewidgetitem8);
  QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
  __qtablewidgetitem9->setFlags(Qt::ItemIsEnabled);
  this->setItem(1, 0, __qtablewidgetitem9);
  QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
  __qtablewidgetitem10->setFlags(Qt::ItemIsEnabled);
  this->setItem(2, 0, __qtablewidgetitem10);
  QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
  __qtablewidgetitem11->setFlags(Qt::ItemIsEnabled);
  this->setItem(3, 0, __qtablewidgetitem11);
  QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
  __qtablewidgetitem12->setFlags(Qt::ItemIsEnabled);
  this->setItem(4, 0, __qtablewidgetitem12);
  QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
  __qtablewidgetitem13->setFlags(Qt::ItemIsEnabled);
  this->setItem(5, 0, __qtablewidgetitem13);
  QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
  __qtablewidgetitem14->setFlags(Qt::ItemIsEnabled);
  this->setItem(6, 0, __qtablewidgetitem14);
  this->setObjectName("info_table");
  this->horizontalHeader()->setVisible(false);
  this->horizontalHeader()->setStretchLastSection(true);
  this->verticalHeader()->setVisible(true);
  this->verticalHeader()->setStretchLastSection(false);

  QTableWidgetItem *___qtablewidgetitem = this->horizontalHeaderItem(0);
  ___qtablewidgetitem->setText(
      QCoreApplication::translate("WavyMainWindow", "New Column", nullptr));
  QTableWidgetItem *___qtablewidgetitem1 = this->verticalHeaderItem(0);
  ___qtablewidgetitem1->setText(
      QCoreApplication::translate("WavyMainWindow", "Creation Date", nullptr));
  QTableWidgetItem *___qtablewidgetitem2 = this->verticalHeaderItem(1);
  ___qtablewidgetitem2->setText(
      QCoreApplication::translate("WavyMainWindow", "Tool", nullptr));
  QTableWidgetItem *___qtablewidgetitem3 = this->verticalHeaderItem(2);
  ___qtablewidgetitem3->setText(
      QCoreApplication::translate("WavyMainWindow", "Timescale", nullptr));
  QTableWidgetItem *___qtablewidgetitem4 = this->verticalHeaderItem(3);
  ___qtablewidgetitem4->setText(
      QCoreApplication::translate("WavyMainWindow", "Comments", nullptr));
  QTableWidgetItem *___qtablewidgetitem5 = this->verticalHeaderItem(4);
  ___qtablewidgetitem5->setText(
      QCoreApplication::translate("WavyMainWindow", "Warnings", nullptr));
  QTableWidgetItem *___qtablewidgetitem6 = this->verticalHeaderItem(5);
  ___qtablewidgetitem6->setText(
      QCoreApplication::translate("WavyMainWindow", "Errors", nullptr));
  QTableWidgetItem *___qtablewidgetitem7 = this->verticalHeaderItem(6);
  ___qtablewidgetitem7->setText(
      QCoreApplication::translate("WavyMainWindow", "Length", nullptr));

  const bool __sortingEnabled = this->isSortingEnabled();
  this->setSortingEnabled(false);
  QTableWidgetItem *___qtablewidgetitem8 = this->item(0, 0);
  ___qtablewidgetitem8->setText(QString::fromStdString(data->date.date));
  QTableWidgetItem *___qtablewidgetitem9 = this->item(1, 0);
  ___qtablewidgetitem9->setText(QString::fromStdString(data->version.version));
  QTableWidgetItem *___qtablewidgetitem10 = this->item(2, 0);
  ___qtablewidgetitem10->setText(QString::fromStdString(
      std::to_string(data->timescale.precision) + " " + data->timescale.unit));
  QTableWidgetItem *___qtablewidgetitem11 = this->item(3, 0);
  QString cms;
  for (auto &c : data->comments) {
    cms += c.comment + '\n';
  }
  ___qtablewidgetitem11->setText(cms);
  QTableWidgetItem *___qtablewidgetitem12 = this->item(4, 0);
  QString warns;
  for (auto &w : data->warns) {
    warns += w + '\n';
  }
  ___qtablewidgetitem12->setText(warns);
  QTableWidgetItem *___qtablewidgetitem13 = this->item(5, 0);
  QString errors;
  for (auto &e : data->errors) {
    errors += e + '\n';
  }
  ___qtablewidgetitem13->setText(warns);
  QTableWidgetItem *___qtablewidgetitem14 = this->item(6, 0);
  if (data->timepoints.size())
    ___qtablewidgetitem14->setText(
        QString::fromStdString(std::to_string(data->timepoints.back().time)));

  this->setSortingEnabled(__sortingEnabled);
  this->resizeRowToContents(0);
  this->resizeRowToContents(1);
  this->resizeRowToContents(2);
  this->resizeRowToContents(3);
  this->resizeRowToContents(4);
  this->resizeRowToContents(5);
}
