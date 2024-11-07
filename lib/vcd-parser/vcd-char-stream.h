#pragma once

#include <fstream>
#include <string>

/*////////////////////////////////////////////////////////////////////////////

  This class (as well as VCDTokenStream) was inspired by Mihai Bazon and his
  project (https://lisperator.net/pltut/).
  
////////////////////////////////////////////////////////////////////////////*/

class VCDCharStream {

public:
  /**
   * @brief Construct a new VCDCharStream::VCDCharStream object.
   *
   * @param filepath - accepts a file of VCD format that not
   *                   necessarily has .vcd extension. Content
   *                   is what matters.
   */
  VCDCharStream(std::string filepath);

  /**
   * @brief Destroy the VCDCharStream::VCDCharStream object.
   *
   * @details Closes the `input` file handle;
   */
  ~VCDCharStream();

  /**
   * @brief Returns next char from the stream, even if that's
   *         a line break or a white space.
   *
   * @warning This is a destructive operation (this char
   *          cannot be obtained from the stream again
   *          after the call).
   *
   * @return char - next char taken from the `input` stream.
   */
  char next();

  /**
   * @brief Returns next char of the stream, leaving it
   *        in the stream.
   *
   * @return char - next char that may be taken from the `input`
   *                stream.
   */
  char peek();

  /**
   * @brief Peeks the next char of the `input` stream.
   *
   * @return true - the next char is eof (all meaningful chars)
   *                were taken from the stream.
   * @return false - there are more meaningful chars in the stream.
   */
  bool eof();

  /**
   * @brief throws error for a master of the char stream to catch.
   *        Should effectively stop stream from further execution.
   *
   * @param message - to show along with the line and column of the error.
   */
  void die(std::string message);

private:
  int column = 0;
  int line = 1;

  std::fstream input;
};
