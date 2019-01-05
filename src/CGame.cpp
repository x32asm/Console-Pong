#include "CGame.h"

#include <iostream>
#include <stdexcept>


CGame::CGame(SHORT nScreenWidth, SHORT nScreenHeight) :
	szTitle("CONSOLE PONG"),
	nScreenWidth(nScreenWidth), nScreenHeight(nScreenHeight), nScreenResolution(nScreenWidth*nScreenHeight),
	hConsole(NULL), srBufferRect(), csbi(), rgScreenBuffer(nScreenResolution) {
	csbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

	if (!GetConsoleHandle()) {
		std::printf("GetConsoleHandle: %X\n", GetLastError());
		throw std::runtime_error("Failed to get console handle.");
	}
}

CGame::CGame(SHORT nScreenWidth, SHORT nScreenHeight, COLORREF pColorTable[16]) :
	szTitle("CONSOLE PONG"),
	nScreenWidth(nScreenWidth), nScreenHeight(nScreenHeight), nScreenResolution(nScreenWidth*nScreenHeight),
	hConsole(NULL), srBufferRect(), csbi(), rgScreenBuffer(nScreenResolution) {
	csbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

	if (!GetConsoleHandle()) {
		std::printf("GetConsoleHandle: %X\n", GetLastError());
		throw std::runtime_error("Failed to get console handle.");
	}
}

CGame::~CGame() {
	CloseHandle(hConsole);
}




void CGame::ClearScreen() {
	for (SIZE_T i = 0; i < rgScreenBuffer.size(); ++i) {
		rgScreenBuffer[i].Char.UnicodeChar = L' ';
		rgScreenBuffer[i].Attributes = 0;
	}
}

void CGame::DrawRect(INT X, INT Y, INT nRectWidth, INT nRectHeight, WCHAR chFillCharacter, INT nColor) {
	INT i, nIndex;

	for (i = X; i < X + nRectWidth; ++i) {
		nIndex = i + Y * nScreenWidth;
		if (nIndex >= 0 && nIndex < nScreenResolution) {
			rgScreenBuffer[nIndex].Char.UnicodeChar = chFillCharacter;
			rgScreenBuffer[nIndex].Attributes = nColor;
		}
	}
	for (i = X; i < X + nRectWidth; ++i) {
		nIndex = i + (Y + nRectHeight - 1) * nScreenWidth;
		if (nIndex >= 0 && nIndex < nScreenResolution) {
			rgScreenBuffer[nIndex].Char.UnicodeChar = chFillCharacter;
			rgScreenBuffer[nIndex].Attributes = nColor;
		}
	}

	for (i = Y + 1; i < Y + nRectHeight; ++i) {
		nIndex = X + i * nScreenWidth;
		if (nIndex >= 0 && nIndex < nScreenResolution) {
			rgScreenBuffer[nIndex].Char.UnicodeChar = chFillCharacter;
			rgScreenBuffer[nIndex].Attributes = nColor;
		}
	}
	for (i = Y + 1; i < Y + nRectHeight; ++i) {
		nIndex = X + nRectWidth - 1 + i * nScreenWidth;
		if (nIndex >= 0 && nIndex < nScreenResolution) {
			rgScreenBuffer[nIndex].Char.UnicodeChar = chFillCharacter;
			rgScreenBuffer[nIndex].Attributes = nColor;
		}
	}
}

void CGame::FillRect(INT X, INT Y, INT nRectWidth, INT nRectHeight, WCHAR chFillCharacter, INT nColor) {
	INT nIndex;
	for (INT r = 0; r < nRectHeight; ++r) {
		for (INT c = 0; c < nRectWidth; ++c) {
			nIndex = (X + c) + (Y + r) * nScreenWidth;
			if (nIndex < nScreenResolution && nIndex >= 0) {
				rgScreenBuffer[nIndex].Char.UnicodeChar = chFillCharacter;
				rgScreenBuffer[nIndex].Attributes = nColor;
			}
		}
	}
}

void CGame::DrawLine(INT X1, INT Y1, INT X2, INT Y2, WCHAR chFillCharacter, INT nColor) {
	FLOAT nDX = -(FLOAT)(X1 - X2) / 50, nDY = -(FLOAT)(Y1 - Y2) / 50;
	FLOAT fX = (FLOAT)X1, fY = (FLOAT)Y1;
	INT nIndex;
	while ((INT)fX != X2 && (INT)fY != Y2) {
		nIndex = (INT)(fX + fY * nScreenWidth);
		if ((INT)fX >= 0 && (INT)fX < nScreenWidth && (INT)fY >= 0 && (INT)fY < nScreenHeight && nIndex > 0 && nIndex < nScreenResolution) {
			rgScreenBuffer[nIndex].Char.UnicodeChar = chFillCharacter;
			rgScreenBuffer[nIndex].Attributes = nColor;
		}
		fX += nDX;
		fY += nDY;
	}
}

void CGame::DrawPixel(INT X, INT Y, WCHAR chFillCharacter, INT nColor) {
	if (X + Y * nScreenWidth < nScreenResolution) {
		rgScreenBuffer[X + Y * nScreenWidth].Char.UnicodeChar = chFillCharacter;
		rgScreenBuffer[X + Y * nScreenWidth].Attributes = nColor;
	}
}

void CGame::DrawString(INT X, INT Y, const WCHAR * szFillString, SIZE_T nStringLen, INT nColor) {
	for (SIZE_T i = 0; i < nStringLen; ++i) {
		if (X + i + Y * nScreenWidth < (SIZE_T)nScreenResolution) {
			rgScreenBuffer[X + i + Y * nScreenWidth].Char.UnicodeChar = szFillString[i];
			rgScreenBuffer[X + i + Y * nScreenWidth].Attributes = nColor;
		}
	}
}


BOOL CGame::FlushBuffer() {
	return WriteConsoleOutput(hConsole, &rgScreenBuffer[0], { nScreenWidth, nScreenHeight }, { 0, 0 }, &srBufferRect);
}




BOOL CGame::GetConsoleHandle(const COLORREF pColorTable[16]) {
	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	if (hConsole == INVALID_HANDLE_VALUE) return FALSE;

	srBufferRect = { 0, 0, nScreenWidth - 1, nScreenHeight - 1 };

	SetConsoleScreenBufferSize(hConsole, { nScreenWidth, nScreenHeight });
	SetConsoleWindowInfo(hConsole, TRUE, &srBufferRect);
	if (!GetConsoleScreenBufferInfoEx(hConsole, &csbi)) goto Error;

	for (SIZE_T i = 0; i < 16; ++i) {
		csbi.ColorTable[i] = pColorTable[i];
	}

	csbi.dwSize = { nScreenWidth, nScreenHeight };
	csbi.srWindow = { csbi.srWindow.Left, csbi.srWindow.Top, nScreenWidth, nScreenHeight };
	csbi.dwMaximumWindowSize = { nScreenWidth, nScreenHeight };

	if (!SetConsoleScreenBufferInfoEx(hConsole, &csbi)) goto Error;

	if (!SetConsoleTitleA((LPCSTR)szTitle.c_str())) goto Error;
	if (!SetConsoleActiveScreenBuffer(hConsole)) goto Error;

	return TRUE;

Error:
	CloseHandle(hConsole);
	return FALSE;
}




void CGame::SetTitle(const std::string & szTitle) {
	CGame::szTitle = szTitle;
	SetConsoleTitleA((LPCSTR)szTitle.c_str());
}
