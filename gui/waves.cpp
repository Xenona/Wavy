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
bool is_x_state(std::string &v) { return v == "x" || v == "X"; }
bool is_z_state(std::string &v) { return v == "z" || v == "Z"; }
bool is_partial_state(std::string &v) {
  for (int i = 0; i < v.size(); i++) {
    if (v[i] != '0' && v[i] != '1') {
      return true;
    }
  }
  return false;
}

auto color(std::string &v) {
  if (is_x_state(v)) {
    return Qt::red;

  } else if (is_z_state(v)) {
    return Qt::blue;

  } else if (is_partial_state(v)) {
    return Qt::yellow;

  } else {
    return Qt::green;
  }
}

auto letter(std::string &v) {
  std::string s = "x";
  if (is_x_state(v)) {
    return s;
  }
  if (is_z_state(v)) {
    return std::string{"z"};
  }
  if (is_partial_state(v)) {

    return std::string{"?"};
  }
  return std::string{"?"};
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
    std::string otherPrevString;
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
      double t = timepoint.time;

      double scenePos = ((t - a) / (double)(b - a)) * width;
      painter->drawLine(QPointF{scenePos, 0}, QPointF{scenePos, (double)this->top->size().height()});
      painter->drawText(scenePos, 20, QString::fromStdString(std::to_string(std::floor(t))));
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
              otherPrevString = dump.vecs[idx].valueVec;
              inited = true;
              if (t == 0) {

                path.moveTo(0, yHalf);
                path.lineTo(HEXAGONS_STEP, y);
                auxPath.moveTo(0, yHalf);
                auxPath.lineTo(HEXAGONS_STEP, yLow);
              } else {

                path.moveTo(0, yHalf);
                path.lineTo(HEXAGONS_STEP, y);
                path.lineTo(scenePos - HEXAGONS_STEP, y);
                path.lineTo(scenePos, yHalf);

                // path.lineTo(scenePos + HEXAGONS_STEP, y, c);

                auxPath.moveTo(0, yHalf);
                auxPath.lineTo(HEXAGONS_STEP, yLow);
                auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow);
                auxPath.lineTo(scenePos, yHalf);
 
                std::string s = letter(dump.vecs[idx].valueVec);

                this->addText(scenePos, prevScenePos, painter, lineHeight,
                              WAVES_GAP, prev, prevFloat, dump, idx, y, otherPrevString, i);
                prevScenePos = scenePos;
              }
            } else {
              if (((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                   (prev != dump.vecs[idx].valueVecDec)) &&
                  (otherPrevString != dump.vecs[idx].valueVec)) {

                path.lineTo(prevScenePos + HEXAGONS_STEP, y);
                path.lineTo(scenePos - HEXAGONS_STEP, y);
                path.lineTo(scenePos, yHalf);

                auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow);
                auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow);
                auxPath.lineTo(scenePos, yHalf);

                std::string s = letter(prevString);
                this->addText(scenePos, prevScenePos, painter, lineHeight,
                              WAVES_GAP, prev, prevFloat, dump, idx, y, otherPrevString, i);

                prevScenePos = scenePos;
                prev = dump.vecs[idx].valueVecDec;
                prevFloat = dump.vecs[idx].valueVecDecFloat;
                otherPrevString = dump.vecs[idx].valueVec;
              }
            }

            double availLen = scenePos - prevScenePos;

          } else {
            if (((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                 (prev != dump.vecs[idx].valueVecDec)) &&
                (otherPrevString != dump.vecs[idx].valueVec)) {

              // c = color(prevString);
              path.lineTo(prevScenePos + HEXAGONS_STEP, y);
              path.lineTo(scenePos - HEXAGONS_STEP, y);
              path.lineTo(scenePos, yHalf);

              auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow);
              auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow);
              auxPath.lineTo(scenePos, yHalf);

              double availLen = scenePos - prevScenePos;

              // QTextItem

              std::string s = letter(prevString);
              this->addText(scenePos, prevScenePos, painter, lineHeight,
                            WAVES_GAP, prev, prevFloat, dump, idx, y, otherPrevString, i);

              prevScenePos = scenePos;
              prev = dump.vecs[idx].valueVecDec;
              prevFloat = dump.vecs[idx].valueVecDecFloat;
              otherPrevString = dump.vecs[idx].valueVec;

            } else {
              // path.lineTo(scenePos);
              // path.lineTo(scenePos, y);
              // auxPath.lineTo(scenePos, yLow);
            }
          }

        } else {
          idx = this->isScalar(dump, this->top->vars[i].identifier);
          if (idx != -1) {
            // draw an angle depending on state
            if (prevTime)
              path.lineTo(scenePos, yPrev);
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
            }
            path.lineTo(scenePos, yNew);
            yPrev = yNew;

            // todo add x and z
            prev = dump.scals[idx].value;
            prevString = dump.scals[idx].stringValue;
            otherPrevString = dump.scals[idx].stringValue;

          } else {
          }
        }
      } else if (scenePos > width) {
        break;
      } else {

        if (isVector && idx != -1) {
          prev = dump.vecs[idx].valueVecDec;
          prevFloat = dump.vecs[idx].valueVecDecFloat;
          otherPrevString = dump.vecs[idx].valueVec;

        } else {
          idx = this->isScalar(dump, this->top->vars[i].identifier);

          if (idx != -1) {
            prev = dump.scals[idx].value;
            prevString = dump.scals[idx].stringValue;
            otherPrevString = dump.scals[idx].stringValue;

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
      // prevScenePos = scenePos;
    }
    if (!isVector) {

      if (!inited && b <= this->top->data->timepoints[0].time) {
        path.moveTo(0, yHalf);
      }

      path.lineTo(width, yPrev);
    } else {

      path.lineTo(prevScenePos + HEXAGONS_STEP, y);
      path.lineTo(width, y);
      auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow);
      auxPath.lineTo(width, yLow);
      std::string s = letter(otherPrevString);

      if (!inited) {

        this->addText(width, prevScenePos, painter, lineHeight, WAVES_GAP, prev,
                      prevFloat, {}, -1, y, otherPrevString, i);
      } else {
        this->addText(width, prevScenePos, painter, lineHeight, WAVES_GAP, prev,
                      prevFloat, {}, -1, y, otherPrevString, i);
      }
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

  QString val;
  if (prevString == "") {

    val = idx != -1
              ? QString::fromStdString(
                    prevString == ""
                        ? (dump.vecs[idx].valueVecDec == 0 ? valFloat : valInt)
                        : prevString)
              : QString::fromStdString(prev == 0 ? valFloat : valInt);
  } else {
    val = QString::fromStdString(prevString);
  }
  if (prevFloat >= 922337203685477580 || prevFloat <= -922337203685477580) {
    val = "x";
  }
  double valWidth = fm.horizontalAdvance(val);
  QRectF rect = {prevScenePos, y, availLen, lineHeight - WAVES_GAP};

  if (col == Qt::transparent) {
    // todo
    // painter->setPen(this->top->waveStates[index].color);
  } else {
    painter->setPen(col);
  }

  if (valWidth <= availLen) {
    painter->drawText(rect, Qt::AlignCenter, val);
  } else {
    painter->drawText(rect, Qt::AlignCenter, ".");
  }
  painter->restore();
}

QRectF Waves::boundingRect() const { return QRectF(0, 0, 10, 10); };
