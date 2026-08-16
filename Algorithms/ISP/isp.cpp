/*

    Approach
    ========

    1. Write a minimum ISP for Sony IMX477 
        1. Width = 4032
        2. Height = 3040
        3. RAW 10
        4. Bayer Pattern - RGGB

    To achieve Point 1
        1. read the image.

    2. Steps for sub point 1
        1. ls -lh frame.raw 
        2. stat -f %z frame.raw -> tells us the size of the raw image in bytes which shows 24514560
        3. 4032 * 3040 * 16 bits (if unpacked 16 bit) = 4032 * 3040 * 2 bytes = 24514560
        4. read the data row by row and each data is 10 bit data stored inside a 16 bit integer.  
*/

#include <string>
¬
void readRawFrame(const )