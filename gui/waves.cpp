#include "waves.h"
#include "path.h"
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

bool is_half_state(std::string &v) {
  return v == "x" || v == "X" || v == "z" || v == "Z";
}

Waves::Waves(VCDGraphicsView *top)
    : top(top) {

      };

void Waves::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget) {

  this->top->setUpdatesEnabled(false);

  const int WAVES_GAP = 5;
  int HEXAGONS_STEP;

  double y = WAVES_GAP;
  int a = this->top->top->leftFOVborder;
  int b = this->top->top->rightFOVborder;
  if (this->top->data->timepoints.size())
    HEXAGONS_STEP = std::min(((this->top->data->timepoints.back().time -
                               this->top->data->timepoints[0].time) /
                              (b - a)),
                             10);
  else {
    HEXAGONS_STEP = 1;
  }
  int width = this->top->size().width();
  double lineHeight = this->top->itemRect.height();

  // iterating over variables starting from visible ones (scrollHeight idx)
  for (int i = this->top->scrollHeight; i < this->top->vars.length(); i++) {

    Path path = Path(this->top->waveStates[i].color);
    Path auxPath = Path(this->top->waveStates[i].color);

    path.moveTo(0, y);

    double prevScenePos = 0;

    double prevFloat = INFINITY;
    long long prev = 0;
    std::string prevString;
    bool inited = false;
    bool vecInited = false;

    int prevTime = 0;
    bool isVector = false;

    double yLow = y + lineHeight - WAVES_GAP;
    double yHalf = y + (yLow - y) / 2;
    double yPrev = yHalf;
    auxPath.moveTo(0, yLow);

    // iterating over time points (events)
    for (auto &timepoint : this->top->data->timepoints) {

      DumpData dump = timepoint.data;
      int t = timepoint.time;

      double scenePos = ((t - a) / (double)(b - a)) * width;
      int idx = this->isVector(dump, this->top->vars[i].identifier);
      if (idx != -1) {
        isVector = true;
      }
      if (scenePos >= 0 && scenePos <= width) {

        if (idx != -1) {
          // todo
          // vector drawing
          isVector = true;
          if (!inited) {
            if (prevFloat == INFINITY) {
              prevFloat = dump.vecs[idx].valueVecDecFloat;
              prev = dump.vecs[idx].valueVecDec;
              prevString = dump.vecs[idx].valueVec;
              inited = true;
              if (t == 0) {

                path.moveTo(0, yHalf);
                path.lineTo(HEXAGONS_STEP, y, Qt::red);
                auxPath.moveTo(0, yHalf);
                auxPath.lineTo(HEXAGONS_STEP, yLow, Qt::red);
              } else {

                path.moveTo(0, yHalf);
                path.lineTo(HEXAGONS_STEP, y, Qt::red);
                path.lineTo(scenePos - HEXAGONS_STEP, y, Qt::red);
                path.lineTo(scenePos, yHalf, Qt::red);
                path.lineTo(scenePos + HEXAGONS_STEP, y, Qt::red);

                auxPath.moveTo(0, yHalf);
                auxPath.lineTo(HEXAGONS_STEP, yLow, Qt::red);
                auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, Qt::red);
                auxPath.lineTo(scenePos, yHalf, Qt::red);
                auxPath.lineTo(scenePos + HEXAGONS_STEP, yLow, Qt::red);
                this->addText(scenePos, prevScenePos, painter, lineHeight,
                              WAVES_GAP, prev, prevFloat, dump, idx, y, "x", i,
                              Qt::red);
              }
            } else {
              if ((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                  (prev != dump.vecs[idx].valueVecDec)) {

                auto col = Qt::green;
                if (prevString == "X" || prevString == "x")
                  col = Qt::red;
                if (prevString == "Z" || prevString == "z")
                  col = Qt::red;
                path.lineTo(scenePos - HEXAGONS_STEP, y, col);
                path.lineTo(scenePos, yHalf, col);
                path.lineTo(scenePos + HEXAGONS_STEP, y, col);

                auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, col);
                auxPath.lineTo(scenePos, yHalf, col);
                auxPath.lineTo(scenePos + HEXAGONS_STEP, yLow, col);

                this->addText(scenePos, prevScenePos, painter, lineHeight,
                              WAVES_GAP, prev, prevFloat, dump, idx, y, "", i,
                              col);

                prevScenePos = scenePos;
                prev = dump.vecs[idx].valueVecDec;
                prevFloat = dump.vecs[idx].valueVecDecFloat;
                prevString = dump.vecs[idx].valueVec;
              }
            }

            double availLen = scenePos - prevScenePos;

            prevScenePos = scenePos;

          } else {
            if ((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                (prev != dump.vecs[idx].valueVecDec)) {
              auto col = Qt::green;
              if (prevString == "X" || prevString == "x")
                col = Qt::red;
              if (prevString == "Z" || prevString == "z")
                col = Qt::red;

              path.lineTo(scenePos - HEXAGONS_STEP, y, col);
              path.lineTo(scenePos, yHalf, col);
              path.lineTo(scenePos + HEXAGONS_STEP, y, col);

              auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, col);
              auxPath.lineTo(scenePos, yHalf, col);
              auxPath.lineTo(scenePos + HEXAGONS_STEP, yLow, col);

              double availLen = scenePos - prevScenePos;

              // QTextItem
              this->addText(scenePos, prevScenePos, painter, lineHeight,
                            WAVES_GAP, prev, prevFloat, dump, idx, y, "", i,
                            col);

              prevScenePos = scenePos;
              prev = dump.vecs[idx].valueVecDec;
              prevFloat = dump.vecs[idx].valueVecDecFloat;
              prevString = dump.vecs[idx].valueVec;

            } else {
              // path.lineTo(scenePos);
              path.lineTo(scenePos - HEXAGONS_STEP, y);
              auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow);
            }
          }

        } else {
          idx = this->isScalar(dump, this->top->vars[i].identifier);
          if (idx != -1) {
            // draw an angle depending on state
            auto col = Qt::magenta;
            if (prevTime) {
              if (prevString == "X" || prevString == "x")
                col = Qt::red;
              if (prevString == "Z" || prevString == "z")
                col = Qt::red;
              path.lineTo(scenePos, yPrev, col);
            }
            double yNew = 0;
            if (is_half_state(dump.scals[idx].stringValue)) {
              yNew = yHalf;
            } else if (dump.scals[idx].value == 0) {
              yNew = yLow;
            } else {
              yNew = y;
            }
            if (!prevTime) {
              path.moveTo(0, yNew);
              qDebug() << prevScenePos << scenePos << yPrev << yNew;
            }
            path.lineTo(scenePos, yNew, col);
            yPrev = yNew;

            // todo add x and z
            prev = dump.scals[idx].value;
            prevString = dump.scals[idx].stringValue;
          } else {
          }
        }
      } else if (scenePos > width) {
        break;
      } else {

        if (isVector && idx != -1) {
          prev = dump.vecs[idx].valueVecDec;
          prevFloat = dump.vecs[idx].valueVecDecFloat;
          prevString = dump.vecs[idx].valueVec;
        } else {
          idx = this->isScalar(dump, this->top->vars[i].identifier);

          if (idx != -1) {
            prev = dump.scals[idx].value;
            prevString = dump.scals[idx].stringValue;

            if (is_half_state(prevString)) {
              path.moveTo(0, yHalf);
              yPrev = yHalf;
            } else if (prev == 0) {
              path.moveTo(0, yLow);
              yPrev = yLow;
            } else {
              path.moveTo(0, y);
              yPrev = y;
            }
          }
        }
      };
      prevTime = t;
    }
    auto col = Qt::green;
    if (prevString == "X" || prevString == "x")
      col = Qt::red;
    if (prevString == "Z" || prevString == "z")
      col = Qt::red;
    if (!isVector) {
      path.lineTo(width, yPrev, col);
    } else {
      path.lineTo(width, y, col);
      auxPath.lineTo(width, yLow, col);
      this->addText(width, prevScenePos, painter, lineHeight, WAVES_GAP, prev,
                    prevFloat, {}, -1, y, "", i);
    }
    path.drawPath(painter);
    auxPath.drawPath(painter);
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

void Waves::addText(double scenePos, double prevScenePos, QPainter *painter,
                    double lineHeight, int WAVES_GAP, long long prev,
                    long long prevFloat, DumpData dump, int idx, double y,
                    std::string prevString, long long index,
                    Qt::GlobalColor col) {
  double availLen = scenePos - prevScenePos;

  // QTextItem
  painter->save();
  QFont font = painter->font();
  font.setPixelSize(lineHeight - WAVES_GAP * 3);

  painter->setFont(font);
  QFontMetricsF fm(font);
  std::string valInt = std::to_string(prev);
  std::string valFloat = std::to_string(prevFloat);

  QString val =
      idx != -1
          ? QString::fromStdString(
                prevString == ""
                    ? (dump.vecs[idx].valueVecDec == 0 ? valFloat : valInt)
                    : prevString)
          : QString::fromStdString(prev == 0 ? valFloat : valInt);
  double valWidth = fm.horizontalAdvance(val);
  QRectF rect = {prevScenePos, y, availLen, lineHeight - WAVES_GAP};

  if (col == Qt::transparent) {
    // todo
    // painter->setPen(this->top->waveStates[index].color);
  } else {
    // painter->setPen(col);
  }

  if (valWidth <= availLen) {
    painter->drawText(rect, Qt::AlignCenter, val);
  } else {
    painter->drawText(rect, Qt::AlignCenter, ".");
  }
  painter->restore();
}

QRectF Waves::boundingRect() const { return QRectF(0, 0, 10, 10); };
