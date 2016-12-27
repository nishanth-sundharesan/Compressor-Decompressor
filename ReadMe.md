# Compressor - Decompressor
Compression and Decompression of floating point data.

### Description:

There are two programs:

 - Compressor.c

 - DeCompressor.c

The input to the "Compressor.c" will be a file of Vertices formatted as ID: x y z U V Texture Name R G B

The input file is named as "Verts.txt" and is present in the folder "CompressorDecompressor\AppData"

x,y,z can be floats, any precision

U,V,R,G,B will be between 0.0 and 1.0

Texture name will be a string that is less than 32 characters

An example of the input file looks like:

0: 3.14 5.43 7.28 0.0 0.2 brick.jpg 1.0 1.0 1.0

1: 4.32 5.43 7.69 1.0 1.0 brick1.jpg 1.0 1.0 1.0

…

The "Compressor.c" will output a file of verts x,y,z as a binary data

The binary output will be a bitstream of vert data along with some other additional information that will be needed to read the file and reconstruct the input.

The "DeCompressor.c" will read the vert binary data and recreates the verts input data. The Root Mean Square error of the vert data is then computated.  

The "DeCompressor.c" will then output the RMS error values for bit values between 5 and 16.  

A graph in excel is then created manually for the outputted RMS eror values.
---

#### In summary:

 - The "Compressor.c" writes out the binary vert data as a bit stream.

 - The bit size of each vert data is specified by a user(command line) argument.

 - The "DeCompressor.c" will read the same file and recreate the vert data.

 - The "DeCompressor.c" will then compute the RMS error value of the vert data.

 - Both the programs "Compressor.c" and "DeCompressor.c" are run for bit counts of 5-16 (12 total times). The "DeCompressor.c" plots the RMS error on an excel sheet.

 - The whole process is automated with a batch file that sequentially runs both the programs from the DOS command line.

This was part of the assignment at FIEA.  
---

#### FAQ for the assignment.

1. Do we need to specify the bits for x, y, and z separately?  
 - No. One bit size for all data.

2. Do we have to use every bit in the file, or can we pad?  
 - No padding allowed. Make use of every bit in the stream.

3. What type of data do we put in the output data to help us reconstruct the data?  
 - Be creative, but remember the whole point of this tool is to have the file be as small as possible so try to have as little info as possible to do a great job reconstructing when given adequate bits. As a general rule, your “extra” data should be FAR, FAR, FAR less than the actual data stream – think minimally.

4. What pieces of the vert data do we need to store?  
 - x, y, z

5. So we don’t do anything with the U,V,R,G,B,texture data?  
 - Correct

6. Do we have to create a mechanism to get at the original values to compute the RMS error when unpacking the bitstream?  
 - Yes

7. How do we compute root mean square error?  
 - Look it up.
---

#### The following question was also asked as part of the assignment.

1. If your lead engineer asked you how many bits we should be compressing the data to, what would your answer be?  
 - I would say either 10 or 11 bits, because the data would be accurate upto 2 decimal points. But it all depends on the range of the data. For example, if there is a single large floating point value then the accuracy of the entire data will decrease.

#### To run the program:

1. Execute the "Executable.bat" batch file present in the "CompressorDecompressor\ExecutableFiles" folder.  
2. The "Executable.bat" will run the "Compressor.exe" and "DeCompressor.exe" 12 times. It also passes the size of the vert data to be compressed (5-16) to the "Compressor.exe" as a command line arguement.  
3. The "DeCompressor.c" will generate the file "RMSValues.csv" and it is present in the folder "CompressorDecompressor\AppData" ("CompressedData.bin" and "OriginalData.txt" are the temporary files created and used by the program).  
4. Open the "RMSValues.csv". This will contain the 'Number of bits each floating point value is compressed to', 'RMS Error value' and the 'Compressed file size in bytes'.  
5. Another excel file named "RMSErrorStats.xlsx" is already present in the folder "CompressorDecompressor". This file contains the same exact data as that of the file "RMSValues.csv". It also contains a chart based on the values present to help us analyze the output. This chart was manually added.