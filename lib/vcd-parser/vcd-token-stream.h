#pragma once

#include <functional>
#include <string>

class VCDCharStream;

enum class TokenType {
  NIL  = 0,
  DateKeyword,
  VersionKeyword,
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
  
  VCDTokenStream(VCDCharStream *charStream);
  ~VCDTokenStream();

  static bool isDigit(char digit);
  static bool isWhiteSpace(char space);
  static bool isLetter(char identifier);
  static bool isCharInStr(std::string samples, char ch);
  static bool isInteger(std::string str);
  static bool isScalarMark(char letter); 
  static bool isVectorBitMark(char letter);
  static bool isVectorRealMark(char letter);
  static bool isIdentifier(std::string str);
  static bool isVectorBitDump(std::string str);
  static bool isReal(std::string str);

  Token peek();
  Token next();
  bool eof();
  void dbg();

private:

  Token readIdentifier();
  Token readSimulationTime();
  Token readNext();

  std::string readWhile(std::function<bool(char)> predicate);

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
