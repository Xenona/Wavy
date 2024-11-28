#include "waves.h"
#include <QPainter>
#include <QPainterPath>
#include <climits>
#include <cmath>
#include <qfontmetrics.h>
#include <qgraphicsitem.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qpainterpath.h>
#include <qpoint.h>
#include <string>

// todo
// get all destructors into work

Waves::Waves(VCDGraphicsView *top)
    : top(top) {

      };

void Waves::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget) {

  this->top->setUpdatesEnabled(false);

  const int WAVES_GAP = 5;
  int HEXAGONS_STEP;

  float y = WAVES_GAP;
  int a = this->top->top->leftFOVborder;
  int b = this->top->top->rightFOVborder - 100;
  if (this->top->data->timepoints.size())
    HEXAGONS_STEP = ((this->top->data->timepoints.back().time -
                      this->top->data->timepoints[0].time) /
                     (b - a)) %
                    10;
  else {
    HEXAGONS_STEP = 1;
  }
  int width = this->top->size().width();
  float lineHeight = this->top->itemRect.height();

  // iterating over variables starting from visible ones (scrollHeight idx)
  for (int i = this->top->scrollHeight; i < this->top->vars.length(); i++) {

    QPainterPath path;
    QPainterPath auxPath;
    path.moveTo(0, y);

    float prevScenePos = 0;

    float prevFloat = 0;
    long long prev = 0;
    bool inited = false;

    bool isVector = false;

    float yLow = y + lineHeight - WAVES_GAP;
    float yHalf = y + (yLow - y) / 2;

    // iterating over time points (events)
    for (auto &timepoint : this->top->data->timepoints) {

      DumpData dump = timepoint.data;
      int t = timepoint.time;

      float scenePos = ((t - a) / (float)(b - a)) * width;
      if (scenePos >= 0 && scenePos <= width) {

        int idx = this->isVector(dump, this->top->vars[i].identifier);
        qDebug() << idx;
        if (idx != -1) {
          idx = this->isVector(dump, this->top->vars[i].identifier);
          // todo
          // vector drawing
          isVector = true;
          if (!inited) {
            prevFloat = dump.vecs[idx].valueVecDecFloat;
            prev = dump.vecs[idx].valueVecDec;
            inited = true;
            path.moveTo(0, yHalf);
            path.lineTo(HEXAGONS_STEP, y);
            auxPath.moveTo(0, yHalf);
            auxPath.lineTo(HEXAGONS_STEP, yLow);

            float availLen = scenePos - prevScenePos;

            prevScenePos = scenePos;

          } else {
            if ((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                (prev != dump.vecs[idx].valueVecDec)) {
              path.lineTo(scenePos - HEXAGONS_STEP, y);
              path.lineTo(scenePos, yHalf);
              path.lineTo(scenePos + HEXAGONS_STEP, y);

              auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow);
              auxPath.lineTo(scenePos, yHalf);
              auxPath.lineTo(scenePos + HEXAGONS_STEP, yLow);

              float availLen = scenePos - prevScenePos;

              // QTextItem
              painter->save();
              QFont font = painter->font();
              font.setPixelSize(lineHeight - WAVES_GAP * 3);
              painter->setFont(font);
              QFontMetricsF fm(font);
              std::string valInt = std::to_string(prev);
              std::string valFloat = std::to_string(prevFloat);

              QString val = QString::fromStdString(
                  dump.vecs[idx].valueVecDec == 0 ? valFloat : valInt);
              float valWidth = fm.horizontalAdvance(val);
              QRectF rect = {prevScenePos, y, availLen,
                              lineHeight - WAVES_GAP};
              if (valWidth <= availLen) {
                painter->drawText(rect, Qt::AlignCenter, val);
              } else {
                painter->drawText(rect, Qt::AlignCenter, ".");

              }
              painter->restore();

              prevScenePos = scenePos;
              prev = dump.vecs[idx].valueVecDec;
              prevFloat = dump.vecs[idx].valueVecDecFloat;

            } else {
              // path.lineTo(scenePos);
              path.lineTo(scenePos - HEXAGONS_STEP, y);
              auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow);
            }
          }

        } else {
          idx = this->isScalar(dump, this->top->vars[i].identifier);
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
            // if (prev == 1) {
            //   path.lineTo(scenePos, y);
            // } else {
            //   path.lineTo(scenePos, yLow);
            // }
          }
        }
      };
      // qDebug() << "INFO" << j << a << b << width.width();
    }
    // when all the points are done, closing the line
    if (!isVector) {

      if (prev == 1) {
        path.lineTo(width, y);
      } else if (prev == 0) {
        path.lineTo(width, yLow);
      }
    } else {
      path.lineTo(width, y);
      auxPath.lineTo(width, yLow);
    }
    painter->drawPath(path);
    painter->drawPath(auxPath);
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
    auto &d = data.vecs[i];
    if (d.identifier == id) {
      return i;
    }
  }
  return -1;
}

QRectF Waves::boundingRect() const { return QRectF(0, 0, 10, 10); };
