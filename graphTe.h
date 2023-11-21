#ifndef _GRAPHTE_H_
#define _GRAPHTE_H_

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define _CMAX 10000

#define int16 short int  
#define uint16 unsigned short int

long double map(long double x, long double inMin, long double inMax, long double outMin, long double outMax)
{
	return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

typedef struct
{
	uint16 red, green, blue;
}
color;

color rgb(uint16 red, uint16 green, uint16 blue)
{
	return (color){red, green, blue};
}

struct
{
	HWND hwnd;
	HDC hdc, bufferDC, imageDC;
	HBITMAP bufferBitmap;
	RECT rect;
	uint16 width, height;
	HANDLE outputHandle;
	HPEN linePen;
	char title[_CMAX];
}
host;

typedef enum
{
	BLACK = 0,
	NAVY = 1,
	GREEN = 2,
	TEAL = 3,
	MAROON = 4,
	PURPLE = 5,
	OLIVE = 6,
	SILVER = 7,
	GRAY = 8,
	BLUE = 9,
	LIME = 10,
	AQUA = 11,
	RED = 12,
	FUCHSIA = 13,
	YELLOW = 14,
	WHITE = 15
}
vgaColor;

typedef struct
{
	uint16 x, y;
}
vector2u;

void updateWindowBounds()
{
	GetWindowRect(host.hwnd, &host.rect);
	host.width = host.rect.right - host.rect.left;
	host.height = host.rect.bottom - host.rect.top;
}

void disableConsoleCursor()
{
	CONSOLE_CURSOR_INFO cInfo;
	cInfo.dwSize = 100;
    cInfo.bVisible = FALSE;
	SetConsoleCursorInfo(host.outputHandle, &cInfo);
}

void update()
{
	updateWindowBounds();

	DeleteObject(host.bufferBitmap);
	host.bufferBitmap = CreateCompatibleBitmap(host.hdc, host.width, host.height);
	SelectObject(host.bufferDC, host.bufferBitmap);	

	disableConsoleCursor();
}

void initHost()
{
	//sync handles
	host.hwnd = GetConsoleWindow();
	host.hdc = GetDC(host.hwnd);
	host.bufferDC = CreateCompatibleDC(host.hdc);
	host.imageDC = CreateCompatibleDC(host.bufferDC);
	SelectObject(host.bufferDC, GetStockObject(DC_PEN));
	SelectObject(host.bufferDC, GetStockObject(DC_BRUSH));
	host.outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetBkMode(host.bufferDC, TRANSPARENT);

	disableConsoleCursor();

	update();

	Sleep(100);
}	

void releaseHost()
{
	DeleteObject(host.bufferBitmap);
	DeleteDC(host.bufferDC);
	DeleteObject(host.imageDC);
	ReleaseDC(host.hwnd, host.hdc);
}

void display()
{
	BitBlt(host.hdc, 0, 0, host.width, host.height, host.bufferDC, 0, 0, SRCCOPY);
}

void moveCursor(uint16 x, uint16 y)
{
	COORD position;
	position.X = x;
	position.Y = y;
	SetConsoleCursorPosition(host.outputHandle, position);
}

void pixel(int16 x, int16 y, color fillColor)
{
	SetPixelV(host.bufferDC, x, y, RGB(fillColor.red, fillColor.green, fillColor.blue));
}

void rect(int16 x, int16 y, uint16 width, uint16 height, color fillColor)
{
	RECT frame;

	frame.left = x;
	frame.top = y;
	frame.right = x + width;
	frame.bottom = y + height;

	SetDCBrushColor(host.bufferDC, RGB(fillColor.red, fillColor.green, fillColor.blue));
	FillRect(host.bufferDC, &frame, GetStockObject(DC_BRUSH));
}

void fill(color fillColor)
{
	rect(0, 0, host.width, host.height, fillColor);
}

void line(int16 x1, int16 y1, int16 x2, int16 y2, uint16 width,color fillColor)
{
	host.linePen = CreatePen(PS_SOLID, width, RGB(fillColor.red, fillColor.green, fillColor.blue));
	SelectObject(host.bufferDC, host.linePen);
	MoveToEx(host.bufferDC, x1, y1, NULL);
	LineTo(host.bufferDC, x2, y2);
	DeleteObject(host.linePen);
	SelectObject(host.bufferDC, GetStockObject(DC_PEN));
}
	
void ellipse(int16 x, int16 y, uint16 width, uint16 height, color fillColor)
{
	SetDCPenColor(host.bufferDC, RGB(fillColor.red, fillColor.green, fillColor.blue));
	SetDCBrushColor(host.bufferDC, RGB(fillColor.red, fillColor.green, fillColor.blue));
	Ellipse(host.bufferDC, x, y, x + width, y + height);
}

void circle(int16 x, int16 y, uint16 radius, color fillColor)
{
	ellipse(x, y, 2 * radius, 2 * radius, fillColor);
}

void image(int16 x, int16 y, uint16 width, uint16 height, char* filenamePTR)
{
	char filename[_CMAX];
	strcpy(filename, filenamePTR); 
	HBITMAP imageBitmap = LoadImageA(NULL, filename, IMAGE_BITMAP, width, height, LR_LOADFROMFILE);
	SelectObject(host.imageDC, imageBitmap);

	BitBlt(host.bufferDC, x, y, width, height, host.imageDC, 0, 0, SRCCOPY);
	
	DeleteObject(imageBitmap);
}

void transparentImage(int16 x, int16 y, uint16 width, uint16 height, char* filenamePTR, color transparentColor)
{
	char filename[_CMAX];
	strcpy(filename, filenamePTR); 
	HBITMAP imageBitmap = LoadImageA(NULL, filename, IMAGE_BITMAP, width, height, LR_LOADFROMFILE);
	SelectObject(host.imageDC, imageBitmap);

	TransparentBlt(host.bufferDC, x, y, width, height, host.imageDC, 0, 0, width, height, RGB(transparentColor.red, transparentColor.green, transparentColor.blue));

	DeleteObject(imageBitmap);
}

void textRect(int16 x, int16 y, uint16 width, uint16 height, char* textPTR, color fillColor)
{
	char text[_CMAX];
	strcpy(text, textPTR);

	RECT textBox;
	textBox.left = x;
	textBox.top = y;
	textBox.right = x + width;
	textBox.bottom = y + height;

	SetTextColor(host.bufferDC, RGB(fillColor.red, fillColor.green, fillColor.blue));
	DrawText(host.bufferDC, text, strlen(text), &textBox, DT_LEFT);
}

void text(int16 x, int16 y, char* textPTR, color fillColor)
{
	char text[_CMAX];
	strcpy(text, textPTR);

	RECT textBox;
	textBox.left = x;
	textBox.top = y;
	textBox.right = x + 10;
	textBox.bottom = y + 10;

	SetTextColor(host.bufferDC, RGB(fillColor.red, fillColor.green, fillColor.blue));
	DrawText(host.bufferDC, text, strlen(text), &textBox, DT_NOCLIP);
}

uint16 getVGAColor(vgaColor fillColor, vgaColor bgColor)
{
	return bgColor * 16 + fillColor;
}

void setPrintColor(vgaColor fillColor, vgaColor bgColor)
{
	SetConsoleTextAttribute(host.outputHandle, getVGAColor(fillColor, bgColor));
}

void setWindowTitle(char* title)
{
	SetConsoleTitle(title);
}

char* getWindowTitle()
{	
	GetWindowTextA(host.hwnd, host.title, _CMAX);
	return host.title;
}

void setWindowSize(uint16 width, uint16 height)
{
	updateWindowBounds();
	MoveWindow(host.hwnd, host.rect.left, host.rect.top, width, height, TRUE);

	Sleep(100);
}

vector2u getWindowSize()
{
	updateWindowBounds();
	return (vector2u){host.width, host.height};
}

void setWindowPosition(uint16 x, uint16 y)
{
	updateWindowBounds();
	MoveWindow(host.hwnd, x, y, host.width, host.height, TRUE);
}

vector2u getWindowPosition()
{
	updateWindowBounds();
	return (vector2u){host.rect.left, host.rect.top};
}

char input()
{
	if(kbhit())
		return getch();
	else 
		return 0;
}

char forceInput()
{
	return getch();
}

//disable quick edit mode in conhost for mouse input
BOOL checkKeyLiveInput(WORD keyCode)
{
	return GetAsyncKeyState(keyCode);
}

POINT getMousePosition()
{
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(GetForegroundWindow(), &mPos);

	return mPos;
}

void playSound(char* filenamePTR)
{
	char filename[_CMAX];
	strcpy(filename, filenamePTR);

	PlaySoundA(filename, NULL, SND_FILENAME | SND_ASYNC);
}

#endif 