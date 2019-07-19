#include <iostream>
#include <string> 
#include <thread>
#include <vector>
#include <algorithm>
#include "Display.h";

using namespace std;

const int w = 4;

const int nFieldWidth  = 12;
const int nFieldHeight = 18;
unsigned char *pField = nullptr;

const int nScreenWidth  = 80;     // Console Screen Size X (columns)
const int nScreenHeight = 30;     // Console Screen Size Y (rows)

vector<int> vLines;
vector<wstring> figures;

int nScore;

int Rotate(int px, int py, int r) {
	switch (r % w) {
		case 0: return py * w + px;                  // 0   degrees
		case 1: return w * (w - 1) + py - (px * w);  // 90  degrees
		case 2: return (w * w) - 1 - (py * w) - px;  // 180 degrees
		case 3: return w - 1 - py + (px * 4);        // 270 degrees
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	int curX, curY;
	for (int px = 0; px < w; px++) {
		for (int py = 0; py < w; py++) {
			curY = py + nPosY;
			curX = px + nPosX;

			// Index into piece
			int pi = Rotate(px, py, nRotation);

			// Index into field
			int fi = curY * nFieldWidth + curX;

			if (curX >= 0 && curX < nFieldWidth) {
				if (curY >= 0 && curY < nFieldHeight) {
					if (figures[nTetromino][pi] == L'X' && pField[fi] != 0) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

void setFigures(vector<wstring> &_figures) {
	_figures.clear();
	_figures.push_back({ L"..X." L"..X." L"..X." L"..X." });
	_figures.push_back({ L".X.." L".XX." L"..X." L"...." });
	_figures.push_back({ L"..X." L".XX." L".X.." L"...." });
	_figures.push_back({ L".XX." L".XX." L"...." L"...." });
	_figures.push_back({ L"..X." L".XX." L"..X." L"...." });
	_figures.push_back({ L".XX." L"..X." L"..X." L"...." });
	_figures.push_back({ L".XX." L".X.." L".X.." L"...." });
}

void StartGame(Display &display) {
	pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play field buffer
	for (int x = 0; x < nFieldWidth; x++) { // Board Boundary
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	int count = nScreenHeight * nScreenWidth;
	wchar_t* screen = new wchar_t[count];

	for (int i = 0; i < count; i++) {
		screen[i] = L' ';
	}
	

	// Game Logic Stuff
	bool bGameOver = false;

	int nCurrentPiece = rand() % 7;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bRotateHold = true;
	bool bKey[4];

	int nSpeed = 20;
	bool bForceDown = false;

	int nPieceCount = 0;
	int nSpeedCount = 0;

	while (!bGameOver) {
		// Game Timing
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		// Input
		for (int k = 0; k < 4; k++) {
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		// Gane Logic

		// Movement by player
		nCurrentX += bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY) ? 1 : 0;
		nCurrentX -= bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY) ? 1 : 0;
		nCurrentY += bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1) ? 1 : 0;

		// Rotate, but latch to stop wild spinning
		if (bKey[3]) {
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else {
			bRotateHold = true;
		}

		if (bForceDown) {
			// Update difficulty every 50 pieces
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0) {
				if (nSpeed >= 10) nSpeed--;
			}

			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
			else {
				// It can't! Lock the piece in place
				for (int px = 0; px < w; px++) {
					for (int py = 0; py < w; py++) {
						if (figures[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}

				// Check full lines
				for (int py = 0; py < w; py++) {
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++) {
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}

						// All characters set to = if line is full
						if (bLine) {
							for (int px = 1; px < nFieldWidth - 1; px++) {
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							}
							vLines.push_back(nCurrentY + py);
						}
					}
				}

				nScore += 25;
				if (!vLines.empty()) {
					nScore += (1 << vLines.size()) * 100;
				}


				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		// Draw Field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		// Draw Current Piece
		for (int px = 0; px < w; px++) {
			for (int py = 0; py < w; py++) {
				if (figures[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
				}
			}
		}

		// Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		// Animate Line Completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			display.Draw(screen);
			this_thread::sleep_for(400ms); // Delay a bit

			for (auto& v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--) {
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}

			vLines.clear();
		}

		// Display Frame
		display.Draw(screen);
	}

}

int main() {
	setFigures(figures);
	
	while (true) {
		
		if (nScore > 0) {
			cout << "Game Over!! Score:" << nScore << endl;
			nScore = 0;
		}
		
		cout << "Do you want to start the game?" << endl;
		std::string line;
		cin >> line;
		std::transform(line.begin(), line.end(), line.begin(), 
			[](unsigned char c) { return std::tolower(c); });
		if (static_cast<int>(line.find("yes")) > -1) {
			Display display(nScreenWidth, nScreenHeight);
			StartGame(display);
		}
		else {
			break;
		}

	}

	return 0;
}
