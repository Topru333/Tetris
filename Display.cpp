#include "Display.h"
#include <Windows.h>


Display::Display(int screenWidth, int screenHeight) : _screenWidth(screenWidth), _screenHeight(screenHeight) {
	_hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(_hConsole);

	COORD sizeOfBuff;
	sizeOfBuff.X = screenWidth;
	sizeOfBuff.Y = screenHeight;
	SetConsoleScreenBufferSize(_hConsole, sizeOfBuff);

	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL) {
		SetWindowPos(hwnd, 0, 0, 0, 680, 500, SWP_SHOWWINDOW | SWP_NOMOVE);
	}
}

Display::~Display() {
	CloseHandle(_hConsole);
}

void Display::Draw(const wchar_t* _screen) {
	WriteConsoleOutputCharacter(_hConsole, _screen, _screenWidth * _screenHeight, { 0,0 }, &_dwBytesWritten);
}
