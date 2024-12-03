#include "../../../lib/vcd-parser/vcd-data.h"
#include <QDialog>

#pragma once

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

  pthread_mutex_t data_mutex;
  pthread_mutex_t pico_mutex;

private:
  Ui::CaprureDeviceDialog
      *ui; // This will be the acces to the widgets defined in .ui
  int fd;
  pthread_t pt;
};
