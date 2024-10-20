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
  void error(std::string message);
  void warn(std::string message);

private:
  VCDTokenStream* tokenStream;
  VCDData* vcd;
  ParserState state = ParserState::Header;

};
