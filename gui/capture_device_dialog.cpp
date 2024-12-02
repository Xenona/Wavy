#include "capture_device_dialog.h"
#include "../pikernely/proto.h"
#include "ui_capture_device_dialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWidget>
#include <chrono>
#include <fcntl.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <string>
#include <thread>
#include <unistd.h>

CaprureDeviceDialog::CaprureDeviceDialog(int fd, QWidget *parent)
    : QDialog(parent), ui(new Ui::CaprureDeviceDialog), fd(fd) {
  ui->setupUi(this); // This will init all the widgets
  ui->buttonBox->clear();
  auto buttonCancel = new QPushButton("Cancel");
  ui->buttonBox->addButton(buttonCancel, QDialogButtonBox::RejectRole);
  auto buttonStop = new QPushButton("Stop");
  ui->buttonBox->addButton(buttonStop, QDialogButtonBox::ActionRole);

  // auto buttonPause = new QPushButton("Pause");
  // ui->buttonBox->addButton(buttonPause, QDialogButtonBox::ActionRole);
  auto buttonStart = new QPushButton("Start");
  ui->buttonBox->addButton(buttonStart, QDialogButtonBox::ActionRole);
  auto buttonDone = new QPushButton("Done");
  ui->buttonBox->addButton(buttonDone, QDialogButtonBox::AcceptRole);

  QObject::connect(buttonStart, &QPushButton::clicked, this, [this]() {
    this->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->prepare();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pthread_create(&pt, NULL, &CaprureDeviceDialog::readAllTrampoline, this);
    std::this_thread::sleep_for(std::chrono::milliseconds(0));
    this->start();
  });

  QObject::connect(buttonStop, &QPushButton::clicked, this, [this]() {
    this->stop();
    pthread_join(pt, NULL);
    printf("Kernel read all\n");
  });
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

}

void CaprureDeviceDialog::start() {

  struct KERNELY_PKT c;
  c.state_flags = KERNELY_ENABLE;
  c.timer_period = 5;
  write(fd, &c, sizeof(c));
  printf("Kernel start\n");
}

void CaprureDeviceDialog::stop() {
  struct KERNELY_PKT c;
  c.state_flags = KERNELY_FINISH;
  c.timer_period = 0;
  write(fd, &c, sizeof(c));
  printf("Kernel finish\n");
}

CaprureDeviceDialog::~CaprureDeviceDialog() {

};

void CaprureDeviceDialog::readAll(CaprureDeviceDialog *cdd) {
  uint16_t seq = 0;
  bool last = false;
  uint8_t prev = 0;
  uint64_t prev_time = 0;
  int samples = 0;

  uint64_t first_time = 0;

  while (!last) {
    struct PICOY_PKT pkt;

    int a = read(fd, &pkt, sizeof(pkt));
    if (a <= 0) {
      printf("%d", a);
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

        for (int k = 0; k < 7; k++) {
          if (((prev >> k) & 1) != ((pkt.data[i] >> k) & 1)) {
            // printf("REad hhheee\n");
            data.scals.push_back({.value = (pkt.data[i] >> k) & 1,
                                  .identifier = std::to_string(k + 1)});
          }
        }

        this->data->timepoints.push_back(
            {.time = (int)(time - first_time), .data = data});
        printf("[%ldus](+ %ldus) =%x\n", time, time - prev_time, pkt.data[i]);
        prev_time = time;
        prev = pkt.data[i];
      }
    }
    // printf("Recieved packet %d started=%ld duration=%d last=%s\n",
    // pkt.packet_id,
    //        pkt.time_start, pkt.time_duration, last ? "y" : "n");
  }
  printf("Toal captured: %d\n", samples);
}
