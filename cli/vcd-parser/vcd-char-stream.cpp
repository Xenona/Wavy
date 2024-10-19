#include "vcd-char-stream.h"
#include <QtDebug>
#include <QtLogging>
#include <ios>

VCDCharStream::VCDCharStream(std::string filepath) {
  this->input.open(filepath);
  this->input >> std::noskipws;

  if (!this->input) {
    qFatal() << "Could not open a file";
    return;
  }
}

VCDCharStream::~VCDCharStream() { this->input.close(); }

char VCDCharStream::next() {
  char c;
  this->input >> c;
  if (c == '\n') {
    this->line++;
    this->column = 0;
  } else {
    this->column++;
  }

  return c;
}

char VCDCharStream::peek() { return this->input.peek(); }

bool VCDCharStream::eof() {
  int c = this->peek();
  if (c == EOF) {
    if (this->input.eof()) {
        return true;
    }
    else {
        qFatal() << "Error reading the end of file (?)";
        return false;
    }
  } else {
    return false;
  }
}

void VCDCharStream::die(std::string message) {
    qFatal() << message << "(" << this->line << ":" << this->column << ")";
}

void VCDCharStream::logState() {
    qDebug() << "Line:" << this->line << "Column" << this->column;
}
