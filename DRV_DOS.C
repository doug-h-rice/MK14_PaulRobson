/********************************************************************/
/*																	*/
/*						Portable MK14 emulator in 'C'				*/
/*																	*/
/*							DOS Console Interface					*/
/*																	*/
/********************************************************************/
/*	By defining "GFX" this can be a graphical version (requires 	*/
/*  EGAVGA.BGI to operate											*/
/********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#ifdef GFX
#include <graphics.h>
#endif
#include "scmp.h"

static void Vertical(int x,int y,int Colour);
static void Horizontal(int x,int y,int Colour);
static void LLine(int,int,int,int,int);

typedef struct _LEDRECT
{
int left,top,right,bottom,sx,cy;
} LEDRECT;

int ConX = 0;        					/* Console horizontal position */
char CurrentKey = ' ';					/* Current key being pressed */
clock_t KeyTime = 0;					/* Time that key was pressed */
void *ScBuffer;							/* Buffer for graphics sccopy */
int  CurrentKeyID = -1;					/* Current key pressed */
int  KeyPressMatrix[32];				/* Matrix of key presses */
LEDRECT rcLED[DIGITS];					/* LED rectangles */

/********************************************************************/
/* 		Initialise Keyboard & Display, Open Windows etc.			*/
/********************************************************************/

void CONInitialise(void)
{
char *p;
int i,x,GDriver,GMode;
p = " Portable MK14 Emulator in 'C' (MSDOS version)";

#ifdef GFX
GDriver = EGA;GMode = EGAHI;			/* Graphical version */
initgraph(&GDriver,&GMode,".");			/* Set up the BGI Driver */
if (graphresult() != grOk) exit(0);
setcolor(YELLOW);
outtextxy(320-textwidth(p)/2,8,p);
setcolor(WHITE);
rectangle(0,0,639,349);
ScBuffer = (void *)						/* Space for scrolling buffer */
				malloc(imagesize(0,100,640,360));
if (ScBuffer == NULL) exit(0);
x = 480/DIGITS;    						/* Work out where the LEDs are */
for (i = 0;i < DIGITS;i++)
	{
	rcLED[i].sx = x / 8;
	rcLED[i].top = 36;rcLED[i].bottom = 120;
	rcLED[i].left = i * x + 80;
	rcLED[i].right = rcLED[i].left + x - rcLED[i].sx*2;
	rcLED[i].cy = (rcLED[i].top+rcLED[i].bottom)/2;
	}
#else
textattr(0x7);clrscr();					/* Non graphical version */
textcolor(YELLOW);textbackground(RED);
gotoxy(40-strlen(p)/2,1);cputs(p);
#endif

ConX = 0;CurrentKey = ' ';KeyTime = 0;	/* Initialise everything */
CurrentKeyID = 0;
for (i = 0;i < 32;i++)
			KeyPressMatrix[i] = 0;
										/* Personal prompt */
p = "Mk14 'C' Emulator & DOS Port by Paul Robson.\r";
while (*p != '\0') CONWrite(*p++);
}

/********************************************************************/
/*				Tidy up everything ready for exit					*/
/********************************************************************/

void CONTerminate(void)
{
#ifdef GFX
closegraph();							/* Graphics version */
free(ScBuffer);
#else
textattr(0x7);							/* Text version */
clrscr();
#endif
}

/********************************************************************/
/*			  Write a character to the information console			*/
/********************************************************************/

void CONWrite(char cOut)
{
char szTemp[2];

if (cOut == 8 && ConX > 0)				/* Handle Backspace */
	{
	ConX--;
	gotoxy(ConX+5,23);
	textattr(CYAN);putch(' ');
	}
if (cOut == 13 || ConX >= 70)			/* Handle Return */
	{
	ConX = 0;
	#ifdef GFX
	getimage(10,160,629,346,ScBuffer);	/* Scrolling graphics */
	putimage(10,150,ScBuffer,COPY_PUT);
	#else
	movetext(1,14,78,24,1,13);			/* Scrolling text */
	#endif
	}
if (cOut >= ' ')						/* Handle normal characters */
	{
	#ifdef GFX
	szTemp[0] = cOut;szTemp[1] = '\0';
	setcolor(CYAN);
	outtextxy((ConX+5)*8,320,szTemp);
	#else
	textattr(CYAN);
	gotoxy(ConX+5,23);putch(cOut);
	#endif
	ConX++;
	}
#ifndef GFX
gotoxy(ConX+5,23);						/* Position the DOS Cursor */
#endif
}

/********************************************************************/
/*		Read a Key. Characters 8 (BS) and 13 (CR) als supported		*/
/********************************************************************/

char CONRead(void)
{
char c;
while(kbhit() != 0) getch();			/* Flush the input buffer */
do
	{
	c = getch();						/* Read a character */
	if (c == 0x00)						/* Ignore controls */
		{ c = getch(),c = NULL; }
	if (c & 0x80) c = NULL;				/* Ignore 80-FF */
	if (c < ' ' &&						/* Unwanted control characters */
			c != 13 && c != 8) c = NULL;
	if (c == 0x7F) c = 0x08;			/* BS == Delete */
	}
while (c == NULL);
return(c);
}

/********************************************************************/
/*					Check to see if a key is pressed				*/
/*	This is a huge fudge because bloody DOS has no way of telling	*/
/*	whether or not a key is pressed without pulling the INT 9 		*/
/*	routine to tiny bits.											*/
/********************************************************************/

int CONKeyPressed(int KeyID)
{
int Test = -1;

while (kbhit())							/* new Key press ? */
	{
	KeyPressMatrix[CurrentKeyID] = 0;
	CurrentKey = getch();				/* Get which key it was */
	CurrentKey = toupper(CurrentKey);
	KeyTime = clock();					/* This is when we got it... */

	CurrentKeyID = -1;					/* Find out which key it is */
	if (isdigit(CurrentKey))
			CurrentKeyID = CurrentKey-'0';
	if (CurrentKey >= 'A' && CurrentKey <= 'F')
			CurrentKeyID = CurrentKey-'A'+10;
	if (CurrentKey == 'T')
			CurrentKeyID = KEY_TERM;
	if (CurrentKey == 'M')
			CurrentKeyID = KEY_MEM;
	if (CurrentKey == 'Z')
			CurrentKeyID = KEY_ABORT;
	if (CurrentKey == 'G')
			CurrentKeyID = KEY_GO;
	if (CurrentKey == '/')
			CurrentKeyID = KEY_BREAK;
	if (CurrentKey == 'T')
			CurrentKeyID = KEY_TERM;
	if (CurrentKey == 'R')
			CurrentKeyID = KEY_RESET;
	if (CurrentKeyID >= 0)
		KeyPressMatrix[CurrentKeyID] = 1;
	else
		CurrentKeyID = 0;
	}

if (CurrentKey != ' ')					/* auto release after 2 ticks */
	{
	if (clock() > KeyTime+4)
		{
		CurrentKey = ' ';
		KeyPressMatrix[CurrentKeyID] = 0;
		}
	}

return(KeyPressMatrix[KeyID] != 0);
}

/********************************************************************/
/*		   Synchronise the emulated and real CPU Speeds				*/
/********************************************************************/

clock_t	LastClock;
long CycCount = 0L;

void CONSynchronise(long AddCycles)
{
clock_t c;

CycCount = CycCount+AddCycles;

while (CycCount > 60439L)
	{
	CycCount = CycCount - 60439L;
	while (c = clock(), c == LastClock) {}
	LastClock = c;
	}
}

#ifdef GFX
/********************************************************************/
/*				Draw a LED pattern on the screen					*/
/********************************************************************/

void CONDrawLED(int Digit,int BitPattern)
{
LEDRECT lr;
int x;
lr = rcLED[Digit];x = lr.sx;
setlinestyle(SOLID_LINE,0xFF,3);
LLine(BitPattern & 0x01,lr.left+x*2,lr.top,lr.right+x*2,lr.top);
LLine(BitPattern & 0x02,lr.right+x*2,lr.top,lr.right+x,lr.cy);
LLine(BitPattern & 0x04,lr.right+x,lr.cy,lr.right,lr.bottom);
LLine(BitPattern & 0x08,lr.left,lr.bottom,lr.right,lr.bottom);
LLine(BitPattern & 0x10,lr.left,lr.bottom,lr.left+x,lr.cy);
LLine(BitPattern & 0x20,lr.left+x,lr.cy,lr.left+x*2,lr.top);
LLine(BitPattern & 0x40,lr.left+x,lr.cy,lr.right+x,lr.cy);
LLine(BitPattern & 0x80,lr.right+x-2,lr.bottom,lr.right+x+1,lr.bottom);
}

static void LLine(int Test,int x1,int y1,int x2,int y2)
{
setcolor(Test == 0 ? BLACK : GREEN);
line(x1,y1,x2,y2);
}

#else
/********************************************************************/
/*				Draw a LED pattern on the screen					*/
/********************************************************************/

#define XLED	(5)						/* Segment width */
#define YLED	(3)						/* Segment height */

#define FGR		(0x02)
#define BGR		(0x00)

void CONDrawLED(int Digit,int BitPattern)
{
int x,y;
x = Digit*(XLED+3)+41-DIGITS*(XLED+3)/2;/* Work out position */
y = 4;
Horizontal(x,y-1,BitPattern & 0x01);
Vertical(x+XLED+1,y,BitPattern & 0x02);
Vertical(x+XLED+1,y+YLED+1,BitPattern & 0x04);
Horizontal(x,y+YLED*2+1,BitPattern & 0x08);
Vertical(x,y+YLED+1,BitPattern & 0x10);
Vertical(x,y,BitPattern & 0x20);
Horizontal(x,y+YLED,BitPattern & 0x40);
textbackground((BitPattern & 0x80) == 0 ? BGR : FGR);
gotoxy(x+XLED+2,y+YLED*2+1);putch(' ');
gotoxy(ConX+5,23);						/* Position the DOS Cursor */
}

/********************************************************************/
/*						LED Drawing subfunctions					*/
/********************************************************************/

static void Vertical(int x,int y,int Colour)
{
int n;
textbackground(Colour == 0 ? BGR : FGR);
for (n = 0;n < YLED;n++)
	{ gotoxy(x,y+n);putch(' ');	}
}

static void Horizontal(int x,int y,int Colour)
{
int n;
textbackground(Colour == 0 ? BGR : FGR);
for (n = 1;n <= XLED;n++)
	{ gotoxy(x+n,y);putch(' ');	}
}
#endif




