/********************************************************************/
/*																	*/
/*						Portable MK14 emulator in 'C'				*/
/*																	*/
/*						   Interface Console & CPU					*/
/*																	*/
/********************************************************************/
/*																	*/
/*	This section should remain unchanged. Only the DRV_xxx.C files	*/
/*	should change for each target platform							*/
/*																	*/
/********************************************************************/

#include "scmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static void RDump(char *,int,int);
static void Dump(void);

void main(int argc,char *argv[])
{
int n;
char Cmd;
CONInitialise();						/* Set up and titles */
InitialiseDisplay();					/* Initialise the display */
LoadROM();								/* Load the ROM */
ResetCPU();								/* Reset the CPU */
if (argc == 2) LoadObject(argv[1]);		/* Load file */
OutStr("Version 1.0 (15-May-98)\r");	/* Version information */
do										/* Main loop */
	{
	CONWrite(']');						/* Prompt & Get command */
	Cmd = CONRead();Cmd = toupper(Cmd);
	CONWrite(Cmd);CONWrite('\r');
	switch(Cmd)							/* Do the command */
		{
		case 'G':						/* Run program */
			while (CONKeyPressed(KEY_BREAK) == 0)
				BlockExecute();
			Dump();						/* Status dump */
			break;
		case 'R':             			/* Reset */
			ResetCPU();InitialiseDisplay();break;
		case 'S':						/* Single Step */
			Execute(1);Dump();break;
		case 'C':						/* CPU Register Dump */
			Dump();break;
		case '?':						/* Help */
		case 'H':
			OutStr("[G]o [D]ump [R]eset [S]tep [R]eset [C]pu Status [/]Break [Q] Quit\r");
			break;
		}
	}
while (Cmd != 'Q');
CONTerminate();							/* Tidy up - we've finished */
}

/********************************************************************/
/*							Output a string							*/
/********************************************************************/

void OutStr(char *Text)
{
while (*Text != '\0') CONWrite(*Text++);
}

/********************************************************************/
/*							Dump registers							*/
/********************************************************************/

static void Dump(void)
{
RDump("A",Acc,2);RDump("E",Ext,2);RDump("S",Stat,2);
RDump("P0",Ptr[0],4);RDump("P1",Ptr[1],4);
RDump("P2",Ptr[2],4);RDump("P3",Ptr[3],4);
CONWrite('-');CONWrite(' ');
RDump(NULL,ReadMemory(ADD12(Ptr[0],1)),2);
RDump(NULL,ReadMemory(ADD12(Ptr[0],2)),2);
CONWrite('\r');
}

/********************************************************************/
/*						Register Dump Stuff							*/
/********************************************************************/

static void RDump(char *Title,int Data,int Width)
{
char Temp[8];
if (Title != NULL)
	{
	OutStr(Title);
	CONWrite(':');
	}
sprintf(Temp,"%0*X ",Width,Data);
OutStr(Temp);
}

