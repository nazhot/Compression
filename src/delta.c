#include "delta.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

String deltaFileIntoString( const char *inputFileName ) {
    FILE *inputFile = fopen( inputFileName, "r" );
    if ( !inputFile ) {
        fprintf( stderr, "Cannot open file %s (delta)\n", inputFileName );
        return ( String ) { NULL, 0 };
    }

    fseek( inputFile, 0, SEEK_END );
    const long inputFileSize = ftell( inputFile );
    printf( "File Size: %li\n", inputFileSize );
    fseek( inputFile, 0, SEEK_SET );

    char *fileContents = malloc( sizeof( char ) * inputFileSize + 1 );
    fileContents[inputFileSize] = '\0';
    if ( !fileContents ) {
        fprintf( stderr, "Cannot allocate enough memory for file %s (delta)\n", inputFileName );
        fclose( inputFile );
        return ( String ) { NULL, 0 };
    }
    fread( fileContents, 1, inputFileSize, inputFile );
    uint lastBit = 0;
    for ( long i = 0; i < inputFileSize - 1; ++i ) {
        char currentByte = 0;
        char fileByte = fileContents[i];
        for ( uint8_t bitIndex = 7; bitIndex != 255; --bitIndex ) {
            uint8_t currentBit = fileByte >> bitIndex & 1;
            currentByte |= ( ( currentBit != lastBit ) & 1 ) << bitIndex;
            lastBit = currentBit;
        }
        fileContents[i] = currentByte;
    }

    FILE *outputFile = fopen( "delta_output", "w" );
    fwrite( fileContents, 1, inputFileSize, outputFile );
    fclose( outputFile );

    fclose( inputFile );

    return ( String ) { .content = fileContents, .size = inputFileSize };
}

void deltaFileIntoFile( const char *inputFileName, const char *outputFileName ) {
    FILE *outputFile = fopen( outputFileName, "w" );
    if ( !outputFile ) {
        fprintf( stderr, "Couldn't open output file %s (delta)\n", outputFileName );
        return;
    }
    String string = deltaFileIntoString( inputFileName );
    fwrite( string.content, 1, string.size, outputFile );
    free( string.content );
    fclose( outputFile );
}
