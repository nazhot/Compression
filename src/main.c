#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

int main( int argc, char *argv[] ) {
    printf( "Working\n" );
    huffman_encode( "Tester", 6 );
    return 0;
}
