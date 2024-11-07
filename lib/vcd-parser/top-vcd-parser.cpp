#include "vcd-char-stream.h"
#include "vcd-parser.h"
#include "vcd-token-stream.h"
#include <iostream>

/////////////////////////////////////////////////////////////////////////////
//  This file is dedicated for manual testing of the library vcd-parser.
//
//  One may not seek any 'business' logic behind all written here, as
//  it exists solely for testing purposes.
// 
//  I was using vscode with launch.json of this configuration:              (1)
//  {
//      "version": "0.2.0",
//      "configurations": [
//        {
//          "name": "Cmake Debug Target",
//          "type": "lldb",
//          "request": "launch",
//          "program": "${command:cmake.launchTargetPath}",
//          "args": ["<path_to_the_repository>/examples/std.vcd"],
//          "cwd": "${workspaceFolder}"
//        }
//      ]
//  }
/////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // See (1)
  std::string path = argv[1];
  auto vcdtokenizer = new VCDTokenStream(new VCDCharStream(path));

  auto vcdparser = new VCDParser();
  vcdparser->dbg(vcdtokenizer);

  // Adding some output to see the program, at least, was launched correctly  
  std::cout << 5 << std::endl;

  return 0;
}
