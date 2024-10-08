#pragma once
#include <functional>
#include <string>
#include <vector>

class VCDCharStream;

enum TokenType {
  NIL  = 0,
  DeclarationKeyword,
  SimulationKeyword,
  EndKeyword,
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
  
  Token readIdentifier();
  Token readSimulationTime();
  Token readNumber();
  Token readNext();

  std::string readWhile(std::function<bool(char)> predicate);

  Token peek();
  Token next();
  bool eof();

private:
  Token currentToken = {TokenType::NIL};
  VCDCharStream *charStream;
};
