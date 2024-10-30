#pragma once

#include "vcd-data.h"
#include "vcd-token-stream.h"
#include <string>

int test(int a);

enum class ParserState {
  Header = 0,
  Date,
  Version,
  Comment,
  Timescale,
  Scope,
  ScopeVar,
  EndDefinitions,
  Dumps,
  Data,
};

class VCDParser {

public:
  VCDData *getVCDData(VCDTokenStream *tokenStream);
  void dbg(VCDTokenStream *s);

private:
  void error(std::string message, VCDData *vcd);
  void warn(std::string message, VCDData *vcd);
  void reset();

  VCDTokenStream *tokenStream = nullptr;
  ParserState state = ParserState::Header;
};
