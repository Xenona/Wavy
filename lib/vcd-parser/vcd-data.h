#pragma once

#include <string>
#include <vector>
#include <map>

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

enum class VarTypes {
  NIL,
  Event,
  Integer,
  Parameter,
  Real,
  Realtime,
  Reg,
  Supply0,
  Supply1,
  Time,
  Tri,
  Triand,
  Trior,
  Trireg,
  Tri0,
  Tri1,
  Wand,
  Wire,
  Wor
};

// todo
// use this as a pointer in *changedata
struct VarData {
  VarTypes type;
  int size;
  std::string identifier;
  std::string trueName;
};

enum class ScopeTypes { NIL, Begin, Fork, Function, Module, Task, Undefined };
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
  long long valueVecDec;
  double valueVecDecFloat;
  std::string identifier;
};

struct ScalarValueChangeData {
  int value;
  std::string stringValue;
  std::string identifier;
};

enum class DumpType { NIL, Vars, All, Off, On };
struct DumpData {
  DumpType type;
  std::vector<VectorValueChangeData> vecs;
  std::vector<ScalarValueChangeData> scals;
};

struct TimestampData {
  int time;
  DumpData data;
};

// todo
// make the lexer check whether timepoint integers grow only
// otherwise a no-no and throw and error
 
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
