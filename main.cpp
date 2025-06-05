#include "include.h"

#define CMD 0

#define CHAR 1

#define COL 2
#define ROW 3

#define POS_L 2
#define POS_H 3

#define X1_L 2
#define X1_H 3
#define Y1_L 4
#define Y1_H 5

#define X2_L 6
#define X2_H 7
#define Y2_L 8
#define Y2_H 9

#define SX_L 6
#define SX_H 7
#define SY_L 8
#define SY_H 9

#define W1_L 6
#define W2_L 7
#define H1_L 8
#define H2_L 9

#define R_L 6
#define R_H 7

#define F_COL 10
#define B_COL 11

volatile unsigned int inp = 0;
volatile bool int_h = false;

char REG[16];

char textBuffer[TEXTSIZE];
char tmpBuffer[] = {0,0};
short cursorPostiton = 0;

char CURSOR = 0; // X  X  X  X  X  X  BL EN

// void interrupt(uint gpio, uint32_t event_mask){
// 	inp = *(unsigned int*)(0xd0000004);
// 	interrupt_triggered = true;
// }

char buff[] = {1,0};
void drawCursor(){
	DrawTextBg(buff, (cursorPostiton % 80) * 8,(cursorPostiton / 80) * 8,1,1,REG[F_COL],REG[B_COL]);
}

void scrollScreen(){
	cursorPostiton = TEXTSIZE - 80;
		for(int i = 80; i < TEXTSIZE; i++){
			textBuffer[i - 80] = textBuffer[i];
		}
		for(int i = TEXTSIZE - 80; i < TEXTSIZE; i++){
			textBuffer[i] = 0;
		}
		DrawTextBuf(textBuffer, REG[F_COL], REG[B_COL]);
}

void print_chr(char inp){
	if(cursorPostiton == TEXTSIZE){
		scrollScreen();
		print_chr(inp);
	}else{
		tmpBuffer[0] = inp;
		DrawTextBg(tmpBuffer, (cursorPostiton % 80) * 8,(cursorPostiton / 80) * 8,1,1,REG[F_COL],REG[B_COL]);
		textBuffer[cursorPostiton] = inp;
		cursorPostiton++;
	}
}

void print_chr_console(char inp){
	switch (inp)
	{
	case '\n':
		if(cursorPostiton + 80 < TEXTSIZE){
			DrawTextBg(" ", (cursorPostiton % 80) * 8,(cursorPostiton / 80) * 8,1,1,REG[F_COL],REG[B_COL]);
			for(int i = (cursorPostiton % 80); i < 80; i++){
			textBuffer[cursorPostiton] = ' ';
			cursorPostiton++;
		}
		}else{
			scrollScreen();
		}
		break;
	
	case '\b':
		if(cursorPostiton > 0){
			cursorPostiton--;
			textBuffer[cursorPostiton] = ' ';
			DrawTextBg(" ", (cursorPostiton % 80) * 8,(cursorPostiton / 80) * 8,1,1,REG[F_COL],REG[B_COL]);
		}
		break;

	case '\r':
		break;

	default:
		print_chr(inp);
		break;
	}
	drawCursor();
}

int main()
{
	for(char i = 0; i < 8; i++){
		gpio_init(i);
		gpio_set_dir(i, GPIO_IN);
	}
	
	for(char i = 12; i < 16; i++){
		gpio_init(i);
		gpio_set_dir(i, GPIO_IN);
	}
	gpio_init(18); // Is Busy
	gpio_set_dir(18, GPIO_IN);	

	gpio_init(20); // Enable
	gpio_set_dir(20, GPIO_IN);	

	gpio_init(21); // RWB
	gpio_set_dir(21, GPIO_IN);

	gpio_init(22); // Busy Reset
	gpio_set_dir(22, GPIO_OUT);
	
	char buff[9];
	int addr = 0;
	for(int x = 0; x < 8; x++){
		gpio_put(22,1);
	}
	gpio_put(22,0);
	REG[F_COL] = COL_WHITE;
	drawCursor();

	while (true){
		//(inp & 0x40000) == 0x40000
		if(gpio_get(18) == true){
			while(gpio_get(20) == true);
			inp = *(unsigned int*)(0xd0000004);
			addr = (inp & 0xF000) >> 12;

			REG[addr] = (char)(inp & 0xFF);

			if(addr == 0){
				switch (REG[CMD])
				{
				case 0:
					print_chr_console(REG[CHAR]);
					break;
				case 1:
					print_chr(REG[CHAR]);
					break;
				default:
					break;
				}
			}
			
			// DecHexNum(buff,inp, 8);
			// DrawTextBg(buff, 200,400,1,1,COL_WHITE,COL_BLACK);
			// DecHexNum(buff,REG[addr], 2);
			// DrawTextBg(buff, 200,(addr * 8),1,1,COL_WHITE,COL_BLACK);
			for(int x = 0; x < 8; x++){
				gpio_put(22,1);
			}
			gpio_put(22,0);
		}
		
	}
}