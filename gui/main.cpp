#include "wavy-main-window.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  WavyMainWindow w;
  w.show();
  return a.exec();
}
