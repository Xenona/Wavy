#include "../../../lib/vcd-parser/vcd-data.h"
#include <QDateTime>
#include <QDialog>
#include <QTableWidgetItem>
#include <cstdint>


#pragma once

struct TableInfo {
  uint64_t bytes;
  uint64_t events;
  short pins;
};

namespace Ui {
class CaprureDeviceDialog;
}

class CaprureDeviceDialog : public QDialog {
  Q_OBJECT

public:
  CaprureDeviceDialog(int fd, QWidget *parent = 0);
  ~CaprureDeviceDialog();

  VCDData *data = nullptr;
  void start();
  void prepare();
  void stop();
  void readAll(CaprureDeviceDialog *cdd);

  static void *readAllTrampoline(void *arg) {
    CaprureDeviceDialog *cdd = (CaprureDeviceDialog *)arg;
    cdd->readAll(cdd);
    return nullptr;
  };
  QString timeDifference(const QDateTime &start, const QDateTime &end);

  pthread_mutex_t data_mutex;
  pthread_mutex_t pico_mutex;
  pthread_mutex_t table_mutex;

  QPushButton *buttonStart;
  QPushButton *buttonDone;
  QPushButton *buttonStop;
  QPushButton *buttonCancel;

  QTableWidgetItem *item_bytes;
  QTableWidgetItem *item_events;
  QTableWidgetItem *item_pins_active;
  QTableWidgetItem *item_time_passed;

  TableInfo info;

  QTimer *timer;
  QDateTime prevTime;

  bool isRunning = false;

private:
  Ui::CaprureDeviceDialog
      *ui; // This will be the acces to the widgets defined in .ui
  int fd;
  pthread_t pt;
};
