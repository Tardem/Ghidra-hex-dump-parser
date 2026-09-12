A simple parser written in C.
It is designed to analyze assembly dumps from Ghidra and supports either retaining only the hexadecimal code
or transforming the resulting sequence into ASCII characters.
-s - print at screen
-f - print in the file
-sf print at screen and in the file
-a - convert to ascii
example: ./parser -sf encode.txt decode.txt
