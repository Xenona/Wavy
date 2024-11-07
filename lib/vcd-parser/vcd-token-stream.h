#pragma once

#include <functional>
#include <string>

/////////////////////////////////////////////////////////////////////////////
//  This class (as well as VCDCharStream) was inspired by Mihai Bazon and his
//  project (https://lisperator.net/pltut/).
/////////////////////////////////////////////////////////////////////////////

class VCDCharStream;

// Tokens are inferred from IEEE Std 1800-2023.
enum class TokenType {
  NIL = 0,
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

/////////////////////////////////////////////////////////////////////////////
//  This class performs tokenization of the VCD file. There are some      (1)
//  pitfalls one may encounter when implementing such logic, and the
//  biggest one is the fact that dollar sign which is used to mark
//  the beginning of a keyword, can be a part of identifier, or just
//  be the identifier. Same applies to '#'.
//
//  Thus when parsing, one cannot just state that any word starting       (2)
//  with '$' is actually a keyword, because '$a' is just an identifier
//  code for a variable. To be sure, one should strictly compare.
//
//  To be fair, I had thoughts that it's actually possible to get an
//  identifier code that will be identical to some keyword (like $end),
//  but the dump should be unmaintainable huge for that.
//
//  So for eliminating issue (1), one should track the context. This      (3)
//  exact code tracks whether the header or the dumps are being written
//  at this time, and, if dumps, the code actually expects one word
//  for a scalar dump and two words for a vector one, according to IEEE
//  Std 1800-2023.
//
//  If a VCD file breaks context epectations, an error will be thrown.    (4)
// 
//  VCD file might have any keyword inside, except `$end`. If a keyword   (5)
//  is met in a wrong place, an error is thrown, though.
//
//
/////////////////////////////////////////////////////////////////////////////

class VCDTokenStream {

public:
  /**
   * @brief Construct a new VCDTokenStream object
   *
   * @param charStream - instance of VCDCharStream. Will be
                         handled by VCDTokenStream in destructor.
   */
  VCDTokenStream(VCDCharStream *charStream);

  /**
   * @brief Destroy the VCDTokenStream object
   * @details deletes `charStream` passed during instantiation
   */
  ~VCDTokenStream();

  /**
   * @brief Checks whether a given char is a digit
   *
   * @param digit - character to check
   * @return true
   * @return false
   */
  static bool isDigit(char digit);

  /**
   * @brief Checks whether a given char is either \t, \n or a whitespace.
   *
   * @param space - character to check
   * @return true
   * @return false
   */
  static bool isWhiteSpace(char space);

  /**
   * @brief Checks whether a given char is a valid ASCII code
            according to IEEE Std 1800-2023.
   *
   * @param identifier - character to check
   * @return true
   * @return false
   */
  static bool isLetter(char identifier);

  /**
   * @brief Checks whether a char is in a given string
   * @details For an example usage of this function, please refer
   *          to the `isWhiteSpace`, `isVectorRealMark`, `isVectorBitMark`,
   *          `isScalarMark`, etc. functions.
   *
   *
   * @param samples - a string of characters
   * @param ch - char to find in a `samples` string
   * @return true
   * @return false
   */
  static bool isCharInStr(std::string samples, char ch);

  /**
   * @brief checks whether a given string is an integer
   *
   * @param str - a string to check
   * @return true
   * @return false
   */
  static bool isInteger(std::string str);

  /**
   * @brief Checks whether a given string is a real number
   * @warning The function checks for a decimal point, not comma
   *
   * @param str - a string to check
   * @return true
   * @return false
   */
  static bool isReal(std::string str);

  /**
   * @brief Checks whether a given letter is a correct one
   *        to start a `scalar_value_change` (please, refer to
   *        IEEE Std 1800-2023, p. 688) from.
   *
   * @param letter
   * @return true
   * @return false
   */
  static bool isScalarMark(char letter);

  /**
   * @brief Checks whether a given letter is a correct one
   *        to start a *binary* `vector_value_change` (please, refer to
   *        IEEE Std 1800-2023, p. 688) from.
   *
   * @param letter - a letter to check
   * @return true
   * @return false
   */
  static bool isVectorBitMark(char letter);

  /**
   * @brief Checks whether a given letter is a correct one
   *        to start a *real* `vector_value_change` (please, refer to
   *        IEEE Std 1800-2023, p. 688) from.
   *
   * @param letter - a letter to check
   * @return true
   * @return false
   */
  static bool isVectorRealMark(char letter);

  /**
   * @brief Checks whether a given string is a valid identifier. Uses `isLetter`
   *        inside.
   *
   * @param str - a string to check
   * @return true
   * @return false
   */
  static bool isIdentifier(std::string str);

  /**
   * @brief Checks whether a given string is a valid bit vector dump.
   *
   * @param str - a string to check
   * @return true
   * @return false
   */
  static bool isVectorBitDump(std::string str);

  /**
   * @brief Retuns the next token from the file. This is non-destructive
   *        operation, and stream remains unchanged.
   *
   * @return Token - next token in the file
   */
  Token peek();

  /**
   * @brief Retuns the next token from the file. This is a destructive
   *        operation, and stream moves to the next token.
   *
   * @return Token - token taken from the file
   * @throws when dump is wrong (keyword is met in a wrong place or vector dump lost its identifier, etc.)
   */
  Token next();

  /**
   * @brief Indicates whether there are more tokens in the file.
   *
   * @return true
   * @return false
   */
  bool eof();

  /**
   * @brief Function existing solely for purposes of debugging.
   * @warning This function empties a char stream of the object. Do not call it.
   *
   */
  void dbg();

private:
  /**
   * @brief Reads while the chars comply with the `letter` definition from the
   * standard.
   *
   * @return Token
   */
  Token readIdentifier();

  /**
   * @brief Works directly with `charStream` and constructs a token from chars
   * read.
   *
   * @return Token
   */
  Token readNext();

  /**
   * @brief Skips all the char while the predicate is truthful.
   *
   * @param predicate - a function to check for each character.
   * @return std::string - skipped string
   * @throws if character is invalid from the point of the IEEE Std 1800-2023 
   */
  std::string readWhile(std::function<bool(char)> predicate);

  Token currentToken = {TokenType::NIL};
  VCDCharStream *charStream;

  /*
    0 - before enddefinitions
    1 - expecting scalar or vector value
    2 - expecting vector identifier, if previously read a vector value
  */
  short processingDumps = 0;

  // if comment contains keywords, they must
  // be ignored
  bool processingComment = false;
};
