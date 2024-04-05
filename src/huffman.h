#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <stdlib.h>

void huffman_encode( const char *stringToCompress, const size_t stringLength );
void huffman_decode( const char *stringToDecompress );

#endif
