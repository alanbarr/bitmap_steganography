/**
 * @file bitmap_steganography.c 
 * @brief Contains all functions used to encode and decode data into a bitmap
 *        image.
 *
 * Copyright:
 *  @author Alan Barr
 *  @date April 2012
 *
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
 *
 */

#include "bitmap_steganography.h"


/** @brief Determines whether encoding or decoding a bitmap is desired.
 *         If argc is 2 decoding is chosen. If argc is 3 encoding is chosen.
 *  @param argv[1] Bitmap file to be decoded/encoded.
 *  @param argv[2] For encoding only - Data file to be encoded. */
int main(int argc, char ** argv)
{
    tError errRtn = errorDefault;

    if (argc == 2)
    {
        errRtn = decoding(argv);
    }

    else if (argc == 3)
    {
        errRtn = encoding(argv);
    }

    else
    {
        printf("Useage:\n"
               "To decode a file pass the BMP file in as an arugment.\n"
               "To encode a file pass in the destination BMP and data\n"
               "in that order as arugments.\n");
    }
    
    if (errRtn == success)
    {
        printf("Success\n");
    }

    return errRtn;
}


/** @brief Handles all the retrieving of encoded information from a bitmap and
 *         saves it in a file. Bitmap file name is retrieved from argv.
 *  @param argv[0] - Filename of the bitmap file to decode.
 *  @return An error value from enum eErrors. */
tError decoding(IN char ** argv)
{
    tError errRtn = errorDefault;
    FILE * fpBitmap = NULL;
    char extension[EXTENSION_SIZE + 1] = {0};
    uint64_t encodedDataSize = 0;
    uint64_t imageDataSize = 0;
    tBitmapFileHeader fileHeader;
    tBitmapInfoHeader infoHeader;
    uint8_t * pImageData = NULL;
    uint8_t padding = 0;
    uint8_t * pEncodedData = NULL;
    uint64_t encodedDataIndex = 0;

    if ((fpBitmap = fopen(argv[BITMAP_FILE], "rb")) == NULL)
    {
        errRtn = errorFopen;
        ERROR_ERRNO_PRINT(errRtn);
    }

    else if (fgetc(fpBitmap) != 'B' || fgetc(fpBitmap) != 'M')
    {
        errRtn = errorFileType;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = parseBitmap(fpBitmap, &fileHeader, &infoHeader, &padding, 
                                   &imageDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }
    
    else if (infoHeader.bitsPerPixel != 24)
    {   
        errRtn = errorFileType;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = copyBitmapData(fpBitmap, &pImageData, imageDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if (pImageData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = parseEncodedData(pImageData, imageDataSize, padding, 
                       infoHeader.width, &encodedDataIndex, extension, 
                       &encodedDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if ((pEncodedData = malloc(encodedDataSize)) == NULL)
    {
        errRtn = errorMalloc;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = decodeData(pImageData, encodedDataSize, encodedDataIndex, 
                                  infoHeader.width, padding, pEncodedData)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = createOutputFile(extension, pEncodedData, encodedDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else
    {
        errRtn = success;
    }
    
    if (fpBitmap != NULL)
    {
        if (fclose(fpBitmap) != success) 
        {
            errRtn = errorFclose;
            ERROR_ERRNO_PRINT(errRtn);

        }
    }

    return errRtn;
}


/** @brief Handles all the encoding of data into a bitmap file. The data is 
 *         pulled from the third argument of argv and placed into the bitmap 
 *         file which should be the second argument.
 *  @param argv[0] Bitmap file to copy and hide data in.
 *  @param argv[1] Data file to be hidden.
 *  @return An error value from enum eErrors. */
tError encoding(IN char ** argv)
{
    tError errRtn = errorDefault;
    FILE * fpBitmap = NULL;
    FILE * fpDataFile = NULL;
    tBitmapFileHeader fileHeader;
    tBitmapInfoHeader infoHeader;
    uint8_t padding = 0;
    uint8_t * pImageData = NULL;
    uint64_t imageDataSize = 0;
    uint64_t dataToEncodeSize = 0;
    uint64_t bitmapFileSize = 0;

    memset(&fileHeader, 0, sizeof(tBitmapFileHeader));
    memset(&infoHeader, 0, sizeof(tBitmapInfoHeader));
    
    if ((fpBitmap = fopen(argv[BITMAP_FILE], "rb")) == NULL)
    {
        errRtn = errorFopen;
        ERROR_ERRNO_PRINT(errRtn);
    }
    
    else if (fgetc(fpBitmap) != 'B' || fgetc(fpBitmap) != 'M')
    {
        errRtn = errorFileType;
        ERROR_PRINT(errRtn);
    }

    else if ((fpDataFile = fopen(argv[ENCODE_FILE], "rb")) == NULL)
    {
        errRtn = errorFopen;
        ERROR_ERRNO_PRINT(errRtn);
    }

    else if ((errRtn = parseBitmap(fpBitmap, &fileHeader, &infoHeader, &padding, 
                                   &imageDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if (infoHeader.bitsPerPixel != 24)
    {   
        errRtn = errorFileType;
        ERROR_PRINT(errRtn);
    }
    
    else if ((errRtn = copyBitmapData(fpBitmap, &pImageData, imageDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }
    
    else if (pImageData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = fileSize(fpDataFile, &dataToEncodeSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = fileSize(fpBitmap, &bitmapFileSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }
    
    else if ((errRtn = validateSizes(bitmapFileSize, infoHeader.width * infoHeader.height * 3,
                                     padding * infoHeader.height)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if ((imageDataSize - (padding * infoHeader.height) - EXTENSION_SIZE - DATA_SIZE) < dataToEncodeSize)
    {
        errRtn = errorSize;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = encodeDataFileContents(fpDataFile, argv[ENCODE_FILE], pImageData, 
                                              imageDataSize, infoHeader.width, padding)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = createOutputBitmap(&fileHeader, &infoHeader, pImageData,
                                          imageDataSize)) != success)
    {
        ERROR_PRINT(errRtn);
    }

    else
    {
        errRtn = success;
    }

    if (fpBitmap != NULL)
    {
        if (fclose(fpBitmap) != success)
        {
            errRtn = errorFclose;
            ERROR_ERRNO_PRINT(errRtn);
        }
    }
    
    if (fpBitmap != NULL)
    {
        if (fclose(fpDataFile) != success)
        {
            errRtn = errorFclose;
            ERROR_ERRNO_PRINT(errRtn);
        }
    }

    if (pImageData != NULL)
    {
        free(pImageData);
    }

    return success;
}


/** @brief Returns the file size of an open file
 *  @param fpFile File to get the size of.
 *  @param size Returns the file size.
 *  @return An error value from enum eErrors. */
tError fileSize(IN FILE * fpFile, OUT uint64_t * size)
{
    tError errRtn = errorDefault;

    if (fpFile == NULL || size == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else
    {
        fseek(fpFile, 0, SEEK_END);
        *size = ftell(fpFile);
        errRtn = success;
    }

    return errRtn;
}


/** @brief Validates the file sizes are correct when encoding information to a
 *         bitmap.
 *  @param bitmapFileSizeBytes Size of the bitmap in bytes.
 *  @param pixelSizeBytes Size of all valid pixels in bytes.
 *  @param filePaddingSizeBytes Total padding size in bytes.
 *  @return An error value from enum eErrors. */
tError validateSizes(uint64_t bitmapFileSizeBytes, uint64_t pixelSizeBytes,
                     uint64_t filePaddingSizeBytes)
{
    tError errRtn = errorDefault;
    uint64_t total = pixelSizeBytes + sizeof(tBitmapFileHeader) + 
                     sizeof(tBitmapInfoHeader) + filePaddingSizeBytes;
    
    if (bitmapFileSizeBytes != total)
    {
        printf("pixelSizeBytes       = %llu\n"
               "headerSizeBytes      = %u\n"
               "filePaddingSizeBytes = %llu\n"
               "---------------------------\n"
               "Total                = %llu\n"
               "File Size            = %llu\n",
                pixelSizeBytes,
                sizeof(tBitmapFileHeader) + sizeof(tBitmapInfoHeader),
                filePaddingSizeBytes, total, bitmapFileSizeBytes);

        errRtn = errorSize;
    }

    else
    {
        errRtn = success;
    }

    return errRtn;
}


/** @brief Creates the output bitmap from the two header files and image data 
 *         containing hidden information. 
 *  @param pFileHeader Pointer to the file header. 
 *  @param pInfoHeader Pointer to the info header.
 *  @param pData Pointer to image data with hidden information. 
 *  @param dataSize The size of the image data.
 *  @return An error value from enum eErrors. */
tError createOutputBitmap(IN const tBitmapFileHeader * pFileHeader, 
                          IN const tBitmapInfoHeader * pInfoHeader,
                          IN const uint8_t * pData,
                          IN uint64_t dataSize)
{
    tError errRtn = errorDefault;
    FILE * fpOutputBitmap = NULL;
    
    if (pFileHeader == NULL || pInfoHeader == NULL || pData == NULL)
    {
        errRtn = errorNull;
        PRINT("NULL");
    }

    else if ((fpOutputBitmap = fopen("out.bmp", "wb")) == NULL)
    {
        errRtn = errorFopen;
        ERROR_ERRNO_PRINT(errRtn);
    }

    else if (fseek(fpOutputBitmap, 0, SEEK_SET) != success)
    {
        errRtn = errorFseek;
        ERROR_ERRNO_PRINT(errRtn);
    }

    else if (fwrite(pFileHeader, sizeof(tBitmapFileHeader), 1, fpOutputBitmap) != 1)
    {
        errRtn = errorFwrite;
        ERROR_ERRNO_PRINT(errRtn); 
    }

    else if (fwrite(pInfoHeader, sizeof(tBitmapInfoHeader), 1, fpOutputBitmap) != 1)
    {
        errRtn = errorFwrite;
        ERROR_ERRNO_PRINT(errRtn); 
    }

    else if (fwrite(pData, dataSize, 1, fpOutputBitmap) != 1)
    {
        errRtn = errorFwrite;
        ERROR_ERRNO_PRINT(errRtn); 
    }

    else
    {
        errRtn = success;
    }
    
    if (fpOutputBitmap != NULL)
    {
        if (fclose(fpOutputBitmap) != success)
        { 
            errRtn = errorFclose;
            ERROR_ERRNO_PRINT(errRtn);
        }
    }
    return errRtn;
}


/** @brief Reads the data from fpDataFile and stores it into the bitmap image 
 *         data, pData.
 *  @param fpDataFile File pointer to data to "hide" in image.
 *  @param dataFileName File name of data file to "hide" - used to get extension.
 *  @param pData Image data pointer returned with hidden data.
 *  @param dataSize Size of the data to hide in bytes.
 *  @param width Image width in bytes.
 *  @param padding Size of padding on line.
 *  @return An error value from enum eErrors. */
tError encodeDataFileContents(IN FILE * fpDataFile, 
                              IN const char * dataFileName,
                              IN_OUT uint8_t * pData, 
                              uint64_t dataSize, 
                              uint32_t width, 
                              uint8_t padding)
{
    tError errRtn = errorDefault;
    char * extension = NULL;
    uint64_t dataIndex = 0;
    uint8_t * pDataToEncode = NULL;
    uint8_t * pDataToEncodeTemp = NULL;
    uint8_t bitShift = 0;
    uint64_t sizeOfDataToEncode = 0;
    uint32_t extensionIndex = 0;
    uint64_t encodedBytes = 0;
    
    if ((extension = strrchr(dataFileName, '.')) == NULL)
    {
        extension = ".\0\0\0";
    }

    if (fpDataFile == NULL || pData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if ((errRtn = fileSize(fpDataFile, &sizeOfDataToEncode)) != success)
    {
        ERROR_PRINT(errRtn);
    }
    
    else if (fseek(fpDataFile, 0, SEEK_SET) != success)
    {
        errRtn = errorFseek;
        ERROR_PRINT(errRtn);
    }
    
    else if (pData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if ((pDataToEncodeTemp = pDataToEncode = malloc(dataSize)) == NULL)
    {
        errRtn = errorMalloc;
        ERROR_PRINT(errRtn);
    }

    else if (fread(pDataToEncode, sizeof(uint8_t), sizeOfDataToEncode, fpDataFile)
             != sizeOfDataToEncode)
    {
        errRtn = errorMalloc;
        ERROR_PRINT(errRtn);
    }

    
    else
    {
        bitShift = 0;
        
        /* Remove decimal point from extension */
        extension = &extension[1];

        while (dataIndex < PIXELS_TO_STORE_DATA_LEN)
        { 
            checkPadding(&dataIndex, width, padding);

            /* Zero bits on pixel */
            pData[dataIndex + BLUE]  &= ~BLUE_BITMASK;
            pData[dataIndex + GREEN] &= ~GREEN_BITMASK;
            pData[dataIndex + RED]   &= ~RED_BITMASK;

            pData[dataIndex + BLUE]  |= (sizeOfDataToEncode >> bitShift) & BLUE_BITMASK;
            bitShift += BLUE_BITS;
            pData[dataIndex + GREEN] |= (sizeOfDataToEncode >> bitShift) & GREEN_BITMASK;
            bitShift += GREEN_BITS;
            pData[dataIndex + RED]   |= (sizeOfDataToEncode >> bitShift) & RED_BITMASK;
            bitShift += RED_BITS;

            dataIndex += BYTES_IN_PIXEL;
        }

        encodedBytes = 0;
        while (encodedBytes < EXTENSION_SIZE)
        {
            checkPadding(&dataIndex, width, padding);

            pData[dataIndex + BLUE]  &= ~BLUE_BITMASK;
            pData[dataIndex + GREEN] &= ~GREEN_BITMASK;
            pData[dataIndex + RED]   &= ~RED_BITMASK;

            /* If there are characters in extension string, copy the next 
             * character to the current pixel */
            if (extension[extensionIndex])
            {    
                pData[dataIndex + BLUE]  |=  extension[extensionIndex]                & BLUE_BITMASK;
                pData[dataIndex + GREEN] |= (extension[extensionIndex] >> BLUE_BITS)  & GREEN_BITMASK;
                pData[dataIndex + RED]   |= (extension[extensionIndex] >> (BLUE_BITS + GREEN_BITS)) & RED_BITMASK;
            }
             
            dataIndex += BYTES_IN_PIXEL;
            extensionIndex++;
            encodedBytes++;

        }

        encodedBytes = 0;
        while (encodedBytes < sizeOfDataToEncode)
        {
            checkPadding(&dataIndex, width, padding);
            
            pData[dataIndex + BLUE]  &= ~BLUE_BITMASK;
            pData[dataIndex + GREEN] &= ~GREEN_BITMASK;
            pData[dataIndex + RED]   &= ~RED_BITMASK;

            pData[dataIndex + BLUE]  |=  *pDataToEncode               & BLUE_BITMASK;
            pData[dataIndex + GREEN] |= (*pDataToEncode >> BLUE_BITS) & GREEN_BITMASK;
            pData[dataIndex + RED]   |= (*pDataToEncode >> (BLUE_BITS + GREEN_BITS))  & RED_BITMASK;

            dataIndex += BYTES_IN_PIXEL;

            pDataToEncode++;
            encodedBytes++;
        }
        
        errRtn = success;
    }

    return errRtn;
}


/** @brief Retrieves the file and info headers from the bitmap file as well as 
 *         how much padding there is at the end of the line and the size of the 
 *         image data in bytes 
 * @param fpBitmap File pointer to bitmap file.
 * @param pFileHeader File Header Structure to be populated.
 * @param pInfoHeader Info Header Structure to be populated.
 * @param pPadding Amount of padding on each line.
 * @param pSizeOfData Size of image data - both pixels and padding.
 * @return An error value from enum eErrors. */
tError parseBitmap(IN FILE * fpBitmap, 
                   OUT tBitmapFileHeader * pFileHeader, 
                   OUT tBitmapInfoHeader * pInfoHeader, 
                   OUT uint8_t * pPadding,
                   OUT uint64_t * pSizeOfData)
{
    uint32_t widthBytes = 0;
    tError errRtn = errorDefault;
    
    if (pFileHeader == NULL || pInfoHeader == NULL || fpBitmap == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if (fseek(fpBitmap, 0, SEEK_SET) != success)
    {
        errRtn = errorFseek;
        ERROR_PRINT(errRtn);
    }

    else if (fread(pFileHeader, sizeof(tBitmapFileHeader), 1, fpBitmap) != 1)
    {
        errRtn = errorFread;
        ERROR_PRINT(errRtn);
    }

    else if (fread(pInfoHeader, sizeof(tBitmapInfoHeader), 1, fpBitmap) != 1)
    {
        errRtn = errorFread;
        ERROR_PRINT(errRtn);
    }

    else
    {
        widthBytes = pInfoHeader->width * BYTES_IN_PIXEL;

        if (widthBytes % 4 != 0)
        {
             widthBytes += 4 - (widthBytes % 4);
        }

        *pPadding =  widthBytes - (pInfoHeader->width * BYTES_IN_PIXEL); 

        *pSizeOfData = widthBytes  * pInfoHeader->height;
    
        errRtn = success;
    }   

    return errRtn;
}


/** @brief Creates memory at pData for the image data and copies across the 
 *         bytes from fpBitmap.
 *  @param fpBitmap File pointer to bitmap file.
 *  @param pData Pointer to data read from bitmap.
 *  @param dataSize Size of data to read from bitmap.
 *  @return An error value from enum eErrors. */
tError copyBitmapData(IN FILE * fpBitmap, OUT uint8_t ** pData, uint64_t dataSize)
{
    tError errRtn = errorDefault;

    if ((*pData = malloc(dataSize)) == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if (fseek(fpBitmap, sizeof(tBitmapInfoHeader) + sizeof(tBitmapFileHeader),
             SEEK_SET) != success)
    {
        errRtn = errorFseek;
        ERROR_ERRNO_PRINT(errRtn);
    }

    else if (fread(*pData, sizeof(uint8_t), dataSize, fpBitmap) != dataSize)
    {
        errRtn = errorFread;
        ERROR_ERRNO_PRINT(errRtn);
    }
    
    else if (*pData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }
    
    else
    {
        errRtn = success;
    }

    return errRtn;
}


/** @brief Creates the output file for the decoded data. The output file retains
 *         its original extension.
 *  @param extension The extension of the hidden data / the extension of the 
 *        output file.
 *  @param pFileData Pointer to the decoded data to be written to the file.
 *  @param dataSizeBytes The size of the decoded data.
 *  @return An error value from enum eErrors. */
tError createOutputFile(IN char * extension, 
                        IN uint8_t * pFileData,
                        uint64_t dataSizeBytes)
{
    tError errRtn = errorDefault;
    FILE * fpOutput = NULL;
    char outputFileNameAndExt[OUTPUT_NAME_SIZE] = "decoded";
    
    if (extension != NULL)
    {
        if (strlen(extension))
        {
            snprintf(outputFileNameAndExt, OUTPUT_NAME_SIZE, "decoded.%s", extension);
        }
    }

    if (pFileData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if ((fpOutput = fopen(outputFileNameAndExt, "wb")) == NULL)
    {
        errRtn = errorFopen;
        ERROR_ERRNO_PRINT(errRtn);
    }

    else if (fwrite(pFileData, dataSizeBytes, 1, fpOutput) != 1)
    {
        errRtn = errorFwrite;
        ERROR_ERRNO_PRINT(errRtn); 
    }
    
    else
    {
        errRtn = success;
    }
    
    if (fpOutput != NULL)
    {
        if (fclose(fpOutput) != success)
        {
            errRtn = errorFclose;
            ERROR_ERRNO_PRINT(errRtn);
        }   
    }
    return errRtn;
}


/** @brief Decodes the hidden data from the bitmap.
 *  @param encodedData Pointer to the bitmap.
 *  @param encodedDataSize The size of the bitmap
 *  @param startOfEncodedDataIndex The index which the image data begins.
 *  @param width The width of the bitmap. Required for padding calculations.
 *  @param padding The size of the line padding.
 *  @param decodedData Pointer to the memory where the decoded data is to be 
 *         stored.
 *  @return An error value from enum eErrors. */
tError decodeData(IN uint8_t * encodedData,
                  uint64_t encodedDataSize, 
                  uint64_t startOfEncodedDataIndex,
                  uint64_t width,
                  uint64_t padding,
                  OUT uint8_t * decodedData)
{
    tError errRtn = errorDefault;
    uint64_t decodedDataIndex = 0;
    uint64_t encodedDataIndex = startOfEncodedDataIndex;

    if (encodedData == NULL || decodedData == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }
    
    else
    {
        while (decodedDataIndex < encodedDataSize)
        {
            checkPadding(&encodedDataIndex, width, padding); 

            decodedData[decodedDataIndex]  =  encodedData[encodedDataIndex + BLUE]  & BLUE_BITMASK;
            decodedData[decodedDataIndex] |= (encodedData[encodedDataIndex + GREEN] & GREEN_BITMASK) << BLUE_BITS;
            decodedData[decodedDataIndex] |= (encodedData[encodedDataIndex + RED]   & RED_BITMASK)   << (BLUE_BITS + GREEN_BITS);
            decodedDataIndex++;
            encodedDataIndex += BYTES_IN_PIXEL;
        }

        errRtn = success;
    }
    return errRtn;
}


/** @brief Parses the bitmap containing hidden information and retrieves the 
 *         index where the image starts, the extension of the hidden data and 
 *         the size of the hidden data.
 *  @param pImageData Pointer to the bitmap.
 *  @param imageDataSize The size of the bitmap image.
 *  @param padding Size of the padding on each "line".
 *  @param width The width of the image.
 *  @param pStartOfEncodedDataIndex The index at which the bitmap image starts 
 *         in pImageData.
 *  @param pExtension Pointer to memory to which will hold the original extension
 *         of the hidden data.
 *  @param pEncodedDataSize The size in bytes of the hidden data.
 *  @return An error value from enum eErrors. */
tError parseEncodedData(IN uint8_t * pImageData, 
                        uint64_t imageDataSize,
                        uint8_t padding,
                        uint64_t width,
                        OUT uint64_t * pStartOfEncodedDataIndex, 
                        OUT char * pExtension, 
                        OUT uint64_t * pEncodedDataSize)
{
    tError errRtn = errorDefault;
    uint64_t dataIndex = 0;
    uint8_t bitShift = 0;
    uint32_t extensionIndex = 0;
    uint64_t decodedBytes = 0;

    if (imageDataSize < (sizeof(tBitmapFileHeader) + sizeof(tBitmapInfoHeader) 
        + DATA_SIZE + EXTENSION_SIZE))
    {
        errRtn = errorSize;
        ERROR_PRINT(errRtn);
    }

    else
    {
        *pEncodedDataSize = 0;

        decodedBytes = 0;
        while (decodedBytes < DATA_SIZE)
        {   
            checkPadding(&dataIndex, width, padding);

            *pEncodedDataSize |= (pImageData[dataIndex + BLUE]  & BLUE_BITMASK)  << bitShift;
            bitShift  += BLUE_BITS; 
            *pEncodedDataSize |= (pImageData[dataIndex + GREEN] & GREEN_BITMASK) << bitShift;
            bitShift  += GREEN_BITS;
            *pEncodedDataSize |= (pImageData[dataIndex + RED]   & RED_BITMASK)   << bitShift;
            bitShift  += RED_BITS;

            dataIndex += BYTES_IN_PIXEL;
            decodedBytes++;
        }
        
        decodedBytes = 0;
        while (decodedBytes < EXTENSION_SIZE)
        {   
            checkPadding(&dataIndex, width, padding);

            pExtension[extensionIndex]  = (pImageData[dataIndex + BLUE]  & BLUE_BITMASK);
            pExtension[extensionIndex] |= (pImageData[dataIndex + GREEN] & GREEN_BITMASK) << BLUE_BITS;
            pExtension[extensionIndex] |= (pImageData[dataIndex + RED]   & RED_BITMASK)   << (BLUE_BITS + GREEN_BITS);
            
            extensionIndex++;
            dataIndex += BYTES_IN_PIXEL;
            decodedBytes++;

        }

        errRtn = success;
    }

    *pStartOfEncodedDataIndex = dataIndex;
    return errRtn;
}


/** @brief Checks the current index against the width of the image to calculate
 *         if image padding needs to be compensated for.
 *  @param pIndex The index to be checked. Incremented by padding when required.
 *  @param width The width of the bitmap image.
 *  @param padding The size of the padding in the bitmap.
 *  @return An error value from enum eErrors. */
tError checkPadding(IN_OUT uint64_t * pIndex, uint64_t width, uint64_t padding)
{
    tError errRtn = errorDefault;

    if (pIndex == NULL)
    {
        errRtn = errorNull;
        ERROR_PRINT(errRtn);
    }

    else if ((*pIndex + 1) % width == 0)
    {
        *pIndex += padding;
    }

    return errRtn;
}


/** @brief A debug function which prints out the elements of 
 *         a tBitmapFileHeader structure.
 *  @param pFileHeader The file header structure to be
 *         printed. */
void printFileHeader(IN const tBitmapFileHeader * pFileHeader)
{
    printf("\nFILE HEADER\n");
    printf("type: %u\n"
           "size: %u\n"
           "offsetbits: %u\n",
    pFileHeader->type,
    pFileHeader->size,
    pFileHeader->offsetbits);
}


/** @brief A debug function which prints out the elements of 
 *         a tBitmapInfoHeader structure.
 *  @param pInfoHeader The info header structure to be
 *         printed. */
void printInfoHeader(IN const tBitmapInfoHeader * pInfoHeader)
{
    printf("\nINFO HEADER\n");
    printf("size: %u\n"
           "width: %u\n"
           "height: %u\n"
           "planes: %u\n"
           "bitsPerPixel: %u\n"
           "compressionType: %u\n"
           "imageDataSize: %u\n"
           "horizontalResolution: %d\n"
           "verticalResolution: %d\n"
           "numberOfColours: %u\n"
           "numberOfImportantColours:%u\n",
    pInfoHeader->size, pInfoHeader->width, pInfoHeader->height, pInfoHeader->planes,
    pInfoHeader->bitsPerPixel,pInfoHeader->compressionType, 
    pInfoHeader->imageDataSize, pInfoHeader->horizontalResolution,
    pInfoHeader->verticalResolution, pInfoHeader->numberOfColours, 
    pInfoHeader->numberOfImportantColours);
}

