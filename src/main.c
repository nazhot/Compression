#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

int main( int argc, char *argv[] ) {
    char *outputFileName = "huffman_output";
    char *inputFileName = "huffman_input";
    huffman_encode( inputFileName, outputFileName );
    huffman_decode( outputFileName );
    return 0;
}
