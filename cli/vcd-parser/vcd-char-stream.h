#pragma once

#include <fstream>
#include <string>

class VCDCharStream {

public:
  VCDCharStream(std::string filepath);
  ~VCDCharStream();

  char next();
  char peek();
  bool eof();
  void die(std::string message);
  void logState();

private:

  int column = 0;
  int line = 1;

  std::fstream input;
};
