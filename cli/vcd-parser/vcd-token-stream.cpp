#include "./vcd-token-stream.h"
#include "vcd-char-stream.h"
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
  return this->isCharInStr(" \t\n", space);
}

bool VCDTokenStream::isLetter(char identifier) {
  if (((int)'!' <= identifier && identifier <= (int)'~') &&
      !(this->isDigit(identifier)) && !(this->isPunctuation(identifier)))
    return true;
  return false;
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

Token VCDTokenStream::readIdentifier() {
  return {TokenType::Identifier,
          this->readWhile([this](char c) { return this->isLetter(c); })};
}

Token VCDTokenStream::readNext() {
  this->readWhile([this](char c) { return this->isWhiteSpace(c); });
  if (this->charStream->eof())
    return {};
  char ch = this->charStream->peek();
  if (this->isLetter(ch))
    return readIdentifier();
  if (this->isDigit(ch))
    return readNumber();
  if (this->isPunctuation(ch))
    return {TokenType::Punctiation, std::string{this->charStream->next()}};
  this->charStream->die("Can't handle character:" + std::string{ch});
}

Token VCDTokenStream::peek() {
  if (this->currentToken.type != TokenType::NIL) {
    return currentToken;
  }
  this->currentToken = this->readNext();
  return this->currentToken;
}

Token VCDTokenStream::next() {
  Token token = this->currentToken;
  this->currentToken = {};
  if (token.type != TokenType::NIL) {
    return token;
  }
  return readNext();
}

bool VCDTokenStream::eof() { return (this->peek().type == TokenType::NIL); }

VCDTokenStream::VCDTokenStream(VCDCharStream *charStream) {
  this->charStream = charStream;
}

VCDTokenStream::~VCDTokenStream() { delete this->charStream; }
