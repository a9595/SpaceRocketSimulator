#include "interface.h"

InterfaceDlg* InterfaceDlg::ptr = NULL;

InterfaceDlg::InterfaceDlg()
{
	ptr = this;
}

void InterfaceDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL InterfaceDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	//получимо дискриптори елементив наших
	hLR = GetDlgItem(hwnd, IDC_LR);
	hUp = GetDlgItem(hwnd, IDC_FORWARD);
	hFuel = GetDlgItem(hwnd, IDC_FUEL);
	hGame = hwnd;

	SendMessage(hLR, TBM_SETRANGE, TRUE, MAKELPARAM(-100, 100));
	SendMessage(hUp, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));

	hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_TOOLTIPS, 0, hGame, WM_USER);
	int parts[5] = {150, 300, 450,600 , -1};
	SendMessage(hStatus, SB_SETPARTS, 5, (LPARAM)parts);

	// Для каждой секции установим текст и всплывающую подсказку
	SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) TEXT("speed: 0"));

	SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) TEXT("altitude: 0"));

	SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM) TEXT("fuel:"));

	SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM) TEXT("engine LR:"));

	SendMessage(hStatus, SB_SETTEXT, 4, (LPARAM) TEXT("engine UP:"));

	return TRUE;
}

void InterfaceDlg::Cls_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{

	if(hwndCtl == hLR)
	{
		int nCurrrentPosition = SendMessage(hwndCtl, TBM_GETPOS, TRUE, MAKELPARAM(-100, 100));
		TCHAR engineLR[30];
		wsprintf(engineLR, TEXT("engine LR %d "), nCurrrentPosition);
		// отобразим состояние счётчика в строке состояния
		SendMessage(hStatus, SB_SETTEXT, 3,(LPARAM)engineLR);
	}
	else if(hwndCtl == hUp)
	{
		int nCurrrentPosition = SendMessage(hwndCtl, TBM_GETPOS, TRUE, MAKELPARAM(0, 100));
		TCHAR engineUp[30];
		wsprintf(engineUp, TEXT("engine UP: %d "), nCurrrentPosition);
		// отобразим состояние счётчика в строке состояния
		SendMessage(hStatus, SB_SETTEXT, 4,(LPARAM)engineUp);
	}
}


void InterfaceDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{

}
void InterfaceDlg::Cls_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	// установим размер строки состояния, равный ширине клиентской области главного окна
	SendMessage(hStatus, WM_SIZE, 0, 0);
}

BOOL CALLBACK InterfaceDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
		HANDLE_MSG(hwnd, WM_SIZE, ptr->Cls_OnSize);
		HANDLE_MSG(hwnd, WM_HSCROLL, ptr->Cls_OnHScroll);
	}
	return FALSE;
}