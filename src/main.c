#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

int main( int argc, char *argv[] ) {
    printf( "Working\n" );
    char *string = "This is a bit of a longer string just to test how it handles something like this";
    huffman_encode( string, strlen( string ) );
    FILE *inputFile = fopen( "output", "r" );
    huffman_decode( inputFile );
    return 0;
}
