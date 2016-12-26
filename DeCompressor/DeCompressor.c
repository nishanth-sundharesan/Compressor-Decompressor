#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Boolean.h"
#include <sys/stat.h>

//Defining file names
#define FILENAME_COMPRESSED_DATA_TO_READ  "../AppData/CompressedData.bin"
#define FILENAME_ORIGINAL_DATA_TO_READ  "../AppData/OriginalData.txt"
#define FILENAME_RMS_VALUE_TO_WRITE	"../AppData/RMSValues.csv"

//Defining macros
#define SCRAP_DATA(data, bits) data >> bits
#define MAKE_SPACE_FOR_REMAINING_DATA(data, bits) data << bits
#define APPEND_REMAINING_DATA(currentData, remainingData) currentData | remainingData

//Defining constants
#define MAX_COMPRESSION_BITS 16
#define NO_OF_BITS "Number Of Bits"
#define RMS_ERROR_VALUE "RMS Error Value"
#define FILE_SIZE "File Size In Bytes"

//Static bit masking data
short bitMasks[MAX_COMPRESSION_BITS + 1] = { 0, 1,3,7,15,31,63,127,255,511, 1023, 2047, 4095, 8191, 16383, 32767, 65535 };
//0=0000, 1 = 0001, 3 = 0011, 7 = 0111 and so on...

//Global variables
double RMSSum = 0;
int dataCount = 0;																									//The number of values to decompress from the binary file

//Function declarations/prototyping
int SavePreviousData(int currentData, int bitsToSave);
void DecompressAndCalculateRMS(double minValue, unsigned short value, double quantumLength, FILE* originalDataToRead);

int main(int argc, char* argv[])
{
	/************************ Declaring file pointers ************************/
	FILE *compressedDataToRead = NULL;
	FILE *originalDataToRead = NULL;
	FILE *rmsDataToWrite = NULL;
	/************************************************************************/

	//Checking if the RMS Stats file exists or not	
	struct stat buffer;
	bool isFilePresent = (stat(FILENAME_RMS_VALUE_TO_WRITE, &buffer) == 0) ? true : false;
	
	/*********** Opening the files to read and write ************************/
	if (fopen_s(&compressedDataToRead, FILENAME_COMPRESSED_DATA_TO_READ, "rb") != 0)
	{
		printf("Unable to open the file %s", FILENAME_COMPRESSED_DATA_TO_READ);
		return 0;
	}
	if (fopen_s(&originalDataToRead, FILENAME_ORIGINAL_DATA_TO_READ, "r") != 0)
	{
		printf("Unable to open the file %s", FILENAME_ORIGINAL_DATA_TO_READ);
		return 0;
	}
	if (fopen_s(&rmsDataToWrite, FILENAME_RMS_VALUE_TO_WRITE, "a+") != 0)
	{
		printf("Unable to open the file %s", FILENAME_RMS_VALUE_TO_WRITE);
		return 0;
	}
	/************************************************************************/

	double minValue = 0;
	double maxValue = 0;
	unsigned short compressionBitLength = 0;																		//The number of bits used to compress each value
	/******* Reading the minimum, maximum, compressionBitLength & the number of values to decompress ******/
	fread(&minValue, sizeof(double), 1, compressedDataToRead);
	fread(&maxValue, sizeof(double), 1, compressedDataToRead);
	fread(&compressionBitLength, sizeof(unsigned short), 1, compressedDataToRead);
	fread(&dataCount, sizeof(int), 1, compressedDataToRead);														//dataCount is declared as a global variable
	/****************************************************************************************************/

	//Calculating the segmentLength or the total number of values that can fit within the compressionBitLength
	unsigned int segmentLength = (unsigned int)pow(2, compressionBitLength);
	//Calculating the quantumLength or bucketLength based on the minimum and maximum values
	double quantumLength = (maxValue - minValue) / (segmentLength - 1);
	int savedDataCount = dataCount;

	unsigned short currentData = 0;																					//The current value read from the bit stream
	unsigned short binaryInputLength = MAX_COMPRESSION_BITS;														//The max number of bits that will be used for compression
	unsigned short bitsToScrap = binaryInputLength;																	//The number of bits to scrap from the current read value
	unsigned short previousData = 0;																				//The bits saved from the current value which will be used to calculate a complete value in the next iteration
	unsigned short missingBits = 0;																					//The number of bits missing in the current data
	unsigned short tempValue = 0;																					//A temporary variable																							

	/************** Reading the bit stream and decompressing **************/
	while (dataCount > 0)
	{
		fread(&currentData, sizeof(short), 1, compressedDataToRead);												//Reading from the binary file into currentData
		if (missingBits != 0)
		{
			bitsToScrap = binaryInputLength - missingBits;
			tempValue = currentData;																				//Save the newly read value in a temporary variable

			currentData = MAKE_SPACE_FOR_REMAINING_DATA(previousData, missingBits);									//Left shift the saved data with the missing bits to make space for the remaining bits
			previousData = SavePreviousData(tempValue, bitsToScrap);												//AND the newly read value using the masking bits to save the upcoming value
			tempValue = SCRAP_DATA(tempValue, bitsToScrap);															//Now scrap or right shift the newly read data to get the remaining bits of the current value
			currentData = APPEND_REMAINING_DATA(currentData, tempValue);											//OR both the variables to get the appropriate value
			DecompressAndCalculateRMS(minValue, currentData, quantumLength, originalDataToRead);					//Decompress the acquired appropriate value and calculate the RMS error
			currentData = previousData;
		}
		while (bitsToScrap > compressionBitLength)																	
		{
			bitsToScrap = bitsToScrap - compressionBitLength;														
			previousData = SavePreviousData(currentData, bitsToScrap);												//AND the current data using the masking bits to save the upcoming value
			currentData = SCRAP_DATA(currentData, bitsToScrap);														//Now scrap or right shift the current data by the same number of bits to get the appropriate value
			DecompressAndCalculateRMS(minValue, currentData, quantumLength, originalDataToRead);					//Decompress the extracted or appropriate value and calculate the RMS error
			currentData = previousData;																				//Assign the saved data to the current data
		}
		missingBits = compressionBitLength - bitsToScrap;
		if (missingBits == 0)																						//This condition succeeds if there are no more missing bits in the value
		{
			previousData = bitsToScrap == compressionBitLength ? currentData : previousData;						//This condition succeeds only when the compressionBitLength = MAX_COMPRESSION_BITS
			DecompressAndCalculateRMS(minValue, previousData, quantumLength, originalDataToRead);					//The appropriate bits are read and the data is ready to be decompressed. Decompress and calculate the RMS error

			previousData = 0;																						//Resetting all the variables and preparing for the next iteration
			missingBits = 0;																						
			bitsToScrap = binaryInputLength;
		}
	}
	/************************************************************************/

	//Printing the headings if the file doesn't exist
	if (!isFilePresent)
	{		
		fprintf_s(rmsDataToWrite, "%s,%s,%s", NO_OF_BITS, RMS_ERROR_VALUE, FILE_SIZE);
	}
		
	fseek(compressedDataToRead, 0, SEEK_END);																		//Moving the file pointer to the end of the file to calculate the file size
	//Printing out the Number Of Bits, RMS Error Value and the File Size
	fprintf_s(rmsDataToWrite, "\n%lu,%f,%lu", compressionBitLength, sqrtl(RMSSum / savedDataCount), (unsigned long)ftell(compressedDataToRead));

	/****** Closing the file pointers *******/
	fclose(compressedDataToRead);
	fclose(originalDataToRead);
	fclose(rmsDataToWrite);
	/****************************************/
	return 1;
}

/*
*	Function: SavePreviousData
*	-----------------------------------
*	Description: Reads the current data and the number of bits to be saved in the current data .
*				 ANDs the current data with the help of bitMasks to generate the saved data
*	Parameters:
*				currentData = the data to be saved
*				bitsToSave = the number of bits in the currentData to be saved
*	Returns: Returns the saved data.
*/
int SavePreviousData(int currentData, int bitsToSave)
{
	return currentData & bitMasks[bitsToSave];
}/*

/*	Function: DecompressAndCalculateRMS
*	-----------------------------------
*	Description: Reads the extracted compressed value and decompressed it.				 
*	Parameters:
*				minValue = the minimum value of the data range
*				compressedValue = the compressed extracted value from the bit stream
*				quantumLength = quantumLength or bucketLength which is calculated based on the min and max value of the data range
*				originalDataToRead = file pointer to the original data file
*/
void DecompressAndCalculateRMS(double minValue, unsigned short compressedValue, double quantumLength, FILE* originalDataToRead)
{
	if (dataCount != 0)
	{
		double originalValue, decompressedValue, rmsValue;															
		fscanf_s(originalDataToRead, "%lf", &originalValue);														//Read the original value which will be used to calculate RMS

		decompressedValue = minValue + (compressedValue * quantumLength);											//Get the decompressed value
		//Calculating the RMS Error
		rmsValue = originalValue - decompressedValue;																//Get the difference between the original value and the decompressed value
		rmsValue *= rmsValue;																						//Square the difference
		RMSSum += rmsValue;																							//Add it to the RMS Sum
		dataCount--;
	}
}