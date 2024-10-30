#include "wavy-frontend.h"
#include "ui_mainwindow.h"
#include <string>

WavyFrontend::WavyFrontend(Ui::MainWindow *ui) {
  this->ui = ui;
  this->selected_dumps_list = ui->selected_dumps_list;
  this->waveform_tabs = ui->waveform_tabs;
  this->sidebar = ui->sidebar;
  this->sidebar_objects_button_close = ui->sidebar_objects_button_close;
  this->sidebar_scope_button_close = ui->sidebar_scope_button_close;
  this->objects_list = ui->objects_list;
  this->dumps_list = ui->dumps_list;
  this->scope_tree = ui->scope_tree;
  this->waveform_scroll = ui->waveform_scroll;
  this->action_open = ui->action_open;
  this->action_save = ui->action_save;
}

void loadVCDData(std::string path) {
  auto vcdTokenStream = new VCDTokenStream(new VCDCharStream(path))
}


