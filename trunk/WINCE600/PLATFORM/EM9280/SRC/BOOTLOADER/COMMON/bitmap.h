#ifndef _BITMAP_H_
#define _BITMAP_H_

/**** BMP file header structure ****/
typedef struct  tagBITMAPFILEHEADER
{
	unsigned int   bfType;           /* Magic number for file */
	unsigned long  bfSize;           /* Size of file */
	unsigned int   bfReserved1;      /* Reserved */
	unsigned int   bfReserved2;      /* ... */
	unsigned long  bfOffBits;        /* Offset to bitmap data */
} BITMAPFILEHEADER;					 // size = 14 bytes

#define BF_TYPE		0x4D42             /* "MB" */

/**** BMP file info structure ****/
typedef struct tagBITMAPINFOHEADER
{
	unsigned long  biSize;           /* Size of info header */
	long           biWidth;          /* Width of image */
	long           biHeight;         /* Height of image */
	unsigned int   biPlanes;         /* Number of color planes */
	unsigned int   biBitCount;       /* Number of bits per pixel */
	unsigned long  biCompression;    /* Type of compression to use */
	unsigned long  biSizeImage;      /* Size of image data */
	long           biXPelsPerMeter;  /* X pixels per meter */
	long           biYPelsPerMeter;  /* Y pixels per meter */
	unsigned long  biClrUsed;        /* Number of colors used */
	unsigned long  biClrImportant;   /* Number of important colors */
} BITMAPINFOHEADER;					 // size = 40 bytes

#define BI_RGB			0             /* No compression - straight BGR data */
#define BI_RLE8			1             /* 8-bit run-length compression */
#define BI_RLE4			2             /* 4-bit run-length compression */
#define BI_BITFIELDS	3             /* RGB bitmap with RGB masks */

/**** Colormap entry structure ****/
typedef struct tagRGBQUAD
{
	unsigned char  rgbBlue;          /* Blue value */
	unsigned char  rgbGreen;         /* Green value */
	unsigned char  rgbRed;           /* Red value */
	unsigned char  rgbReserved;      /* Reserved */
} RGBQUAD;

/*////////////////////////////////////
	bitmap file data struct

struct BITMAPFILEHEADER  bmfh;
struct BITMAPINFOHEADER  bmih;
struct RGBQUAD           aColors[];
unsigned char            abmBits[];

////////////////////////////////////*/
#endif /* !_BITMAP_H_ */

