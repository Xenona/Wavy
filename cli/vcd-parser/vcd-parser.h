#pragma once

#include "vcd-token-stream.h"
#include <string>
#include "vcd-data.h"

int test(int a);

enum ParserState {
  READING_HEADER,
  READING_DATE,
  READING_VERSION,
  READING_COMMENT,
  READING_TIMESCALE,
  READING_SCOPE,
  READING_SCOPE_VAR,
  READING_END_DEFINITIONS,

  READING_DUMPS,
  READING_DATA,


};

class VCDParser {

public:
 

  VCD::VCDData* getVCDData(VCDTokenStream* tokenStream);
  void dbg(VCDTokenStream* s);
  void error(std::string message);
  void warn(std::string message);

private:
  VCDTokenStream* tokenStream;
  VCD::VCDData* vcd;
  ParserState state = ParserState::READING_HEADER;

};
