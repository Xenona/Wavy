#include "waves.h"
#include <QPainter>
#include <QPainterPath>
#include <climits>
#include <cmath>
#include <qgraphicsitem.h>
#include <qlogging.h>
#include <qpainterpath.h>

// todo
// get all destructors into work

Waves::Waves(VCDGraphicsView *top)
    : top(top) {

      };

void Waves::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget) {

  this->top->setUpdatesEnabled(false);

  const int WAVES_GAP = 5;

  int y = WAVES_GAP;
  int a = this->top->top->leftFOVborder;
  int b = this->top->top->rightFOVborder;
  int width = this->top->size().width();
  int lineHeight = this->top->itemRect.height();

  // iterating over variables starting from visible ones (scrollHeight idx)
  for (int i = this->top->scrollHeight; i < this->top->vars.length(); i++) {

    QPainterPath path;
    path.moveTo(0, y);

    float prevFloat = 0;
    long long prev = 0;
    bool inited = false;

    float yLow = y + lineHeight - WAVES_GAP;
    float yHalf = yLow/2;

    // iterating over time points (events)
    for (auto &timepoint : this->top->data->timepoints) {

      // if j is existing time event

      DumpData dump = timepoint.data;
      int t = timepoint.time;

      float scenePos = ((t - a) / (float)(b - a)) * width;
      if (scenePos >= 0 && scenePos <= width) {

        int idx = this->isScalar(dump, this->top->vars[i].identifier);
        if (idx != -1) {
          if (!inited) {
            prev = dump.scals[idx].value;
            inited = true;
            // start low if the first bit is 0
            if (prev == 0) {
              path.moveTo(0, yLow);
            }
          } else {

            // draw an angle depending on state
            if (prev == 0 && dump.scals[idx].value == 1) {
              path.lineTo(scenePos, yLow);
              path.lineTo(scenePos, y);
            }
            if (prev == 1 && dump.scals[idx].value == 0) {
              path.lineTo(scenePos, y);
              path.lineTo(scenePos, yLow);
            }
            // todo add x and z
            prev = dump.scals[idx].value;
          }
        } else {
          idx = this->isVector(dump, this->top->vars[i].identifier);
          // todo
          // vector drawing
          if (!inited) {
            if (dump.vecs[idx].type == 'b' || dump.vecs[idx].type == 'B') {
              prev = dump.vecs[idx].valueVecDec;
              inited = true;
              path.moveTo(0, yHalf);
            } else { 

            }
          }
        }
      };
      // qDebug() << "INFO" << j << a << b << width.width();
    }
    // when all the points are done, closing the line
    if (prev == 1) {
      path.lineTo(width, y);
    } else if (prev == 0) {
      path.lineTo(width, yLow);
    }
    painter->drawPath(path);
    y += lineHeight;
  }
  this->top->setUpdatesEnabled(true);
};

int Waves::isScalar(DumpData data, std::string id) {
  for (int i = 0; i < data.scals.size(); i++) {
    auto &d = data.scals[i];
    if (d.identifier == id) {
      return i;
    }
  }
  return -1;
}

int Waves::isVector(DumpData data, std::string id) {
  for (int i = 0; i < data.vecs.size(); i++) {
    auto &d = data.scals[i];
    if (d.identifier == id) {
      return i;
    }
  }
  return -1;
}


QRectF Waves::boundingRect() const { return QRectF(0, 0, 10, 10); };
