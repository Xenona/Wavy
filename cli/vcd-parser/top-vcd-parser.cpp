#include "vcd-char-stream.h"
#include "vcd-parser.h"
#include "vcd-token-stream.h"
#include <iostream>

int main(int argc, char** argv) {

    std::cout << test(5) << std::endl;
    auto vcdtokenizer = new VCDTokenStream(new VCDCharStream());
    
    
    return 0;
} 
