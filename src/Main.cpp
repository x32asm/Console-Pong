#include "CGame.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <string>

const FLOAT PI = (FLOAT)3.14159265;
#define Radian(angle) ( angle * PI / 180 )

#define BALL_VEL (FLOAT)100
struct Ball {
	FLOAT x, y;
	FLOAT xvel, yvel;

	WCHAR ch;
};

#define PADDLE_VEL (FLOAT)50
#define BOT_PADDLE_VEL (FLOAT)42.5
struct Paddle {
	FLOAT x, y;
	INT width, height;

	WCHAR ch;
};


auto tp1 = std::chrono::system_clock::now();
auto tp2 = std::chrono::system_clock::now();
std::chrono::duration<FLOAT> dElapsedTime;
FLOAT fElapsedTime;

BOOL bBotPlayer = TRUE;

void PaddleCheck(Paddle * pPaddle, INT nVkUp, INT nVkDown, CGame * pCgame) {
	if (GetAsyncKeyState(nVkUp) & 0x8000) {
		pPaddle->y -= PADDLE_VEL * fElapsedTime;
	}
	else if (GetAsyncKeyState(nVkDown) & 0x8000) {
		pPaddle->y += PADDLE_VEL * fElapsedTime;
	}

	if (pPaddle->y < 0) pPaddle->y = 0;
	else if (pPaddle->y > pCgame->GetHeight() - pPaddle->height) pPaddle->y = (FLOAT)(pCgame->GetHeight() - pPaddle->height);
}

void BotPaddleCheck(Paddle * pPaddle, Ball * pBall, CGame * pCgame) {
	if (pBall->y < pPaddle->y) {
		pPaddle->y -= BOT_PADDLE_VEL * fElapsedTime;
	}
	else if (pBall->y > pPaddle->y + pPaddle->height) {
		pPaddle->y += BOT_PADDLE_VEL * fElapsedTime;
	}

	if (pPaddle->y < 0) pPaddle->y = 0;
	else if (pPaddle->y > pCgame->GetHeight() - pPaddle->height) pPaddle->y = (FLOAT)(pCgame->GetHeight() - pPaddle->height);
}

BOOL bP1Win = FALSE, bP2Win = FALSE;
void BallCheck(Ball * pBall, Paddle * pPaddle1, Paddle * pPaddle2, CGame * pCgame) {

	FLOAT step = (FLOAT)0.05;
	BOOL bCollision = FALSE;
	for (FLOAT i = 0; i < 1 / step; ++i) {
		pBall->x += pBall->xvel * step * fElapsedTime;
		pBall->y += pBall->yvel * step * fElapsedTime;


		if (bCollision) continue;
		FLOAT fBallAngle;
		if ((INT)pBall->x == (INT)(pPaddle1->x + pPaddle1->width)) {
			if ((INT)pBall->y >= (INT)pPaddle1->y && (INT)pBall->y <= (INT)(pPaddle1->y + pPaddle1->height)) {
				++pBall->x;
				pBall->xvel *= -1;
				fBallAngle = (FLOAT)(6 * pow(abs(pBall->y - (pPaddle1->y + pPaddle1->height / 2)), 1.5));
				pBall->xvel = std::cos(Radian(fBallAngle)) * BALL_VEL;
				pBall->yvel = std::sin(Radian(fBallAngle)) * BALL_VEL * (pBall->y < pPaddle1->y + pPaddle1->height / 2 ? -1 : 1);
				bCollision = TRUE;
			}
		}

		if ((INT)pBall->x == (INT)pPaddle2->x) {
			if ((INT)pBall->y >= (INT)pPaddle2->y && (INT)pBall->y <= (INT)(pPaddle2->y + pPaddle2->height)) {
				--pBall->x;
				fBallAngle = (FLOAT)(180 + 6 * pow(abs(pBall->y - (pPaddle2->y + pPaddle2->height / 2)), 1.5));
				pBall->xvel = std::cos(Radian(fBallAngle)) * BALL_VEL;
				pBall->yvel = std::sin(Radian(fBallAngle)) * BALL_VEL * (pBall->y < pPaddle2->y + pPaddle2->height / 2 ? 1 : -1);
				bCollision = TRUE;
			}
		}
	}


	if (pBall->y < 0) {
		pBall->y = 0;
		pBall->yvel *= -1;
	}
	else if (pBall->y >= pCgame->GetHeight()) {
		pBall->y = (FLOAT)(pCgame->GetHeight() - 1);
		pBall->yvel *= -1;
	}

	if (pBall->x < 0) {
		bP2Win = TRUE;
	}
	else if (pBall->x >= pCgame->GetWidth()) {
		bP1Win = TRUE;
	}
}


#define VK_W 0x57
#define VK_S 0x53
int main(int argc, char ** argv) {
	if (argc > 1) {
		if (!(strcmp(argv[1], "two") && strcmp(argv[1], "2")))
			bBotPlayer = FALSE;
		else {
			std::cout << "usage:\tpong.exe\t; play single player\n\tpong.exe two\t; play multi-player\n";
			return 0;
		}
	}
	srand((unsigned)time(0));
	rand(); rand(); rand();
	CGame cgame(161, 50);

	
	SHORT nPaddleWidth = 2, nPaddleHeight = 7;

	FLOAT nBallAngle = PI * (rand() % 2);
	Ball ball = { (FLOAT)(cgame.GetWidth() / 2), // X
					(FLOAT)(cgame.GetHeight() / 2), // Y
					std::cos(nBallAngle) * BALL_VEL, // X Velocity
					std::sin(nBallAngle) * BALL_VEL, // Y Velocity
					L'@' }; // Char

	Paddle p1 = {	(FLOAT)10, // X
					(FLOAT)(cgame.GetHeight() / 2 -  nPaddleHeight / 2), // Y
					nPaddleWidth, // Width
					nPaddleHeight, // Height
					L'#' }; // Char

	Paddle p2 = {	(FLOAT)(cgame.GetWidth() - 10 - nPaddleWidth), // X
					(FLOAT)(cgame.GetHeight() / 2 - nPaddleHeight / 2), //Y
					nPaddleWidth, // Width
					nPaddleHeight, //Height
					L'#' }; // Char


	WCHAR wchP1Score = L'0';
	WCHAR wchP2Score = L'0';
	INT nP1ScoreX = cgame.GetWidth() / 2 - 8, nP1ScoreY = 3;
	INT nP2ScoreX = cgame.GetWidth() / 2 + 8, nP2ScoreY = 3;
	BOOL bGameOver = FALSE;
	std::wstring szGameOver = L"Game Over";
	std::wstring szStartMessage = L"Press  space  to  start";

	std::wstring szWin = L"";
	INT nFrameRate = 0, nFrameCount = 0;
	auto fpsTime1 = std::chrono::system_clock::now();
	auto fpsTime2 = std::chrono::system_clock::now();
	std::chrono::duration<FLOAT> dfpsElapsedTime;
	while (1) {
		tp2 = std::chrono::system_clock::now();
		dElapsedTime = tp2 - tp1;
		tp1 = tp2;
		fElapsedTime = dElapsedTime.count();

		cgame.ClearScreen();

		cgame.FillRect(cgame.GetWidth() / 2 + (cgame.GetWidth() % 2 == 0 ? 1 : 0), 0, (cgame.GetWidth() % 2 == 0 ? 2 : 1), cgame.GetHeight(), L'|', 15);

		cgame.FillRect((INT)p1.x, (INT)p1.y, p1.width, p1.height, p1.ch, 3);
		cgame.FillRect((INT)p2.x, (INT)p2.y, p2.width, p2.height, p2.ch, bBotPlayer ? 15 : 5);

		if (!bP1Win && !bP2Win) cgame.DrawPixel((INT)ball.x, (INT)ball.y, ball.ch, 15);

		if (bP1Win || bP2Win) {
			if (bP1Win) {
				++wchP1Score;
				szWin = L"Player 1 Wins";
			}
			if (bP2Win) {
				++wchP2Score;
				if (!bBotPlayer) szWin = L"Player 2 Wins";
				else szWin = L"Bot Player Wins";
			}

			ball.x = (FLOAT)(cgame.GetWidth() / 2);
			ball.y = (FLOAT)(cgame.GetHeight() / 2);
			ball.xvel = BALL_VEL * (ball.xvel < 0 || bBotPlayer ? -1 : 1); ball.yvel = 0;

			p1.y = (FLOAT)(cgame.GetHeight() / 2 - nPaddleHeight / 2);
			p2.y = (FLOAT)(cgame.GetHeight() / 2 - nPaddleHeight / 2);

			if (wchP1Score == '9' + 1 || wchP2Score == '9' + 1) {
				wchP1Score = wchP2Score = '0';
				bGameOver = TRUE;
			}
		}

		if (bGameOver) {
			cgame.FillRect(
				cgame.GetWidth() / 2 - (cgame.GetWidth() % 2 == 0 ? 1 : 0),
				cgame.GetHeight() / 2 - (cgame.GetHeight() % 2 == 0 ? 1 : 0),
				1 + (cgame.GetHeight() % 2 == 0 ? 1 : 0),
				3,
				L' ', 0);

			cgame.DrawString(
				cgame.GetWidth() / 2 - szGameOver.size() / 2,
				cgame.GetHeight() / 2 - 5,
				szGameOver.c_str(),
				szGameOver.size(), bP1Win ? 11 : bBotPlayer ? 8 : 13);

			cgame.DrawString(
				cgame.GetWidth() / 2 - szStartMessage.size() / 2,
				cgame.GetHeight() - 7,
				szStartMessage.c_str(),
				szStartMessage.size(), 12);

			cgame.DrawRect(
				cgame.GetWidth() / 2 - szWin.size() / 2 - 3,
				cgame.GetHeight() / 2 - 2,
				szWin.size() + 5,
				5,
				L'+', 14);
		}

		cgame.DrawString(cgame.GetWidth() / 2 - szWin.size() / 2, cgame.GetHeight() / 2, szWin.c_str(), szWin.size(), 14);
		cgame.DrawString(nP1ScoreX, nP1ScoreY, &wchP1Score, 1, 11);
		cgame.DrawString(nP2ScoreX, nP2ScoreY, &wchP2Score, 1, bBotPlayer ? 8 : 13);

		cgame.FlushBuffer();

		if (bP1Win || bP2Win) {
			bP1Win = bP2Win = FALSE;
			szWin = L"";
			std::this_thread::sleep_for(std::chrono::milliseconds(bGameOver ? 2000 : 1500));
			if (bGameOver) while (!(GetAsyncKeyState(VK_SPACE) & 0x8000))
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
			bGameOver = FALSE;
			tp1 = std::chrono::system_clock::now();
		}


		PaddleCheck(&p1, VK_W, VK_S, &cgame);
		if (!bBotPlayer) PaddleCheck(&p2, VK_UP, VK_DOWN, &cgame);
		else BotPaddleCheck(&p2, &ball, &cgame);
		BallCheck(&ball, &p1, &p2, &cgame);

		fpsTime2 = std::chrono::system_clock::now();
		dfpsElapsedTime = fpsTime2 - fpsTime1;
		++nFrameCount;
		if (dfpsElapsedTime.count() > 0.075) {
			fpsTime1 = fpsTime2;
			nFrameRate = (INT)(nFrameCount / dfpsElapsedTime.count());
			nFrameCount = 0;
			cgame.SetTitle("Console Pong   ----------  (" + std::to_string(nFrameRate) + " fps)");
		}
	}


	return 0;
}
