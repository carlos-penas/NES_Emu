#ifndef CONSTANTS_H
#define CONSTANTS_H

//***VIDEO SETTINGS***

//Resolution
const unsigned int PICTURE_WIDTH = 256;
const unsigned int PICTURE_HEIGHT = 240;

//Pixel size
const unsigned char PIXEL_SIZE = 4;         //Each pixel takes up 4 bytes {R,G,B,A}

//Resolution multiplyer
const unsigned char RES_MULTIPLYER = 5;     //Factor to multiply the original resolution


//Pattern Table
const unsigned int PT_TABLE_WIDTH = 256;    //Pattern Table Resolution
const unsigned int PT_TABLE_HEIGHT = 128;   //

const int PT_TABLE_COLS = 32;               //Size of the Pattern Table to display
const int PT_TABLE_ROWS = 16;               //

//Pattern
const unsigned char PTRN_LNGHT = 8;         //Pattern size = 8x8 pixels
const unsigned char PTRN_SIZE = 16;         //Each pattern takes up 16 bytes of cartridge memory


#endif // CONSTANTS_H
