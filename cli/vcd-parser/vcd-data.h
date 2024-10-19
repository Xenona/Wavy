#pragma once
#include <memory>
#include <string>
#include <vector>

namespace VCD {

struct DateData {
  std::string date;
};

struct VersionData {
  std::string version;
};

struct TimescaleData {
  int precision;
  std::string unit;
};

struct CommentData {
  std::string comment;
};

enum VarTypes {
  NO, 
  event,
  integer,
  parameter,
  real,
  realtime,
  reg,
  supply0,
  supply1,
  time,
  tri,
  triand,
  trior,
  trireg,
  tri0,
  tri1,
  wand,
  wire,
  wor
};

struct VarData {
  VarTypes type;
  int size;
  std::string identifier;
  std::string trueName;
};

enum ScopeTypes { NIL, BEGIN, FORK, FUNCTION, MODULE, TASK, UNDEFINED };

struct ScopeData {
  ScopeTypes type;
  std::string ID;
  std::string name;
  std::vector<VarData> vars;
  std::string parentScopeID;
};

struct VectorValueChangeData {
  char type;
  std::string valueVec;
  std::string identifier;
};

struct ScalarValueChangeData {
  char value;
  std::string identifier;
};

enum DumpType { REGULAR, VARS, ALL, OFF, ON };
struct DumpData {
  DumpType type;
  std::vector<VectorValueChangeData> vecs;
  std::vector<ScalarValueChangeData> scals;
};

struct TimestampData {
  int time;
  DumpData data;
};

class VCDData {

public:

  DateData date;
  VersionData version;
  TimescaleData timescale;
  std::vector<CommentData> comments;
  std::vector<ScopeData> scopes;
  std::vector<TimestampData> timepoints;

  std::vector<std::string> errors;
  std::vector<std::string> warns;
};

} // namespace VCD
