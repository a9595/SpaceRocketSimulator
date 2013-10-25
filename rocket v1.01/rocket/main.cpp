#include <windows.h>
#include "resource.h"
#include <tchar.h>
#include <ctime>
#include "rocket.h"
#include <WindowsX.h>
#include <CommCtrl.h>

#ifdef UNICODE
#define SPRINTF wsprintf
#else
#define SPRINTF sprintf
#endif

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK testProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
// ==================== Глобалі для гри(контролів) ===========================
HWND hGame, hFuel, hLR, hUp, hStatus;
// ==================== Глобалі для меню ===========================
TCHAR name[100];
int lives=0, diff=0;
// ==================== Глобалі для ракети і грунту ===========================
void CreateRocketInTheMiddle(HWND);
void CreateGround(int);
void DrawGround(HWND);
void DrawRocket(HWND, int, int, int, RECT);
int isLanded(RECT, int);					// 0 - не приземлились, 1 - вдало приземлились, 2 - погано приземлились (на рівну почву, але трохи швидко), 3 - взірвались нафік
void ChangeRocketView(int);					// 1 - поломаний корабель, 2 - взрив
void DrawFireLeft(HWND, int, RECT);
void DrawFireRight(HWND, int, RECT);
void DrawFireDown(HWND, int, RECT);
void checkRect(RECT*);

Rocket *Rocky;
//=============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	/*INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(ics);
	ics.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&ics);*/
	if(DialogBox(hInstance,MAKEINTRESOURCE(IDD_HELLO), NULL, testProc) == 0)
		return 0;

	Rocky = new Rocket(diff,diff+4);
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc); 
}
BOOL CALLBACK testProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	HWND hName = GetDlgItem(hwnd, IDC_NAME);
	HWND hLives = GetDlgItem(hwnd, IDC_LIVESNUM);
	HWND hDiff = GetDlgItem(hwnd, IDC_DIFF);

	switch(message)
	{
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			SendMessage(hLives, UDM_SETRANGE32, 1, 5);
			HWND hLivesText = GetDlgItem(hwnd, IDC_LIVESTEXT);
			SetWindowText(hLivesText, TEXT("1"));
			//SendMessage(hLivesText, WM_SETTEXT, 
			SendMessage(hDiff, TBM_SETRANGE, true, MAKELPARAM(1,3));
		}
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wp) == IDOK)
		{
			SendMessage(hName, WM_GETTEXT, WPARAM(100), LPARAM(name));
			lives = SendMessage(hLives, UDM_GETPOS32, 0, NULL);
			diff = SendMessage(hDiff, TBM_GETPOS, 0, 0);
			EndDialog(hwnd, 1);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	switch(message)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0); // закрываем модальный диалог
		return TRUE;

	case WM_INITDIALOG:
		{
			// ======== Створення ракети ==============
			CreateRocketInTheMiddle(hWnd); 
			CreateGround(diff);		// <- Підставити рівень складності
			SendMessage(GetDlgItem(hWnd, IDC_FORWARD), TBM_SETRANGE, 1, MAKELPARAM(0,100));
			SendMessage(GetDlgItem(hWnd, IDC_LR), TBM_SETRANGE, 1, MAKELPARAM(-100,100));
			SendMessage(GetDlgItem(hWnd, IDC_FORWARD), TBM_SETPOS, 1, 0);
			DrawGround(hWnd);
			SetTimer(hWnd, 1, 100, 0);
			//получимо дискриптори елементив наших
			hLR = GetDlgItem(hWnd, IDC_LR);
			hUp = GetDlgItem(hWnd, IDC_FORWARD);
			hFuel = GetDlgItem(hWnd, IDC_FUEL);
			hGame = hWnd;

			SendMessage(hLR, TBM_SETRANGE, TRUE, MAKELPARAM(-100, 100));
			SendMessage(hUp, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));

			hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_TOOLTIPS, 0, hGame, WM_USER);
			int parts[5] = {150, 300, 450,600 , -1};
			SendMessage(hStatus, SB_SETPARTS, 5, (LPARAM)parts);

			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) TEXT("speed: 0"));

			TCHAR tlives[12];
			wsprintf(tlives, TEXT("lives: %d "), lives);
			SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) tlives);

			SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM) TEXT("fuel:"));

			SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM) TEXT("engine LR:"));

			SendMessage(hStatus, SB_SETTEXT, 4, (LPARAM) TEXT("engine UP:"));

			hFuel = GetDlgItem(hWnd, IDC_FUEL);
			SendMessage(hFuel, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // Установка интервала для индикатора 
			SendMessage(hFuel, PBM_SETSTEP, 1, 0); // Установка шага приращения  индикатора 
			SendMessage(hFuel, PBM_SETPOS, Rocky->Get_Fuel(), 0); // Установка текущей позиции индикатора
			SendMessage(hFuel, PBM_SETBKCOLOR, 0, LPARAM(RGB(0, 0, 255))); // Установка цвета фона индикатора
			SendMessage(hFuel, PBM_SETBARCOLOR, 0, LPARAM(RGB(255, 0, 255)));  // Установка цвета заполняемых прямоугольников
		}
		return TRUE;

	case WM_TIMER:
		{
			// ======== Спрацювання таймеру ==============

			Rocky->Set_Y_engine(SendMessage(GetDlgItem(hWnd, IDC_FORWARD), TBM_GETPOS, 1, MAKELPARAM(0,100)));	
			Rocky->Set_X_engine(SendMessage(GetDlgItem(hWnd, IDC_LR), TBM_GETPOS, 1, MAKELPARAM(-100,100)));

			int x_engine = Rocky->Get_X_engine();	// <- Підставити x_engine
			//X status
			TCHAR engineLR[30];
			wsprintf(engineLR, TEXT("engine LR %d "), x_engine);
			SendMessage(hStatus, SB_SETTEXT, 3,(LPARAM)engineLR);

			int y_engine = Rocky->Get_Y_engine();	// <- Підставити y_engine
			//Y status
			TCHAR engineUp[30];
			wsprintf(engineUp, TEXT("engine UP: %d "), y_engine);
			SendMessage(hStatus, SB_SETTEXT, 4,(LPARAM)engineUp);

			int fuel = Rocky->Get_Fuel();		// <- Підставити fuel
			TCHAR tfuel[50];
			wsprintf(tfuel, TEXT("fuel: %d "), fuel);
			SendMessage(hStatus, SB_SETTEXT, 2,(LPARAM)tfuel);
			SendMessage(hFuel, PBM_SETPOS, WPARAM(fuel), 0); // Установка текущей позиции индикатора
			int speed = Rocky->Get_Speed();		// <- Підставити speed
			TCHAR tspeed[50];
			wsprintf(tspeed, TEXT("Speed: %d"), speed);
			// Для каждой секции установим текст и всплывающую подсказку
			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) tspeed);

			Rocky->CalculateNext();

			DrawRocket(hWnd, fuel, x_engine, y_engine, Rocky->Get_Cur_Pos());

			int landed_state = isLanded(Rocky->Get_Cur_Pos(), speed);
			ChangeRocketView(landed_state);

			if(landed_state == 1)
			{
				KillTimer(hWnd, 1);
				//MessageBox
				TCHAR tmp[150];
				wsprintf(tmp, TEXT("Congradulations %s, You won whith %d lives"), name, lives);
				MessageBox(0,tmp,TEXT("You won"),MB_OK | MB_ICONINFORMATION);
			}
			else if(landed_state == 2 || landed_state == 3)
			{
				//MessageBox
				KillTimer(hWnd, 1);
				TCHAR tmp[150];
				wsprintf(tmp, TEXT("%s,try again. You have %d lives"), name, lives);
				MessageBox(0,tmp,TEXT("You lose"),MB_OK | MB_ICONINFORMATION);
				if(lives>0)
				{
					EndDialog(hWnd,0);
					delete Rocky;
					Rocky = new Rocket(diff,diff+4);
					for(int i=0; i<3; i++)
					{
						hFireLeft[i] = 0;	
						hFireRight[i] = 0;			
						hFireDown[i] = 0;	
					}

					DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
				}
				else 
				{
					MessageBox(0,TEXT("You lose. Good bye =)"),TEXT("You lose"),MB_OK | MB_ICONINFORMATION);
					EndDialog(hWnd,0);
				}
			}

		}
		return TRUE;

	}
	return FALSE;
}

void CreateRocketInTheMiddle(HWND hWnd)
{
	RECT r = Rocky->Get_Cur_Pos();
	hRocket = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | WS_VISIBLE | SS_ICON | WS_EX_TRANSPARENT, r.left, r.top, rWidth, rHeight, hWnd, 0, GetModuleHandle(0), 0);
	ChangeRocketView(-1);
}

void CreateGround(int iDifficult)
{
	srand(time(0));

	// Заповняємо почву випадковими значеннями
	for(int i = 0; i < gSize; i+=2)
	{
		groundArr[i] = rand() % (iDifficult + 1);
		if(i + 1 < gSize)
			groundArr[i+1] = groundArr[i];
	}

	// Перевіряємо чи є хоча б одне посадочне місце
	int step = (rWidth / gWidth) + 3;
	int iHaveGoodGround = 0;
	for(int i = 0; i < gSize - step; i++)
	{
		bool good = true;
		for(int j = i; j < i + step; j++)
		{
			if(groundArr[j] > 0)
			{
				good = false;
				break;
			}
		}

		if(good)
		{
			iHaveGoodGround++;
			i += step;	// Інакше один довгий участок порахує за декілька
		}
	}

	// Якщо такої немає - створюємо
	for(int i = 3 - (iHaveGoodGround + iDifficult); i > 0; i--)
	{
		int pos = rand() %  (gSize - step * 2);
		for(int j = pos; j < pos + (step + step * 1.25); j++)
			groundArr[j] = 0;
	}
}

void DrawGround(HWND hWnd)
{
	int X = fX0;
	int Y = gY;
	int nWidth = gWidth;
	int nHeight = gWidth;

	for(int i = 0; i < gSize; i++)
	{
		HWND hGround = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | WS_VISIBLE | SS_BITMAP, X, Y, nWidth, nHeight, hWnd, 0, GetModuleHandle(0), 0);
		HBITMAP bmtGround;

		if(groundArr[i] == 0)
			bmtGround = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_GOOD_GRAUND));
		else 
			bmtGround = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_BAD_GRAUND));

		SendMessage(hGround, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmtGround);
		UpdateWindow(hGround);

		X += gWidth;
	}
}

void checkRect(RECT* r)
{
	int iW = r->right - r->left;
	int iH = r->bottom - r->top;

	if(r->top < fY0)
		r->top = fY0;
	else if(r->top > gY - rHeight)
		r->top = gY - rHeight;

	if(r->left < fX0)
		r->left = fX0;
	else if(r->left > fWidth - rWidth + fX0)
		r->left = fWidth - rWidth + fX0;

	r->right = r->left + iW;
	r->bottom = r->top + iH;
}

void DrawRocket(HWND hWnd, int fuel, int x_engine, int y_engine, RECT cur_pos)
{
	checkRect(&cur_pos);

	// Переміщуємо ракету
	MoveWindow(hRocket, cur_pos.left, cur_pos.top, rWidth, rHeight, TRUE);

	// Включаємо двигуни	
	int iFireL = 0;
	int iFireR = 0;
	int iFireD = 0;

	if(fuel != 0 && cur_pos.bottom < gY)
	{
		if(y_engine > 0) 
			iFireD = (y_engine / 34) + 1;

		if(x_engine > 0)
			iFireR = (x_engine / 34) + 1;
		else if(x_engine < 0)
			iFireL = (abs(x_engine) / 34) + 1;


	}

	DrawFireLeft(hWnd, iFireL, cur_pos);
	DrawFireRight(hWnd, iFireR, cur_pos);
	DrawFireDown(hWnd, iFireD, cur_pos);
}

void DrawFireLeft(HWND hWnd, int p, RECT cur_pos)
{
	int const imgW[3] = {10, 18, 30};
	int const imgH[3] = {14, 14, 14};
	LPCWSTR const IMGS[3] = {MAKEINTRESOURCE(FIRE_LEFT1), MAKEINTRESOURCE(FIRE_LEFT2), MAKEINTRESOURCE(FIRE_LEFT3)};

	// Якщо потрібно створити бітмап - створюємо
	if(hFireLeft[0] == 0)
	{
		for(int i = 0; i < 3; i++)
		{
			hFireLeft[i] = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | SS_BITMAP , 0, 0, imgW[i], imgH[i], hWnd, 0, GetModuleHandle(0), 0);
			HBITMAP fireBMP;			
			fireBMP = LoadBitmap(GetModuleHandle(0), IMGS[i]);
			SendMessage(hFireLeft[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)fireBMP);
		}
	}

	// Прячимо всі вогні крім потрібного нам
	for(int i = 0; i < 3; i++)
	{
		if(i != p-1 && IsWindowVisible(hFireLeft[i]) == TRUE)
			ShowWindow(hFireLeft[i], SW_HIDE);
	}

	if(p == 0)
		return;

	// відображаємо вогонь
	if(IsWindowVisible(hFireLeft[p-1]) == FALSE)
		ShowWindow(hFireLeft[p-1], SW_SHOW);

	// Переміщаємо його
	MoveWindow(hFireLeft[p-1], cur_pos.left - imgW[p-1], cur_pos.bottom - 11, imgW[p-1], imgH[p-1], TRUE);
}

void DrawFireRight(HWND hWnd, int p, RECT cur_pos)
{
	int const imgW[3] = {10, 18, 30};
	int const imgH[3] = {14, 14, 14};
	LPCWSTR const IMGS[3] = {MAKEINTRESOURCE(FIRE_RIGHT1), MAKEINTRESOURCE(FIRE_RIGHT2), MAKEINTRESOURCE(FIRE_RIGHT3)};

	// Якщо потрібно створити бітмап - створюємо
	if(hFireRight[0] == 0)
	{
		for(int i = 0; i < 3; i++)
		{
			hFireRight[i] = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | SS_BITMAP , 0, 0, imgW[i], imgH[i], hWnd, 0, GetModuleHandle(0), 0);
			HBITMAP fireBMP;			
			fireBMP = LoadBitmap(GetModuleHandle(0), IMGS[i]);
			SendMessage(hFireRight[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)fireBMP);
		}
	}

	// Прячимо всі вогні крім потрібного нам
	for(int i = 0; i < 3; i++)
	{
		if(i != p-1 && IsWindowVisible(hFireRight[i]) == TRUE)
			ShowWindow(hFireRight[i], SW_HIDE);
	}

	if(p == 0)
		return;

	// відображаємо вогонь
	if(IsWindowVisible(hFireRight[p-1]) == FALSE)
		ShowWindow(hFireRight[p-1], SW_SHOW);

	// Переміщаємо його
	MoveWindow(hFireRight[p-1], cur_pos.right, cur_pos.bottom - 11, imgW[p-1], imgH[p-1], TRUE);
}

void DrawFireDown(HWND hWnd, int p, RECT cur_pos)
{
	int const imgH[3] = {10, 18, 30};
	int const imgW[3] = {14, 14, 14};
	LPCWSTR const IMGS[3] = {MAKEINTRESOURCE(FIRE_DOWN1), MAKEINTRESOURCE(FIRE_DOWN2), MAKEINTRESOURCE(FIRE_DOWN3)};

	// Якщо потрібно створити бітмап - створюємо
	if(hFireDown[0] == 0)
	{
		for(int i = 0; i < 3; i++)
		{
			hFireDown[i] = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | SS_BITMAP , 0, 0, imgW[i], imgH[i], hWnd, 0, GetModuleHandle(0), 0);
			HBITMAP fireBMP;			
			fireBMP = LoadBitmap(GetModuleHandle(0), IMGS[i]);
			SendMessage(hFireDown[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)fireBMP);
		}
	}

	// Прячимо всі вогні крім потрібного нам
	for(int i = 0; i < 3; i++)
	{
		if(i != p-1 && IsWindowVisible(hFireDown[i]) == TRUE)
			ShowWindow(hFireDown[i], SW_HIDE);
	}

	if(p == 0)
		return;

	// відображаємо вогонь
	if(IsWindowVisible(hFireDown[p-1]) == FALSE)
		ShowWindow(hFireDown[p-1], SW_SHOW);

	// Переміщаємо його
	MoveWindow(hFireDown[p-1], cur_pos.left + 8, cur_pos.bottom, imgW[p-1], imgH[p-1], TRUE);
}

void ChangeRocketView(int t)
{
	if(t == 2)
	{
		HICON rocketIcom= LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_BROKEN));
		SendMessage(hRocket, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)rocketIcom);
		UpdateWindow(hRocket);
	} else if(t == 3) {
		HICON rocketIcom= LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_EXPLOSION));
		SendMessage(hRocket, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)rocketIcom);
		UpdateWindow(hRocket);
	} else {
		HICON rocketIcom= LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ROCKET));
		SendMessage(hRocket, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)rocketIcom);
		UpdateWindow(hRocket);
	}
}

int isLanded(RECT r, int speed)
{
	bool landed = r.bottom >= gY;

	if(landed)
	{
		bool onGoodGround = true;
		for(int i = r.left + 2; i < r.right - 2; i++)
		{
			int iGroundIndx = (i - fY0) / gWidth;
			if(iGroundIndx >= gSize || groundArr[iGroundIndx] != 0)
			{
				onGoodGround = false;
				break;
			}

			if(!onGoodGround)
				break;
		}

		// Погана почва - взрив!
		if(!onGoodGround)
		{
			lives--;
			TCHAR tlives[12];
			wsprintf(tlives, TEXT("lives: %d "), lives);
			SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) tlives);
			return 3;
		}

		if(speed > iMaxLandingSpeed)
		{
			lives--;
			TCHAR tlives[12];
			wsprintf(tlives, TEXT("lives: %d "), lives);
			SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) tlives);
			return 2; // Поломка - занадто швидко
		}
		else
		{
			return 1; // Ура :)
		}
	}

	// Ще не приземлилися
	return 0;
}
