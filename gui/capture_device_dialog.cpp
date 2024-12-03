#include "capture_device_dialog.h"
#include "../pikernely/proto.h"
#include "ui_capture_device_dialog.h"
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <chrono>
#include <cstdint>
#include <fcntl.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <string>
#include <thread>
#include <unistd.h>

CaprureDeviceDialog::CaprureDeviceDialog(int fd, QWidget *parent)
    : QDialog(parent), ui(new Ui::CaprureDeviceDialog), fd(fd),
      timer(new QTimer) {
  ui->setupUi(this); // This will init all the widgets
  ui->buttonBox->clear();
  buttonCancel = new QPushButton("Cancel");
  ui->buttonBox->addButton(buttonCancel, QDialogButtonBox::RejectRole);
  buttonStop = new QPushButton("Stop");
  ui->buttonBox->addButton(buttonStop, QDialogButtonBox::ActionRole);

  // auto buttonPause = new QPushButton("Pause");
  // ui->buttonBox->addButton(buttonPause, QDialogButtonBox::ActionRole);
  buttonStart = new QPushButton("Start");
  ui->buttonBox->addButton(buttonStart, QDialogButtonBox::ActionRole);
  buttonDone = new QPushButton("Accept");
  ui->buttonBox->addButton(buttonDone, QDialogButtonBox::ActionRole);
  this->buttonStart->setVisible(true);
  this->buttonDone->setVisible(false);
  this->buttonStop->setVisible(false);
  item_bytes = this->ui->table->item(0, 0);
  item_events = this->ui->table->item(1, 0);
  item_pins_active = this->ui->table->item(2, 0);
  item_time_passed = this->ui->table->item(3, 0);
  auto f = this->ui->label_listening->font();
  f.setPointSize(20);

  this->ui->label_listening->setText("Click Start to capture pins");
  this->ui->label_listening->setFont(f);
  this->ui->label_listening->setAlignment(Qt::AlignCenter);

  QObject::connect(buttonStart, &QPushButton::clicked, this, [this]() {
    this->stop();
    this->prepare();
    this->start();
    pthread_create(&pt, NULL, &CaprureDeviceDialog::readAllTrampoline, this);
    this->isRunning = true;
    this->buttonStart->setVisible(false);
    this->buttonDone->setVisible(false);
    this->buttonStop->setVisible(true);
    this->prevTime = QDateTime::currentDateTime();
    timer->start(1000);
    auto f = this->ui->label_listening->font();
    f.setPointSize(20);
    this->ui->label_listening->setText("Capturing pins...");
    this->ui->label_listening->setFont(f);
    this->ui->label_listening->setAlignment(Qt::AlignCenter);
  });

  QObject::connect(buttonStop, &QPushButton::clicked, this, [this]() {
    this->stop();
    pthread_join(pt, NULL);
    printf("Kernel read all\n");
    this->isRunning = false;
    this->buttonDone->setVisible(true);
    this->buttonStart->setVisible(true);
    this->buttonStop->setVisible(false);

    pthread_mutex_lock(&this->data_mutex);
    auto d = this->data;
    pthread_mutex_unlock(&this->data_mutex);
    timer->stop();
    auto f = this->ui->label_listening->font();
    f.setPointSize(20);
    this->ui->label_listening->setText("Data gathered");
    this->ui->label_listening->setFont(f);
    this->ui->label_listening->setAlignment(Qt::AlignCenter);
  });

  QObject::connect(this->timer, &QTimer::timeout, this, [this]() {
    pthread_mutex_lock(&this->table_mutex);
    auto i = this->info;
    pthread_mutex_unlock(&this->table_mutex);

    item_bytes->setText(QString::fromStdString(std::to_string(i.bytes)));
    item_events->setText(QString::fromStdString(std::to_string(i.events)));
    item_pins_active->setText(QString::fromStdString(std::to_string(i.pins)));
    item_time_passed->setText(
        timeDifference(this->prevTime, QDateTime::currentDateTime()));
  });

  QObject::connect(buttonDone, &QPushButton::clicked, this, [this]() {
    pthread_mutex_lock(&this->data_mutex);
    auto d = this->data;
    pthread_mutex_unlock(&this->data_mutex);
    if (d->timepoints.size()) {
      this->accept();
    } else {
      QMessageBox msgBox;
      msgBox.setText("No changes were captured, can't show anything.");
      msgBox.exec();
    }
  });
  pthread_mutex_init(&this->data_mutex, nullptr);
  pthread_mutex_init(&this->pico_mutex, nullptr);
  pthread_mutex_init(&this->table_mutex, nullptr);
}

void CaprureDeviceDialog::prepare() {

  if (this->data != nullptr) {
    delete this->data;
  }
  this->data = new VCDData{};
  this->data->version.version = "XM Wave generator";
  this->data->scopes = {{
      .type = ScopeTypes::Module,
      .ID = "1",
      .name = "RPI Pico",
      .vars = {{.type = VarTypes::Wire,
                .size = 1,
                .identifier = "1",
                .trueName = "GP1"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "2",
                .trueName = "GP2"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "3",
                .trueName = "GP3"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "4",
                .trueName = "GP4"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "5",
                .trueName = "GP5"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "6",
                .trueName = "GP6"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "7",
                .trueName = "GP7"},
               {.type = VarTypes::Wire,
                .size = 1,
                .identifier = "8",
                .trueName = "GP8"}},

      .parentScopeID = "",
  }};
  this->data->timepoints = {};
  this->info={};
}

void CaprureDeviceDialog::start() {

  struct KERNELY_PKT c;
  c.state_flags = KERNELY_ENABLE;
  c.timer_period = 5;
  pthread_mutex_lock(&this->pico_mutex);
  write(fd, &c, sizeof(c));
  pthread_mutex_unlock(&this->pico_mutex);

  printf("Kernel start\n");
}

void CaprureDeviceDialog::stop() {
  struct KERNELY_PKT c;
  c.state_flags = KERNELY_FINISH;
  c.timer_period = 0;
  pthread_mutex_lock(&this->pico_mutex);
  write(fd, &c, sizeof(c));
  pthread_mutex_unlock(&this->pico_mutex);

  printf("Kernel finish\n");
}

CaprureDeviceDialog::~CaprureDeviceDialog() {
  pthread_mutex_destroy(&this->data_mutex);
  pthread_mutex_destroy(&this->pico_mutex);
  pthread_mutex_destroy(&this->table_mutex);
};

QString CaprureDeviceDialog::timeDifference(const QDateTime &start,
                                            const QDateTime &end) {
  qint64 secondsDiff = start.secsTo(end);

  int hours = secondsDiff / 3600;
  int minutes = (secondsDiff % 3600) / 60;
  int seconds = secondsDiff % 60;

  return QString("%1h:%2m:%3s")
      .arg(hours, 2, 10, QChar('0'))
      .arg(minutes, 2, 10, QChar('0'))
      .arg(seconds, 2, 10, QChar('0'));
}

void CaprureDeviceDialog::readAll(CaprureDeviceDialog *cdd) {
  uint16_t seq = 0;
  bool last = false;
  uint8_t prev = 0;
  uint64_t prev_time = 0;
  int samples = 0;

  uint64_t num_of_events = 0;

  uint64_t first_time = 0;
  short active_pins = 0;

  while (!last) {
    struct PICOY_PKT pkt;

    pthread_mutex_lock(&this->pico_mutex);
    int a = read(fd, &pkt, sizeof(pkt));
    pthread_mutex_unlock(&this->pico_mutex);

    if (a <= 0) {
      printf("%d", a);
      this->reject();
      QMessageBox msgBox;
      msgBox.setText("Error: RPI Pico got detached!");
      msgBox.exec();
      break;
    }

    last = pkt.packet_flags & PICOY_LAST;
    if (seq != 0 && pkt.packet_id != seq) {
      printf("Seems we lost packet %d %d\n", pkt.packet_id, seq);
    }
    seq = pkt.packet_id + 1;

    for (int i = 0; i < PICOY_BODY; i++) {
      samples++;
      if (pkt.data[i] != prev) {
        uint64_t time =
            pkt.time_start + i * pkt.time_duration / (PICOY_BODY - 1);
        if (first_time == 0) {
          first_time = time;
        }
        DumpData data = {.scals = {}};

        for (int k = 0; k <= 7; k++) {
          if (((prev >> k) & 1) != ((pkt.data[i] >> k) & 1)) {
            active_pins |= (1 << k);
            // printf("REad hhheee %8x\n", active_pins);
            num_of_events++;
            data.scals.push_back({.value = (pkt.data[i] >> k) & 1,
                                  .stringValue = std::to_string((pkt.data[i] >> k) & 1),
                                  .identifier = std::to_string(k + 1)});
          }
        }
        pthread_mutex_lock(&this->data_mutex);
        this->data->timepoints.push_back(
            {.time = (int)(time - first_time), .data = data});
        pthread_mutex_unlock(&this->data_mutex);
        printf("[%ldus](+ %ldus) =%x\n", time, time - prev_time, pkt.data[i]);
        prev_time = time;
        prev = pkt.data[i];
      }
    }
    // printf("Recieved packet %d started=%ld duration=%d last=%s data=%8x\n",
    //        pkt.packet_id, pkt.time_start, pkt.time_duration, last ? "y" : "n",
    //        pkt.data[0]);

    short count = 0;
    short active_pins_c = active_pins;

    while (active_pins_c) {
      count += (active_pins_c & 1);
      active_pins_c >>= 1;
    }
    // printf("active_pins=%8x\n", count);
    pthread_mutex_lock(&this->table_mutex);
    this->info.bytes = samples;
    this->info.events = num_of_events;
    this->info.pins = count;
    // printf("bytes=%d events=%ld pins=%d\n", samples, num_of_events, count);
    pthread_mutex_unlock(&this->table_mutex);
  }
  printf("Total captured: %d\n", samples);
}
