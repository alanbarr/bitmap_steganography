/**
 * @file bitmap_steganography.h
 * @brief Contains typical headery file stuff for bitmap_steganography.c
 *
 * Copyright 
 *  @author Alan Barr
 *  @date April 2012
 * @section License
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BITMAP_ENCODER_H_
#define _BITMAP_ENCODER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/** Index to argv for bitmap file. */
#define BITMAP_FILE                 1
/** Index to argv for data file to encode into bitmap. */
#define ENCODE_FILE                 2

#define DEBUG_VALUES                __FILE__, __LINE__
#define DEBUG_STRING                "%s:%d"

#define PRINT(string)               fprintf(stdout, DEBUG_STRING " " string "\n", DEBUG_VALUES)
#define ERROR_PRINT(err)            fprintf(stderr, DEBUG_STRING " error: %s \n" , DEBUG_VALUES, errorString[err])
#define ERROR_ERRNO_PRINT(err)      fprintf(stderr, DEBUG_STRING " error: %s" " errno: %s \n", DEBUG_VALUES, errorString[err], strerror(errno))

#define EXTENSION_SIZE              3
#define DATA_SIZE                   8
#define PIXELS_TO_STORE_DATA_LEN    24

#define BYTES_IN_PIXEL              3

#define BLUE                        0
#define GREEN                       1
#define RED                         2

/** Bitmask used when encoding / decoding data to a blue byte. */
#define BLUE_BITMASK                0x07
/** Bitmask used when encoding / decoding data to a green byte. */
#define GREEN_BITMASK               0x07
/** Bitmask used when encoding / decoding data to a red byte. */
#define RED_BITMASK                 0x03

/** Number of bits used to store data in a blue byte. */
#define BLUE_BITS                   3
/** Number of bits used to store data in a green byte. */
#define GREEN_BITS                  3
/** Number of bits used to store data in a red byte. */
#define RED_BITS                    2

#if((BLUE_BITS + GREEN_BITS + RED_BITS) != 8)
    #error "Bits stored in pixel != 8"
#endif

#define OUTPUT_NAME_SIZE            15

/** Identifies a pointer argument passes data into a function. */
#define IN
/** Identifies a pointer argument passes data out of a function. */
#define OUT
/** Identifies a pointer argument passes data into and out of a function. */
#define IN_OUT

/** List of errors which can be returned from functions. */
#define ERRORS           \
    ERROR(success)       \
    ERROR(errorDefault)  \
    ERROR(errorFopen)    \
    ERROR(errorFclose)   \
    ERROR(errorMalloc)   \
    ERROR(errorFread)    \
    ERROR(errorFwrite)   \
    ERROR(errorSize)     \
    ERROR(errorNull)     \
    ERROR(errorFileType) \
    ERROR(errorFseek)    \


#undef ERROR
/** Defines error to get numerical value from list for enum. */
#define ERROR(x) x,
/** Holds numerical values for ERRORS */
enum eErrors { 
    ERRORS
};

typedef enum eErrors tError;

#undef ERROR
/** Defines error to stringify list for debug. */
#define ERROR(x) #x,
/** Holds the error names in strings. Compliments eErrors. */
char * errorString[] = { ERRORS };


/** Changes structure packing allowing memcpy to work for the file and info
 *  headers. */
#pragma pack(1)

typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offsetbits;
} tBitmapFileHeader;

typedef struct {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compressionType;
    uint32_t imageDataSize;
    int32_t horizontalResolution;
    int32_t verticalResolution;
    uint32_t numberOfColours;
    uint32_t numberOfImportantColours;
} tBitmapInfoHeader;


void printFileHeader(IN const tBitmapFileHeader * pFileHeader);

void printInfoHeader(IN const tBitmapInfoHeader * pInfoHeader);

tError decoding(IN char ** argv);

tError encoding(IN char ** argv);

tError parseBitmap(IN FILE * fpBitmap, 
                   OUT tBitmapFileHeader * pFileheader, 
                   OUT tBitmapInfoHeader * pInfoheader, 
                   OUT uint8_t * pPadding,
                   OUT uint64_t * pSizeofdata);

tError copyBitmapData(FILE * fpBitmap, uint8_t ** pData, uint64_t dataSize);

tError encodeDataFileContents(IN FILE * fpDataFile, 
                              IN const char * dataFileName,
                              IN_OUT uint8_t * pData, 
                              uint64_t dataSize, 
                              uint32_t width, 
                              uint8_t padding);

tError createOutputBitmap(IN const tBitmapFileHeader * pFileHeader, 
                          IN const tBitmapInfoHeader *pInfoHeader,
                          IN const uint8_t * pData,
                          uint64_t dataSize);

tError createOutputFile(IN char * extension, 
                        IN uint8_t * pFileData,
                        uint64_t dataSizeBytes);

tError validateSizes(uint64_t bitmapFileSizeBytes,
                     uint64_t pixelSizeBytes,
                     uint64_t filePaddingSizeBytes);

tError fileSize(IN FILE * fpFile, OUT uint64_t * size);

tError decodeData(IN uint8_t * encodedData,
                  uint64_t encodedDataSize, 
                  uint64_t startOfEncodedDataIndex,
                  uint64_t width,
                  uint64_t padding,
                  OUT uint8_t * decodedData);

tError parseEncodedData(IN uint8_t * pImageData, 
                        uint64_t imageDataSize,
                        uint8_t padding,
                        uint64_t width,
                        OUT uint64_t * pStartOfEncodedDataIndex, 
                        OUT char * pExtension, 
                        OUT uint64_t * pEncodedDataSize);


tError checkPadding(IN_OUT uint64_t * index, uint64_t width, uint64_t padding);


#endif


