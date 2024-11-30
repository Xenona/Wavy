#pragma once
#include "../../../lib/vcd-parser/vcd-data.h"
#include <QWidget>
#include <qboxlayout.h>
#include <qtablewidget.h>


class InfoTableWidget : public QTableWidget {
public:
  InfoTableWidget(VCDData *data);
};
