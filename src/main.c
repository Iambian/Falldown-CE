/*
 *--------------------------------------
 * Program Name:
 * Author:
 * License:
 * Description:
 *--------------------------------------
*/

#define VERSION_INFO "v0.1"
#define MENU_SELECTED_COLOR 0x4F
#define MENU_NORMAL_COLOR 0x00

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Standard headers (recommended) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External library headers */
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>

/* Put your function prototypes here */
extern int startGame(uint8_t speed);
void keywait();
void keywaitrelease();
void centerstr(char* s,uint8_t y);
void initgfx();

/* Put all your globals here */
struct { int score[3]; uint8_t speed; } file;
char* filename = "FALLDDAT";
char* titlemenu[] = {"Start Game","Change Speed","Quit Game"};
char* speedlabels[] = {"Fast","Medium","Slow"};

void main(void) {
	uint8_t i,y,menuopt;
	kb_key_t k;
	ti_var_t slot;
	int unsigned curscore;
	
	/* Initialize system */
	initgfx();
	
	/* Initialize variables */
	memset(&file,0,sizeof(file));
	file.speed = 2;
	ti_CloseAll();
	if (slot = ti_Open(filename,"r")) ti_Read(&file,sizeof(file),1,slot);
	ti_CloseAll();
	menuopt = 0;
	while (1) {
		kb_Scan();
		k = kb_Data[1];
		if (k) keywait();
		if (k&kb_Mode) break;
		if (k&kb_2nd) {
			if (menuopt==0) { gfx_FillScreen(0); gfx_SwapDraw(); gfx_FillScreen(0); curscore = startGame(file.speed); initgfx(); keywait(); }
			else if (menuopt==1) file.speed = (!file.speed) ? 2 : file.speed-1;
			else break; 
		}
		k = kb_Data[7];
		if (k&kb_Up && menuopt!=0) menuopt--;
		if (k&kb_Down && menuopt!=2) menuopt++;
		if (k) keywait();
		gfx_FillScreen(0xFF);
		gfx_SetTextScale(2,2);
		for(i=0,y=84;i<3;i++,y+=24) {
			if (menuopt==i) gfx_SetTextFGColor(MENU_SELECTED_COLOR);
			centerstr(titlemenu[i],y);
			gfx_SetTextFGColor(MENU_NORMAL_COLOR);
		}
		gfx_SetTextScale(3,3);
		centerstr("Falldown CE",5);
		gfx_SetTextScale(1,1);
		gfx_SetTextXY(5,230);
		gfx_PrintString("High score (");
		gfx_PrintString(speedlabels[file.speed]);
		gfx_PrintString(") : ");
		gfx_PrintUInt(file.score[file.speed],6);
		gfx_PrintStringXY(VERSION_INFO,290,230);
		gfx_SwapDraw();
	}
	gfx_End();
	if (slot = ti_Open(filename,"w")) ti_Write(&file,sizeof(file),1,slot);
	ti_CloseAll();
}

/* Put other functions here */
void keywait() { while (kb_AnyKey()); }
void keywaitrelease() {
	keywait();
	while (!kb_AnyKey());
	keywait();
}
void centerstr(char* s,uint8_t y) {
	gfx_PrintStringXY(s,(LCD_WIDTH-gfx_GetStringWidth(s))>>1,y);
}

void initgfx() {
	gfx_Begin(gfx_8bpp);
	gfx_SetDrawBuffer();
}


