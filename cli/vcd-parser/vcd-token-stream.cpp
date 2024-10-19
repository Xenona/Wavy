#include "./vcd-token-stream.h"
#include "vcd-char-stream.h"
#include <QDebug>
#include <qlogging.h>
#include <string>

bool VCDTokenStream::isDigit(char digit) {
  if ((int)'0' <= digit && digit <= (int)'9') {
    return true;
  }
  return false;
}

bool VCDTokenStream::isCharInStr(std::string samples, char ch) {
  int pos = samples.find(ch);
  if (pos == std::string::npos)
    return false;
  return true;
}

bool VCDTokenStream::isWhiteSpace(char space) {
  return this->isCharInStr(" \t\n\r", space);
}

bool VCDTokenStream::isLetter(char identifier) {
  if (((int)'!' <= identifier && identifier <= (int)'~'))
    return true;
  return false;
}

bool VCDTokenStream::isScalarMark(char letter) {
  return this->isCharInStr("01xXzZ", letter);
}

bool VCDTokenStream::isVectorBitMark(char letter) {
  return this->isCharInStr("bB", letter);
}

bool VCDTokenStream::isVectorRealMark(char letter) {
  return this->isCharInStr("rR", letter);
}

bool VCDTokenStream::isPunctuation(char punctuation) {
  return this->isCharInStr("[:]", punctuation);
}

std::string VCDTokenStream::readWhile(std::function<bool(char)> predicate) {
  std::string str;

  while (!this->charStream->eof() && predicate(this->charStream->peek())) {
    str += this->charStream->next();
  }
  return str;
}

bool VCDTokenStream::isVectorBitDump(std::string str) {
  for (char c : str) {
    if (!this->isScalarMark(c))
      return false;
  }
  return true;
}

Token VCDTokenStream::readNumber() {
  bool *isFloat = new bool(false);
  std::string number = readWhile([this, &isFloat](char c) {
    if (c == '.') {
      if (isFloat)
        return false;
      delete isFloat;
      isFloat = new bool(true);
      return true;
    }
    return this->isDigit(c);
  });
  delete isFloat;
  return {TokenType::Number, number};
}

bool VCDTokenStream::isInteger(std::string str) {
  for (char c : str) {
    if (!this->isDigit(c))
      return false;
  }
  return true;
}

bool VCDTokenStream::isIdentifier(std::string str) {
  for (char c : str) {
    if (!this->isLetter(c))
      return false;
  }
  return true;
}

Token VCDTokenStream::readIdentifier() {
  return {TokenType::Identifier, this->readWhile([this](char c) {
            return this->isLetter(c) || this->isDigit(c) ||
                   this->isPunctuation(c);
          })};
}

Token VCDTokenStream::readNext() {
  auto a = this->readWhile([this](char c) { return this->isWhiteSpace(c); });
  if (this->charStream->eof())
    return {};
  char ch = this->charStream->peek();
  if (this->isLetter(ch) || this->isDigit(ch) || this->isPunctuation(ch))
    return readIdentifier();
  this->charStream->die("Can't handle character:" + std::string{ch});
  return {};
}

void VCDTokenStream::dbg() {
  while (!this->charStream->eof()) {
    char ch = this->charStream->next();
    qDebug() << ch;
  }
}

Token VCDTokenStream::peek() {
  if (this->currentToken.type != TokenType::NIL) {
    return currentToken;
  }
  this->currentToken = this->readNext();
  return this->currentToken;
}

Token VCDTokenStream::next() {
  Token nextToken = this->currentToken;
  this->currentToken = {};
  if (nextToken.type != TokenType::NIL) {

    if (nextToken.type == TokenType::Identifier) {

      if (nextToken.value == "$date")
        return {TokenType::DateKeyword, nextToken.value};
      if (nextToken.value == "$end") {
        this->processingComment = false;
        return {TokenType::EndKeyword, nextToken.value};
      }
      if (nextToken.value == "$version")
        return {TokenType::VersionKeyword, nextToken.value};
      if (nextToken.value == "$timescale")
        return {TokenType::TimescaleKeyword, nextToken.value};
      if (nextToken.value == "$comment") {
        this->processingComment = true;
        return {TokenType::CommentKeyword, nextToken.value};
      }
      if (nextToken.value == "$scope")
        return {TokenType::ScopeKeyword, nextToken.value};
      if (nextToken.value == "$upscope")
        return {TokenType::UpscopeKeyword, nextToken.value};
      if (nextToken.value == "$var")
        return {TokenType::VarKeyword, nextToken.value};
      if (nextToken.value == "$enddefinitions") {
        this->processingDumps = 1;
        return {TokenType::EnddefinitionsKeyword, nextToken.value};
      }
      if (nextToken.value == "$dumpvars")
        return {TokenType::DumpvarsKeyword, nextToken.value};
      if (nextToken.value == "$dumpall")
        return {TokenType::DumpallKeyword, nextToken.value};
      if (nextToken.value == "$dumpon")
        return {TokenType::DumponKeyword, nextToken.value};
      if (nextToken.value == "$dumpoff")
        return {TokenType::DumpoffKeyword, nextToken.value};
      if (nextToken.value[0] == '#' &&
          this->isInteger(nextToken.value.substr(1))) {
        if (this->processingDumps == 2)
          this->charStream->die("Vector value has no identifier");
        if (this->processingDumps == 1)
          return {TokenType::SimulationTime, nextToken.value.substr(1)};
        else
          return {TokenType::Identifier, nextToken.value};
      }

      if (this->processingDumps == 0) {

      } else if (this->processingDumps == 1) {

        if (this->isScalarMark(nextToken.value[0]) &&
            this->isIdentifier(nextToken.value.substr(1)))
          return {TokenType::ScalarValueChange, nextToken.value};
        else if (this->isVectorBitMark(nextToken.value[0]) &&
                 this->isVectorBitDump(nextToken.value.substr(1))) {
          this->processingDumps = 2;
          return {TokenType::VectorValueChange, nextToken.value};
        } else if (!this->processingComment) {
          this->charStream->die("Wrong data token.");
        }

      } else if (this->processingDumps == 2) {
        this->processingDumps = 1;
        return {TokenType::Identifier, nextToken.value};
      }
    }

    return nextToken;
  }

  return readNext();
}

bool VCDTokenStream::eof() { return (this->peek().type == TokenType::NIL); }

VCDTokenStream::VCDTokenStream(VCDCharStream *charStream) {
  this->charStream = charStream;
}

VCDTokenStream::~VCDTokenStream() { delete this->charStream; }
