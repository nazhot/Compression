#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <stdlib.h>
#include <stdio.h>

void huffman_encode( const char *stringToCompress, const size_t stringLength );
void huffman_decode( FILE *inputFile );

#endif
