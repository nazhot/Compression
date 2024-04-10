#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "delta.h"

int main( int argc, char *argv[] ) {
    char *outputFileName = "huffman_output";
    char *inputFileName = "huffman_input";
    huffman_encode( inputFileName, outputFileName );
    huffman_decode( outputFileName );

    char *delta_inputFileName = "delta_input";
    char *delta_outputFileName = "delta_output";
    deltaFileIntoFile( delta_inputFileName, delta_outputFileName );

    return 0;
}
