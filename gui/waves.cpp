#include "waves.h"
#include "path.h"
#include <QPainter>
#include <QPainterPath>
#include <bitset>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <qfontmetrics.h>
#include <qgraphicsitem.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qpainterpath.h>
#include <qpoint.h>
#include <qrgb.h>
#include <sstream>
#include <string>

bool Waves::is_half_state(std::string &v) {
  return v == "x" || v == "X" || v == "z" || v == "Z";
}
bool Waves::is_x_state(std::string &v) {
  for (int i = 0; i < v.size(); i++) {
    if (v[i] != 'x' && v[i] != 'X') {
      return false;
    }
  }
  return true;
}
bool Waves::is_z_state(std::string &v) {
  for (int i = 0; i < v.size(); i++) {
    if (v[i] != 'z' && v[i] != 'Z') {
      return false;
    }
  }
  return true;
}
bool Waves::is_partial_state(std::string &v) {
  for (int i = 0; i < v.size(); i++) {
    if (v[i] != '0' && v[i] != '1') {
      return true;
    }
  }
  return false;
}

auto Waves::color(std::string &v, int i) {
  if (is_x_state(v)) {
    return Qt::red;

  } else if (is_z_state(v)) {
    return Qt::blue;

  } else if (is_partial_state(v)) {
    return Qt::yellow;

  } else {
    return this->top->waveStates[i].color;
  }
}

std::string toOctalString(unsigned long long value) {
  std::stringstream ss;
  ss << std::oct << value;
  return ss.str();
}

std::string toHexString(unsigned long long value) {
  std::stringstream ss;
  ss << std::hex << std::uppercase << value;
  return ss.str();
}

std::string toDecimalString(unsigned long long value) {
  return std::to_string(value);
}

auto Waves::get_text(std::string &v, int i) {
  auto setting = this->top->waveStates[i].base;
  if (setting == base::bin) {
    return "0b"+v;
  }
  if (is_x_state(v))
    return v;
  if (is_z_state(v))
    return v;
  if (is_partial_state(v))
    return std::string{"?"};

  unsigned long long decimalValue = std::bitset<64>(v).to_ullong();

  if (setting == base::oct) {
    return "0o"+toOctalString(decimalValue);
  }
  if (setting == base::dec) {
    return toDecimalString(decimalValue);
  }

  if (setting == base::hex) {
    return "0x"+toHexString(decimalValue);
  }

  
  return std::string{"E"};
}

auto Waves::letter(std::string &v) {
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

  double lineHeight = this->top->itemRect.height();
  double y = WAVES_GAP * 2 + lineHeight;
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
  if (this->top->list.size()) {

    int amount =
        std::max(1, static_cast<int>((float(this->top->top->maxrightborder -
                                            this->top->top->minleftborder) /
                                      float(b - a)) *
                                     3.0));
    double distance = double(b - a) / amount;

    int numTicks;
    if (b - a > 100) {
      numTicks = 10;
    } else if (b - a > 50) {
      numTicks = 5;
    } else {
      numTicks = 2;
    }

    double tickDistance = double(b - a) / numTicks;

    painter->save();
    QFont font = painter->font();
    font.setPixelSize(std::abs(lineHeight - WAVES_GAP * 4));
    painter->setFont(font);
    QFontMetricsF fm(font);
    for (double i = a; i <= b; i += tickDistance) {
      double scenePos = ((i - a) / (double)(b - a)) * width;

      painter->drawLine(QPointF{scenePos, lineHeight},
                        QPointF{scenePos, (double)this->top->size().height()});

      QString label = QString::number(i, 'i', (b - a > 100) ? 0 : 2);

      double valWidth = fm.horizontalAdvance(label);

      QRectF rect = {scenePos - valWidth / 2, 0, valWidth, lineHeight};
      painter->setPen(QColor(136, 139, 143));
      painter->drawText(rect, Qt::AlignCenter, label);
    }
    painter->restore();
  }

  for (int i = this->top->scrollHeight; i < this->top->vars.length(); i++) {

    Path path = Path(this->top->waveStates[i].color);
    Path auxPath = Path(this->top->waveStates[i].color);

    double prevScenePos = 0;
    double otherPrevScenePos = 0;

    double prevFloat = INFINITY;
    long long prev = 0;
    std::string prevString;
    std::string otherPrevString;
    std::string otherPrevPrevString;
    bool inited = false;
    bool scalInited = false;

    int prevTime = 0;
    bool isVector = false;

    double yLow = y + lineHeight - WAVES_GAP;
    double yHalf = y + (yLow - y) / 2;
    double yPrev = yHalf;
    path.moveTo(0, y);
    auxPath.moveTo(0, yLow);

    // iterating over time points (events)
    for (auto &timepoint : this->top->data->timepoints) {

      DumpData dump = timepoint.data;
      double t = timepoint.time;

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
              otherPrevPrevString = otherPrevString;
              otherPrevString = dump.vecs[idx].valueVec;
              auto c = color(otherPrevString, i);
              inited = true;
              if (t == 0) {
                if (is_x_state(dump.vecs[idx].valueVec)) {

                  c = Qt::red;
                }

                path.moveTo(0, yHalf);
                path.lineTo(HEXAGONS_STEP, y, c);
                auxPath.moveTo(0, yHalf);
                auxPath.lineTo(HEXAGONS_STEP, yLow, c);
              } else {

                c = Qt::red;
                if (a == 0) {
                  path.moveTo(0, yHalf);
                  auxPath.moveTo(0, yHalf);

                  path.lineTo(HEXAGONS_STEP, y, c);
                  auxPath.lineTo(HEXAGONS_STEP, yLow, c);
                  path.lineTo(scenePos - HEXAGONS_STEP, y, c);
                  auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, c);

                  if ((otherPrevPrevString != otherPrevString) &&
                      !(otherPrevString == "x")) {
                    path.lineTo(scenePos, yHalf, c);
                    auxPath.lineTo(scenePos, yHalf, c);
                    this->addText(scenePos, prevScenePos, painter, lineHeight,
                                  WAVES_GAP, prev, prevFloat, dump, idx, y, "x",
                                  i, Qt::red);
                  }
                  auto c = color(otherPrevString, i);

                  path.lineTo(scenePos + HEXAGONS_STEP, y, c);
                  auxPath.lineTo(scenePos + HEXAGONS_STEP, yLow, c);

                  if ((otherPrevPrevString != otherPrevString) &&
                      !(otherPrevString == "x")) {
                    prevScenePos = scenePos;
                  }
                } else {

                  path.moveTo(0, y);
                  auxPath.moveTo(0, yLow);

                  path.lineTo(scenePos - HEXAGONS_STEP, y, c);
                  auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, c);
                  if ((otherPrevPrevString != otherPrevString) &&
                      otherPrevString != "x") {

                    path.lineTo(scenePos, yHalf, c);
                    auxPath.lineTo(scenePos, yHalf, c);
                    this->addText(scenePos, prevScenePos, painter, lineHeight,
                                  WAVES_GAP, prev, prevFloat, dump, idx, y, "x",
                                  i, Qt::red);
                    prevScenePos = scenePos;
                  }
                }

                // path.lineTo(scenePos + HEXAGONS_STEP, y, c);

                if ((otherPrevString != "") &&
                    (otherPrevString != dump.vecs[idx].valueVec)) {
                  path.lineTo(scenePos, yHalf, c);
                  auxPath.lineTo(scenePos, yHalf, c);
                  std::string s = get_text(otherPrevString, i);

                  this->addText(scenePos, prevScenePos, painter, lineHeight,
                                WAVES_GAP, prev, prevFloat, dump, idx, y, s, i,
                                c);
                  prevScenePos = scenePos;
                }
              }
              // otherPrevPrevString = otherPrevString;
              // otherPrevString = dump.vecs[idx].valueVec;

            } else {
              auto c = color(otherPrevString, i);

              if (((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                   (prev != dump.vecs[idx].valueVecDec)) ||
                  (otherPrevString != dump.vecs[idx].valueVec)) {

                if (a != t) {
                  path.lineTo(prevScenePos + HEXAGONS_STEP, y, c);
                  auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow, c);
                }
                path.lineTo(scenePos - HEXAGONS_STEP, y, c);
                path.lineTo(scenePos, yHalf, c);

                auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, c);
                auxPath.lineTo(scenePos, yHalf, c);

                std::string s = get_text(otherPrevString, i);
                this->addText(scenePos, prevScenePos, painter, lineHeight,
                              WAVES_GAP, prev, prevFloat, dump, idx, y, s, i,
                              c);

                prevScenePos = scenePos;
                prev = dump.vecs[idx].valueVecDec;
                prevFloat = dump.vecs[idx].valueVecDecFloat;
                otherPrevPrevString = otherPrevString;

                otherPrevString = dump.vecs[idx].valueVec;
              }
            }

            double availLen = scenePos - prevScenePos;

          } else {
            if (((prevFloat != dump.vecs[idx].valueVecDecFloat) ||
                 (prev != dump.vecs[idx].valueVecDec)) ||
                (otherPrevString != dump.vecs[idx].valueVec)) {
              auto c = color(otherPrevString, i);

              // c = color(prevString);
              path.lineTo(prevScenePos + HEXAGONS_STEP, y, c);
              path.lineTo(scenePos - HEXAGONS_STEP, y, c);
              path.lineTo(scenePos, yHalf, c);

              auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow, c);
              auxPath.lineTo(scenePos - HEXAGONS_STEP, yLow, c);
              auxPath.lineTo(scenePos, yHalf, c);

              double availLen = scenePos - prevScenePos;

              // QTextItem

              std::string s = get_text(otherPrevString, i);
              this->addText(scenePos, prevScenePos, painter, lineHeight,
                            WAVES_GAP, prev, prevFloat, dump, idx, y, s, i, c);

              prevScenePos = scenePos;
              prev = dump.vecs[idx].valueVecDec;
              prevFloat = dump.vecs[idx].valueVecDecFloat;
              otherPrevPrevString = otherPrevString;

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
            if (!scalInited) {
              path.moveTo(0, yLow);
              yPrev = yLow;
            }
            scalInited = true;
            // draw an angle depending on state
            if (prevTime)
              path.lineTo(scenePos, yPrev, color(prevString, i));
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
            path.lineTo(scenePos, yNew, color(dump.scals[idx].stringValue, i));
            yPrev = yNew;

            // todo add x and z
            prev = dump.scals[idx].value;
            prevString = dump.scals[idx].stringValue;
            otherPrevPrevString = otherPrevString;

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
          otherPrevPrevString = otherPrevString;

          otherPrevString = dump.vecs[idx].valueVec;

        } else {
          idx = this->isScalar(dump, this->top->vars[i].identifier);

          if (idx != -1) {
              if (!scalInited) {
              path.moveTo(0, yLow);
            }
            scalInited = true;
            prev = dump.scals[idx].value;
            prevString = dump.scals[idx].stringValue;
            otherPrevPrevString = otherPrevString;

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
      if (isVector && idx != -1) {
        // otherPrevPrevString = otherPrevString;

        otherPrevString = dump.vecs[idx].valueVec;
      }
      prevTime = t;
      otherPrevScenePos = scenePos;
    }
    if (!isVector) {

      if (!inited && b <= this->top->data->timepoints[0].time) {
        path.moveTo(0, yHalf);
        std::string x = "x";
        path.lineTo(width, yPrev, color(x, i));

      } else {
        if (!scalInited) {
          path.moveTo(0, yHalf);
          path.lineTo(width, yPrev, Qt::red);

        } else {

          path.lineTo(width, yPrev, color(otherPrevString, i));
        }
      }

    } else {
      auto c = color(otherPrevString, i);

      if (prevFloat == INFINITY) {
        c = Qt::red;
      }

      if (!a) {
        path.moveTo(prevScenePos, yHalf);
        path.lineTo(prevScenePos + HEXAGONS_STEP, y, c);
        path.lineTo(width, y, c);
        auxPath.moveTo(prevScenePos, yHalf);
        auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow, c);
        auxPath.lineTo(width, yLow, c);
      } else {
        if (otherPrevPrevString != otherPrevString) {

          path.lineTo(prevScenePos + HEXAGONS_STEP, y, c);
          auxPath.lineTo(prevScenePos + HEXAGONS_STEP, yLow, c);
        }
        path.lineTo(width, y, c);
        auxPath.lineTo(width, yLow, c);
      }
      std::string s = get_text(otherPrevString, i);

      if (!inited) {

        this->addText(width, prevScenePos, painter, lineHeight, WAVES_GAP, prev,
                      prevFloat, {}, -1, y, s, i, c);
      } else {
        this->addText(width, prevScenePos, painter, lineHeight, WAVES_GAP, prev,
                      prevFloat, {}, -1, y, s, i, c);
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
  font.setPixelSize((lineHeight - WAVES_GAP * 3));

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
    col = Qt::red;
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
