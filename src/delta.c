#include "delta.h"
#include <stdio.h>
#include <stdlib.h>



char* deltaFileIntoString( const char *inputFileName ) {
    FILE *inputFile = fopen( inputFileName, "r" );
    if ( !inputFile ) {
        fprintf( stderr, "Cannot open file %s (delta)\n", inputFileName );
        return "";
    }

    fseek( inputFile, 0, SEEK_END );
    const long inputFileSize = ftell( inputFile );
    fseek( inputFile, 0, SEEK_SET );

    char *fileContents = malloc( sizeof( char ) * inputFileSize );
    if ( !fileContents ) {
        fprintf( stderr, "Cannot allocate enough memory for file %s (delta)\n", inputFileName );
        return "";
    }
    fread( fileContents, 1, inputFileSize, inputFile );

    fclose( inputFile );

    return fileContents;
}
