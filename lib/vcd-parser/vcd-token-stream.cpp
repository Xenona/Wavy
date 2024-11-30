#include "./vcd-token-stream.h"
#include "./vcd-char-stream.h"
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
  return VCDTokenStream::isCharInStr(" \t\n\r", space);
}

bool VCDTokenStream::isLetter(char identifier) {
  if (((int)'!' <= identifier && identifier <= (int)'~'))
    return true;
  return false;
}

bool VCDTokenStream::isScalarMark(char letter) {
  return VCDTokenStream::isCharInStr("01xXzZ", letter);
}

bool VCDTokenStream::isVectorBitMark(char letter) {
  return VCDTokenStream::isCharInStr("bB", letter);
}

bool VCDTokenStream::isVectorRealMark(char letter) {
  return VCDTokenStream::isCharInStr("rR", letter);
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
    if (!VCDTokenStream::isScalarMark(c))
      return false;
  }
  return true;
}

bool VCDTokenStream::isInteger(std::string str) {
  for (char c : str) {
    if (!VCDTokenStream::isDigit(c))
      return false;
  }
  return true;
}

bool VCDTokenStream::isIdentifier(std::string str) {
  for (char c : str) {
    if (!VCDTokenStream::isLetter(c))
      return false;
  }
  return true;
}

Token VCDTokenStream::readIdentifier() {
  return {TokenType::Identifier,
          this->readWhile([this](char c) { return this->isLetter(c); })};
}

bool VCDTokenStream::isReal(std::string str) {
  bool hasDot = false;
  for (char c : str) {
    if (c == '.') {
      if (hasDot)
        return false;
      hasDot = true;
    } else if (!VCDTokenStream::isDigit(c))
      return false;
  }
  return true;
}

Token VCDTokenStream::readNext() {
  // Skips all white spaces, line breaks and tabs.
  auto a = this->readWhile([this](char c) { return this->isWhiteSpace(c); });
  // return NIL token, if it's the end of the file
  if (this->charStream->eof())
    return {};
  // if the next char is letter, than we pull the whole word from the stream and return 
  char ch = this->charStream->peek();
  if (this->isLetter(ch))
    return readIdentifier();

  // the char wasn't an allowed letter, throw
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
      
      // Here goes a part mentioned in (1) of the class description. 
      // After all words are split, checked and inferred TokenType::Identifier
      // by default, we might simplify future parsing by changing keyword type 
      // right here 

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
        // Please, refer to (5) of the class description. 
        // This will help to ignore all the usual words, as 
        // not ignoring them will break the context tracking 
        // type of variable dumped, because $comment is in 
        // the dumps section of the file. Refer to the IEEE Std 1800-2023
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
        // Here we finish with header and move to the dumps
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

      // >1 check is required, because `#` might be just an identifier code
      if (nextToken.value[0] == '#' && (nextToken.value.length() > 1) &&
          this->isInteger(nextToken.value.substr(1))) {

        // If we met a timestamp when were waiting (2nd state) for vector 
        // identifier, that's an error and further parsing cannot be done
        if (this->processingDumps == 2)
          this->charStream->die("Vector value has no identifier");
        // if state is 1st, then stream just gave a timestamp, so
        // it's necessary to take the time
        if (this->processingDumps == 1) {
          
          return {TokenType::SimulationTime, nextToken.value.substr(1)};
        }  
        else
          return {TokenType::Identifier, nextToken.value};
      }

      // this branch left for understanding we do nothing 
      // specific in a header section
      if (this->processingDumps == 0) {

      } else if (this->processingDumps == 1) {

        if (this->isScalarMark(nextToken.value[0]) &&
            this->isIdentifier(nextToken.value.substr(1)))
          return {TokenType::ScalarValueChange, nextToken.value};
        else if (this->isVectorBitMark(nextToken.value[0]) &&
                 this->isVectorBitDump(nextToken.value.substr(1))) {
          this->processingDumps = 2;
          return {TokenType::VectorValueChange, nextToken.value};
        } else if (this->isVectorRealMark(nextToken.value[0]) &&
                   this->isReal(nextToken.value.substr(1))) {
          this->processingDumps = 2;
          return {TokenType::VectorValueChange, nextToken.value};
        }

        else if (!this->processingComment) {
          this->charStream->die("Wrong data token: " + nextToken.value);
        }

      } else if (this->processingDumps == 2) {
        this->processingDumps = 1;
        return {TokenType::Identifier, nextToken.value};
      }
    }

    // here type will be TokenType::Identifier
    return nextToken;
  }

  return readNext();
}

bool VCDTokenStream::eof() { return (this->peek().type == TokenType::NIL); }

VCDTokenStream::VCDTokenStream(VCDCharStream *charStream) {
  this->charStream = charStream;
}

VCDTokenStream::~VCDTokenStream() { delete this->charStream; }
