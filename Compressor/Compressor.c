/*
*	The following program is used to compress a floating point number by some number of bits. This number of bits is taken as an input from the command line.
*	It then prints the compressed value to a binary file in a bit stream format.
*	Logic for compressing the data:
*		1. We first read all the floating point values present in the dataset and calculate the minimum and maximum values of the dataset.
*		2. We know that the floating point values should be compressed by some number of bits from the input. Let's call it compressionBitLength.
*		3. So the total values that can be generated from the compressionBitLength is 2^compressionBitLength. i.e 0 to (2^compressionBitLength) - 1.
*		4. Based on the minimum and maximum values of the data set, we will represent the floating point values as an integer value between 0 and (2^compressionBitLength) - 1.
*		5. Write those integeral representation of the floating point values as a bit stream to the binary file.
*/

#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <math.h>

//Defining file names
#define FILENAME_VERTS_TO_READ  "../AppData/Verts.txt"
#define FILENAME_ORIGINAL_DATA_TO_WRITE  "../AppData/OriginalData.txt"
#define FILENAME_COMPRESSED_DATA_TO_WRITE  "../AppData/CompressedData.bin"

//Defining constants
#define VALUES_IN_A_LINE 3
#define MAX_LINE_LENGTH 200
#define MAX_COMPRESSION_BITS 16

//Defining Macros
#define LEFT_SHIFT_BY_BITS(data, bits) data << bits
#define RIGHT_SHIFT_BY_BITS(data, bits) data >> bits
#define APPEND_DATA(dataPart1, dataPart2) dataPart1 | dataPart2

//Function declarations/prototyping
void OutputValue(unsigned short printablenValue, FILE *compressedDataToWrite);

int main(int argc, char* argv[])
{
	/************************ Declaring file pointers ************************/
	FILE *vertsDataToRead = NULL;
	FILE *originalDataToWrite = NULL;
	FILE *compressedDataToWrite = NULL;	
	/************************************************************************/
	
	/*********** Opening the files to read and write ************************/
	if (fopen_s(&vertsDataToRead, FILENAME_VERTS_TO_READ, "r") != 0)
	{
		printf("Unable to open the file %s", FILENAME_VERTS_TO_READ);
		return 0;
	}
	if (fopen_s(&originalDataToWrite, FILENAME_ORIGINAL_DATA_TO_WRITE, "w+") != 0)
	{
		printf("Unable to open the file %s", FILENAME_ORIGINAL_DATA_TO_WRITE);
		return 0;
	}
	if (fopen_s(&compressedDataToWrite, FILENAME_COMPRESSED_DATA_TO_WRITE, "wb") != 0)
	{
		printf("Unable to open the file %s", FILENAME_COMPRESSED_DATA_TO_WRITE);
		return 0;
	}	
	/************************************************************************/
		
	char inputLine[MAX_LINE_LENGTH];
	const char* formatString[VALUES_IN_A_LINE] = { "%*s %lf","%*s %*s %lf","%*s %*s %*s %lf" };
	unsigned short compressionBitLength = argv[1] == NULL ? 5 : atoi(argv[1]);								//Reading the compression bit length from the command prompt
	double minValue = DBL_MAX;																				//Declaring and initializing the minimum and maximum values of the segment
	double maxValue = DBL_MIN;
	double inputValue = 0.0;																				
	double quantumLength = 0.0;
	unsigned int segmentLength = (unsigned int)pow(2, compressionBitLength);								//Calculating the segmentLength or the total number of values that can fit within the compressionBitLength
	unsigned int dataCount = 0;

	/******* Calculating the minimum and maximum values of the data set ******/
	while (fgets(inputLine, MAX_LINE_LENGTH, vertsDataToRead) != NULL)
	{
		for (int i = 0; i < VALUES_IN_A_LINE; i++)
		{
			sscanf_s(inputLine, formatString[i], &inputValue);
			minValue = min(minValue, inputValue);
			maxValue = max(maxValue, inputValue);
			fprintf_s(originalDataToWrite, "%.10g ", inputValue);											//Save the data read into a separate file. The data in this file will be used again in both compressing & decompressing.
			dataCount++;																					//Calculating the number of values to compress and decompress
		}
		fprintf_s(originalDataToWrite, "\n");
	}
	/************************************************************************/
	quantumLength = (maxValue - minValue) / (segmentLength - 1);											//Calculating the quantumLength or bucketLength based on the minimum and maximum values

	/******* Writing the minimum, maximum, compressionBitLength & the number of values in the dataset to the compressed binary file ******/
	fwrite(&minValue, sizeof(double), 1, compressedDataToWrite);
	fwrite(&maxValue, sizeof(double), 1, compressedDataToWrite);
	fwrite(&compressionBitLength, sizeof(unsigned short), 1, compressedDataToWrite);
	fwrite(&dataCount, sizeof(int), 1, compressedDataToWrite);
	/*************************************************************************************************************************************/

	rewind(originalDataToWrite);																			//Setting the file pointer to the beginning of the file
	unsigned short binaryOutputLength = MAX_COMPRESSION_BITS;												//The max number of bits that will be used for compression
	unsigned short remainingBits = binaryOutputLength;														//The number of bits remaining in the current data, some number of bits from the upcoming data will be appended to the current data
	unsigned short printableValue = 0;																		//The completely filled bits of the current data
	unsigned short tempValue = 0;																			//A temporary variable
	unsigned short quantumNumber = 0;																		

	while (fscanf_s(originalDataToWrite, "%lf", &inputValue) != EOF)
	{
		quantumNumber = (unsigned int)round(((inputValue - minValue) / quantumLength));						//Setting a quantum number for each inputted value between 0 to segmentLength	

		//Writing the bit stream
		if (remainingBits >= compressionBitLength)
		{
			tempValue = LEFT_SHIFT_BY_BITS(quantumNumber, (remainingBits - compressionBitLength));			//Left shift the value to make space for the upcoming value
			printableValue = APPEND_DATA(printableValue, tempValue);										//OR the previous data or merge the previous data with the new data to get the printable value
			remainingBits = (remainingBits - compressionBitLength);
			if (remainingBits == 0)
			{
				OutputValue(printableValue, compressedDataToWrite);											//If there are no remaining bits, then output the value to the binary file
				printableValue = 0;																			//Reset the variables and get ready for the next iteration
				remainingBits = binaryOutputLength;
			}
		}
		else
		{
			tempValue = RIGHT_SHIFT_BY_BITS(quantumNumber, (compressionBitLength - remainingBits));			//Right shift or scrap the currently read data by some number of bits. It will then be used for merging with the previously saved data.
			printableValue = APPEND_DATA(printableValue, tempValue);										//OR the previous data or merge the previous data with the new data to get the printable value
			OutputValue(printableValue, compressedDataToWrite);												//Output the printable value to the binary file
			printableValue = LEFT_SHIFT_BY_BITS(quantumNumber, (binaryOutputLength - (compressionBitLength - remainingBits)));	//Make space or left shift by the number of bits for the upcoming value
			remainingBits = binaryOutputLength - (compressionBitLength - remainingBits);
		}
	}
	OutputValue(printableValue, compressedDataToWrite);														//Output the left out value to the binary file

	/****** Closing the file pointers *******/
	fclose(vertsDataToRead);
	fclose(originalDataToWrite);
	fclose(compressedDataToWrite);
	/****************************************/
	return 1;
}

/*
*	Function:OutputValue
*	-----------------------------------
*	Description: Writes an unsigned short value into the binary file
*	Parameters:
*				printableValue = the compressed value to be outputted to the binary file
*				compressedDataToWrite = the file pointer of the binary file
*/
void OutputValue(unsigned short printableValue, FILE *compressedDataToWrite)
{
	fwrite(&printableValue, sizeof(unsigned short), 1, compressedDataToWrite);
}