#pragma once

#include "vcd-token-stream.h"
#include <string>
#include "vcd-data.h"

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
 
  VCDData* getVCDData(VCDTokenStream* tokenStream);
  void dbg(VCDTokenStream* s);

private:

  void error(std::string message);
  void warn(std::string message);

  VCDTokenStream* tokenStream;
  VCDData* vcd;
  ParserState state;
};
