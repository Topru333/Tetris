#include <Windows.h>

class Display {

public:
	Display(int screenWidth, int screenHeight);
	~Display();
	void Draw(const wchar_t* _screen);

private:
	int _screenWidth, _screenHeight;
	HANDLE _hConsole;
	DWORD _dwBytesWritten = 0;
};

