#pragma once
#include <functional>
#include <string>
#include <vector>

class VCDCharStream;

enum TokenType {
  NIL  = 0,
  DateKeyword,
  VersionKeyword,
  DeclarationKeyword,
  SimulationKeyword,
  EndKeyword,
  TimescaleKeyword,
  VarKeyword,
  ScopeKeyword,
  EnddefinitionsKeyword, 
  UpscopeKeyword,
  CommentKeyword,
  DumpvarsKeyword,
  DumpallKeyword,
  DumponKeyword,
  DumpoffKeyword,
  ObjectType,
  ObejctSize,
  Punctiation,
  Number,
  Operator,
  ScalarValueChange,
  VectorValueChange,
  SimulationTime,
  Identifier,
};

struct Token {
  TokenType type;
  std::string value;
};

class VCDTokenStream {

public:
  const std::vector<std::string> declarationKeywords = {
      "$comment", "$date", "$enddefinitions", "$scope", "$timescale",
      "$upscope", "$var",  "$version",
  };
  const std::vector<std::string> simulationKeywords = {
      "$dumpall",
      "$dumpoff",
      "$dumpon",
      "$dumpvars",
  };

  const std::vector<std::string> objectTypes = {
      "wire", "reg", "trired"
      // TODO add more
  };

  VCDTokenStream(VCDCharStream *charStream);
  ~VCDTokenStream();

  bool isDigit(char digit);
  bool isWhiteSpace(char space);
  bool isLetter(char identifier);
  bool isCharInStr(std::string samples, char ch);
  bool isPunctuation(char punctuation);
  bool isInteger(std::string str);
  bool isScalarMark(char letter); 
  bool isVectorBitMark(char letter);
  bool isVectorRealMark(char letter);
  bool isIdentifier(std::string str);
  bool isVectorBitDump(std::string str);

  Token readIdentifier();
  Token readSimulationTime();
  Token readNumber();
  Token readNext();

  std::string readWhile(std::function<bool(char)> predicate);

  Token peek();
  Token next();
  bool eof();
  void dbg();

private:
  Token currentToken = {TokenType::NIL};
  VCDCharStream *charStream;
  
  /*
    0 - before enddefinitions
    1 - expecting scalar or vector value // if read isScalarValue
    2 - expecting vector identifier
  */
  short processingDumps = 0; 
  bool processingComment = false; 

};
