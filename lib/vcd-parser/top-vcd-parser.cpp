#include "vcd-char-stream.h"
#include "vcd-parser.h"
#include "vcd-token-stream.h"
#include <iostream>

int main(int argc, char* argv[]) {

    std::string path = argv[1];
    auto vcdtokenizer = new VCDTokenStream(new VCDCharStream(path));

    auto vcdparser = new VCDParser();
    vcdparser->dbg(vcdtokenizer);

    std::cout << test(5) << std::endl;
    
    return 0;
} 
