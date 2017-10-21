/*
 *--------------------------------------
 * Program Name: Falldown
 * Author: Rodger 'Iambian' Weisman
 * License: MIT
 * Description: Fall through the gaps and don't hit the top of the screen.
 *--------------------------------------
*/

#define VERSION_INFO "v0.1"
#define MENU_SELECTED_COLOR 0x4F
#define MENU_NORMAL_COLOR 0x00
#define TRANSPARENT_COLOR 0xF8
#define GREETINGS_DIALOG_TEXT_COLOR 0xDF

#define RAMP_DISTANCE 96
#define GAPS_TO_MAKE 4

#define GMBOX_X (LCD_WIDTH/4)
#define GMBOX_Y (LCD_HEIGHT/2-LCD_HEIGHT/16)
#define GMBOX_W (LCD_WIDTH/2)
#define GMBOX_H (LCD_HEIGHT/8)


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
#include <decompress.h>

#include "gfx/sprites_gfx.h"

/* Put your function prototypes here */
extern void clearRectFast(uint8_t ypos);

int gamemode();
void keywait();
void keywaitrelease();
void centerstr(char* s,uint8_t y);
void drawtitle();
void drawblocks(uint8_t ypos);
void vsync();

/* Put all your globals here */
gfx_UninitedSprite(balltemp,ball_width,ball_height);
struct { uint8_t ver; int score[4]; uint8_t speed; } file;
char* filename = "FALLDDAT";
char *title = "Falldown CE";
char* titlemenu[] = {"Start Game","Change Speed","About","Quit Game"};
char* speedlabels[] = {"Slow","Medium","Fast","Hyper"};
#define CREDITS_LEN 6
char* credits[] = {
	"Push LEFT/RIGHT to move the ball",
	"and make it fall through the gaps.",
	"Don't let the ball touch the top",
	"",
	"Program by Rodger 'Iambian' Weisman",
	"Released under the MIT License",
};

void main(void) {
	uint8_t i,y,menuopt;
	kb_key_t k;
	ti_var_t slot;
	int unsigned curscore;
	int *temp_ptr;
	
	/* Initialize system */
	gfx_Begin(gfx_8bpp);
	gfx_SetDrawBuffer();
	gfx_SetTransparentColor(TRANSPARENT_COLOR);

	/* Initialize variables */
	memset(&file,0,sizeof(file));
	ti_CloseAll();
	if (slot = ti_Open(filename,"r")) ti_Read(&file,sizeof(file),1,slot);
	ti_CloseAll();
	menuopt = 0;
	while (1) {
		kb_Scan();
		
		k = kb_Data[1];
		if (k&kb_2nd) {
			if (menuopt==0) { 
				curscore = gamemode();
				temp_ptr = &file.score[file.speed];
				if (curscore > *temp_ptr) *temp_ptr = curscore;
			}
			else if (menuopt==1) file.speed = (file.speed+1)&3;
			else if (menuopt==2) {
				drawtitle();
				for (i=0,y=80;i<CREDITS_LEN;i++,y+=16) centerstr(credits[i],y);
				gfx_SwapDraw();
				keywaitrelease();
			}
			else break;
			keywait();
		} else if (k&kb_Mode) break;
		
		k = kb_Data[7];
		if (k&kb_Up) menuopt--;
		if (k&kb_Down) menuopt++;
		if (k) {
			menuopt &= 3;
			keywait();
		}
		drawtitle();
		gfx_SetTextScale(2,2);
		for(i=0,y=80;i<4;i++,y+=24) {
			if (menuopt==i) gfx_SetTextFGColor(MENU_SELECTED_COLOR);
			centerstr(titlemenu[i],y);
			gfx_SetTextFGColor(MENU_NORMAL_COLOR);
		}
		gfx_SwapDraw();
	}
	gfx_End();
	if (slot = ti_Open(filename,"w")) ti_Write(&file,sizeof(file),1,slot);
	ti_CloseAll();
	keywait();
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


/*  NOTE: Use the following variable/labels to access sprite data:
			ball
			block
	These names have been defined for us in sprites_gfx.h and is of
	the type gfx_sprite_t.
*/
int gamemode() {
	uint8_t speedsteps[] = {1,2,3,4};
	uint8_t gapsteps[] = {4,4,3,2};
	uint8_t gap,i,j,speed,gapmaxdist,bally,y,h,w,timer,ypos;
	int8_t dir;
	uint24_t blockmask;
	int score,ballx,x,gap_ramp_dist,xpos;
	kb_key_t k;
	
	gfx_FillScreen(0xFF);
	
	dir = 0;
	timer = 0;
	score = 0;
	gap = 20;
	gapmaxdist = 120;
	gap_ramp_dist = RAMP_DISTANCE;
	ballx = ((320+ball_width)/2);
	bally = 30;
	gfx_SetTextScale(1,1);
	gfx_SetColor(0xFF);
	
	while (1) {
		speed = speedsteps[file.speed];
		if (bally==(240-16-ball_height)) speed = speed<<1;
		
		//Erase current sprite object
		gfx_FillRectangle_NoClip(2,2,86,8);
		gfx_FillRectangle_NoClip(ballx,bally,ball_width,ball_height);
		gfx_ShiftUp(speed);
		clearRectFast(240-speed);
		// gfx_SetColor(0xFF);
		// gfx_HorizLine_NoClip(0,239,320);
		// if (speed>1) gfx_HorizLine_NoClip(0,238,320);
		// if (speed>2) gfx_HorizLine_NoClip(0,237,320);
		// if (speed>3) gfx_HorizLine_NoClip(0,236,320);
		// if (speed>4) gfx_HorizLine_NoClip(0,235,320);
		gfx_SetTextXY(2,2);
		gfx_PrintString("SCORE: ");
		gfx_PrintUInt(score>>8,5);
		
		h = speed<<1;
		bally -= speed;
		y = bally+ball_height;
		for(i=0;(i<h && bally<(240-16-ball_height));i++,bally++,y++) {
			x = ballx;
			for(j=ball_width;j>0;j--,x++) {
				if ((uint8_t)(gfx_GetPixel(x,y)+1)) break;
			}
			if (j) break;  //ball has collided. Do not descend further.
		}
		
		kb_Scan();
		k = kb_Data[1];
		if (k&kb_Mode) break;
		dir = 0;
		k = kb_Data[7];
		if (k&kb_Left) dir = -1;
		if (k&kb_Right) dir = 1;
		if (dir) {
			w = 4;
			if (dir==1) x = ballx+ball_width;
			else x = ballx-1;
			for (i=0;(i<w && x>0 && x<320);i++,x+=dir,ballx+=dir) {
				y = bally;
				for (j=ball_height;j>0;j--,y++) {
					if ((uint8_t)(gfx_GetPixel(x,y)+1)) break;
				}
				if (j) break;
			}
		}
		
		if (bally>240) {
			gfx_SetColor(0x08);  //xlibc dark blue
			gfx_SetTextBGColor(0xFF);
			gfx_FillRectangle(GMBOX_X,GMBOX_Y,GMBOX_W,GMBOX_H);
			gfx_SetTextFGColor(GREETINGS_DIALOG_TEXT_COLOR);
			centerstr("Game over",GMBOX_Y+10);
			gfx_BlitBuffer();
			keywaitrelease();
			gfx_SetTextFGColor(0x00);
			return score>>8;
		}
		
		if (!((timer++)&1)) {
			if (dir==(-1)) gfx_RotateSpriteCC(ball,balltemp);
			else if (dir==(1)) gfx_RotateSpriteC(ball,balltemp);
			else memcpy(balltemp,ball,ball_height*ball_width+2);
			memcpy(ball,balltemp,ball_height*ball_width+2);
		}
		gfx_TransparentSprite_NoClip(ball,ballx,bally);
		
		gap -= speed;
		if (gap>240) {
			//Start draw blocks
			ypos = gap + (240 - 8);
			blockmask = 0;
			for (i=0;i<gapsteps[file.speed];i++) {
				blockmask |= 1<<randInt(0,19);
			}
			for (i=0,xpos=304;i<20;i++,xpos-=16) {
				if (!(((uint8_t)blockmask)&1)) {
					gfx_Sprite_NoClip(block,xpos,ypos);
				}
				blockmask >>=1;
			}
			//End draw blocks
			gap = gapmaxdist;
		}
		if (!(--gap_ramp_dist)) {
			gap_ramp_dist = RAMP_DISTANCE;
			gapmaxdist-=2;
		}
		
		
		score+=(((int)(speed))*((int)(bally>>1)));
		gfx_SwapDraw();
		gfx_BlitScreen();
//		vsync();
//		gfx_BlitBuffer();
	}
	return score>>8;
}

void drawtitle() {
	gfx_FillScreen(0xFF);
	gfx_SetTextScale(3,3);
	centerstr("Falldown CE",5);
	gfx_SetTextScale(1,1);
	gfx_SetTextXY(5,230);
	gfx_PrintString("High score (");
	gfx_PrintString(speedlabels[file.speed]);
	gfx_PrintString(") : ");
	gfx_PrintUInt(file.score[file.speed],6);
	gfx_PrintStringXY(VERSION_INFO,286,230);
}


/* Use this only if you're going to go the single buffer route */
// void vsync() {
	// asm("ld hl, 0E30000h +  0028h");    //interrupt clear register
	// asm("set 3,(hl)");                  //clear vcomp interrupt (write 1)
	// asm("ld l, 0020h");                 //interrupt raw register
	// asm("_vsync_loop");
	// asm("bit 3,(hl)");                  //Wait until 1 (interrupt triggered)
	// asm("jr z,_vsync_loop");
	// asm("ret");
// }	
