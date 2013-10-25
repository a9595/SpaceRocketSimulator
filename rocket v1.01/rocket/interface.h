#pragma once
#include "header.h"

class InterfaceDlg
{
public:
	InterfaceDlg();

	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static InterfaceDlg* ptr;
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
	void Cls_OnClose(HWND hwnd);
	void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy);
	HWND hGame, hFuel, hLR, hUp, hStatus;
};