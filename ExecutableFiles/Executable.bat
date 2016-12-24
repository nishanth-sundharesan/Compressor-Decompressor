del ..\AppData\RMSValues.csv

for /L %%n in (5,1,16) do (
 Compressor.exe  %%n 
 DeCompressor.exe
)