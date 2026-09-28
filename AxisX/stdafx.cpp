/*

 **********************************************************************
 *
 * Original Axis by:
 * Copyright (C) Philip A. Esterle 1998-2002 + (C) parts Adron 2002
 *
 * 55r,56(x) Mods, and Axis2 re-build by:
 * Copyright (C) Benoit Croussette 2004-2006
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 **********************************************************************

*/

// stdafx.cpp : source file that includes just the standard includes
//	AxisX.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "AxisX.h"
#include "AxisLog.h"
#include "ClientInfo.h"
#include "RemoteConsole.h"
#include "ScriptObjects.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <DLGS.H>
#include <WINUSER.H>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
#include <uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
#include <dwmapi.h>
#include <shlobj.h>		// SHGetFolderPath
#pragma comment(lib, "Dwmapi.lib")
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

HWND hwndUOClient;
HWND hwndHoGInstance;

CString csUOPath;
CString csMulPath;
CString csProfilePath;
CStringArray saClientPaths;
CStringArray saMulPaths;
HKEY hRegLocation;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK EnumInstanceProc(HWND hWnd, LPARAM lParam);

// Client executables recognized by process image, in addition to window-title matching.
static LPCTSTR g_UOClientExeNames[] =
{
	_T("client.exe"),
	_T("classicuo.exe"),
};

static bool IsKnownUOClientWindow(HWND hWnd)
{
	DWORD dwPid = 0;
	if (GetWindowThreadProcessId(hWnd, &dwPid) == 0 || dwPid == 0)
		return false;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
	if (hProcess == NULL)
		return false;

	TCHAR szPath[MAX_PATH] = { 0 };
	bool bMatch = false;
	if (GetModuleFileNameEx(hProcess, NULL, szPath, MAX_PATH) > 0)
	{
		CString csExeName(szPath);
		int iSlash = csExeName.ReverseFind(_T('\\'));
		if (iSlash != -1)
			csExeName = csExeName.Mid(iSlash + 1);

		for (int i = 0; i < sizeof(g_UOClientExeNames) / sizeof(g_UOClientExeNames[0]); i++)
		{
			if (csExeName.CompareNoCase(g_UOClientExeNames[i]) == 0)
			{
				bMatch = true;
				break;
			}
		}
	}
	CloseHandle(hProcess);
	return bMatch;
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if ( !lParam )
	{
		hwndUOClient = NULL;
		return TRUE;
	}

	// Only top-level, visible windows with a caption are plausible game-client candidates.
	if (!IsWindowVisible(hWnd) || GetWindow(hWnd, GW_OWNER) != NULL)
		return TRUE;

	CWnd * pWnd = CWnd::FromHandle(hWnd);
	CString csTitle;
	pWnd->GetWindowText(csTitle);

	bool bTitleMatch = false;
	if (!csTitle.IsEmpty())
	{
		CString csTitleLower(csTitle);
		csTitleLower.MakeLower();

		CString csCustomLower(Main->m_csUOTitle);
		csCustomLower.MakeLower();

		bTitleMatch = (csTitleLower.Find(_T("ultima online")) != -1)
			|| (csTitleLower.Find(_T("uosa")) != -1)
			|| (!csCustomLower.IsEmpty() && csTitleLower.Find(csCustomLower) != -1);
	}

	if (bTitleMatch || IsKnownUOClientWindow(hWnd))
	{
		hwndUOClient = hWnd;
		return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK EnumInstanceProc(HWND hWnd, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	CWnd * pWnd = CWnd::FromHandle(hWnd);
	CString csTitle;
	pWnd->GetWindowText(csTitle);
	if (csTitle.Find(Main->GetVersionTitle()) != -1)
	{
		hwndHoGInstance = hWnd;
		return FALSE;
	}
	else
		return TRUE;
}

void AjustComboBox(CComboBox* pmyComboBox)
{
	CString str;
	CSize sz;
	int dx = 0;
	TEXTMETRIC tm;
	CDC * pDC = pmyComboBox->GetDC();
	CFont * pFont = pmyComboBox->GetFont();

	CFont* pOldFont = pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tm);

	for (int i = 0; i < pmyComboBox->GetCount(); i++)
	{
	   pmyComboBox->GetLBText(i, str);
	   sz = pDC->GetTextExtent(str);

	   sz.cx += tm.tmAveCharWidth;
   
	  if (sz.cx > dx)
	     dx = sz.cx;
	}

	pDC->SelectObject(pOldFont);
	pmyComboBox->ReleaseDC(pDC);

	dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2*::GetSystemMetrics(SM_CXEDGE);

	pmyComboBox->SetDroppedWidth(dx);
}


/////////////////////////////////////////////////////////////////////////////
// Send UO commands

// Commands are typed into the focused client via SendInput: clear the chat line, type the
// text as Unicode input, then press Enter.

static const DWORD SEND_KEY_DELAY_MS = 8;	// Delay per key; SDL clients read input once per frame

static void SendVirtualKey(WORD vk, bool bUp)
{
	INPUT in = {};
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = vk;
	in.ki.wScan = (WORD) MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	in.ki.dwFlags = bUp ? KEYEVENTF_KEYUP : 0;
	SendInput(1, &in, sizeof(INPUT));
}

static void TapVirtualKey(WORD vk)
{
	SendVirtualKey(vk, false);
	SendVirtualKey(vk, true);
	Sleep(SEND_KEY_DELAY_MS);
}

static void SendUnicodeChar(wchar_t ch)
{
	INPUT in[2] = {};
	in[0].type = INPUT_KEYBOARD;
	in[0].ki.wScan = ch;
	in[0].ki.dwFlags = KEYEVENTF_UNICODE;
	in[1] = in[0];
	in[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	SendInput(2, in, sizeof(INPUT));
	Sleep(SEND_KEY_DELAY_MS);
}

// Brings the client to the foreground and waits until Windows reports it as foreground window.
static bool BringClientToFront(HWND hWnd)
{
	if (IsIconic(hWnd))
		ShowWindow(hWnd, SW_RESTORE);
	if (GetForegroundWindow() == hWnd)
		return true;

	DWORD dwMyThread = GetCurrentThreadId();
	DWORD dwFgThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	bool bAttached = dwFgThread != 0 && dwFgThread != dwMyThread && AttachThreadInput(dwMyThread, dwFgThread, TRUE);
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);
	if (bAttached)
		AttachThreadInput(dwMyThread, dwFgThread, FALSE);

	for (int i = 0; i < 30 && GetForegroundWindow() != hWnd; i++)
		Sleep(10);
	return GetForegroundWindow() == hWnd;
}

static bool g_bSendingToUO = false;

bool SendToUO(CString Cmd)
{
	EnumWindows(EnumWindowsProc, 1);
	if (!hwndUOClient || !IsWindow(hwndUOClient))
	{
		EnumWindows(EnumWindowsProc, 1);
	}
	if (!hwndUOClient || !IsWindow(hwndUOClient))
	{
		AxisSetStatus(AXT("Kein UO-Client gefunden - Befehl nicht gesendet: ") + Cmd, 3);
		return false;
	}

	// Ignore a second command while the first one is still being typed.
	if (g_bSendingToUO)
		return false;
	g_bSendingToUO = true;

	int iLen = MultiByteToWideChar(CP_ACP, 0, Cmd, Cmd.GetLength(), NULL, 0);
	CStringW wCmd;
	if (iLen > 0)
	{
		MultiByteToWideChar(CP_ACP, 0, Cmd, Cmd.GetLength(), wCmd.GetBuffer(iLen), iLen);
		wCmd.ReleaseBuffer(iLen);
	}

	if (BringClientToFront(hwndUOClient))
	{
		Sleep(60);	// Give the client a frame to process the focus change

		// Release modifier keys the user may still be holding.
		SendVirtualKey(VK_SHIFT, true);
		SendVirtualKey(VK_CONTROL, true);
		SendVirtualKey(VK_MENU, true);

		// Clear the chat line: select all and delete.
		SendVirtualKey(VK_CONTROL, false);
		TapVirtualKey('A');
		SendVirtualKey(VK_CONTROL, true);
		TapVirtualKey(VK_BACK);

		for (int i = 0; i < wCmd.GetLength(); i++)
			SendUnicodeChar(wCmd[i]);

		TapVirtualKey(VK_RETURN);
		AxisSetStatus(AXT("Gesendet: ") + Cmd, 1);
	}
	else
	{
		AxisSetStatus(AXT("Gesendet ohne Fokus (Client liess sich nicht nach vorne holen): ") + Cmd, 2);
		// No focus: post characters only; key messages cause doubled characters in SDL clients.
		for (int i = 0; i < wCmd.GetLength(); i++)
		{
			PostMessageW(hwndUOClient, WM_CHAR, wCmd[i], 1);
			Sleep(SEND_KEY_DELAY_MS);
		}
		PostMessageW(hwndUOClient, WM_KEYDOWN, VK_RETURN, 1);
		PostMessageW(hwndUOClient, WM_CHAR, L'\r', 1);
		PostMessageW(hwndUOClient, WM_KEYUP, VK_RETURN, 1 | (1 << 30) | (1u << 31));
	}

	g_bSendingToUO = false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//Misc

// Formats a Win32 error code (use instead of strerror_s, which expects errno values).
void FormatWin32Error(DWORD dwError, char* szBuffer, size_t bufferSize)
{
	if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, szBuffer, (DWORD) bufferSize, NULL) == 0)
	{
		sprintf_s(szBuffer, bufferSize, "Error %lu", dwError);
		return;
	}
	size_t len = strlen(szBuffer);
	while (len > 0 && (szBuffer[len - 1] == '\r' || szBuffer[len - 1] == '\n' || szBuffer[len - 1] == ' ' || szBuffer[len - 1] == '.'))
		szBuffer[--len] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
// Theming (light/dark palette, owner-drawn controls)

static bool g_bDarkModeBootstrapped = false;

void InstallDarkDialogHook();

// Loads the theme and opts the process into dark or light common controls
// (uxtheme ordinal 135, SetPreferredAppMode; skipped if unavailable).
void BootstrapDarkMode()
{
	if (g_bDarkModeBootstrapped)
		return;
	g_bDarkModeBootstrapped = true;
	AxisLoadTheme();
	InstallDarkDialogHook();	// Themes every popup dialog opened later

	HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (hUxtheme == NULL)
		return;

	typedef void (WINAPI *PFN_SetPreferredAppMode)(int);
	PFN_SetPreferredAppMode pSetPreferredAppMode =
		(PFN_SetPreferredAppMode) GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
	if (pSetPreferredAppMode != NULL)
		pSetPreferredAppMode(AxisLightTheme() ? 3 : 2); // ForceLight / ForceDark

	typedef void (WINAPI *PFN_FlushMenuThemes)();
	PFN_FlushMenuThemes pFlushMenuThemes =
		(PFN_FlushMenuThemes) GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136));
	if (pFlushMenuThemes != NULL)
		pFlushMenuThemes();
}

// Owner-drawn flat push button (text buttons only; icon buttons keep native theming).
static LRESULT CALLBACK AxisButtonSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	static LPCTSTR pszHoverProp = _T("AxisBtnHover");
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rc;
			GetClientRect(hWnd, &rc);

			LRESULT state = ::SendMessage(hWnd, BM_GETSTATE, 0, 0);
			BOOL bPressed = (state & BST_PUSHED) != 0;
			BOOL bFocus = (state & BST_FOCUS) != 0;
			BOOL bHover = (GetProp(hWnd, pszHoverProp) != NULL);
			BOOL bEnabled = IsWindowEnabled(hWnd);
			// Primary buttons are marked per page via AxisMarkPrimary (BS_DEFPUSHBUTTON follows focus).
			BOOL bPrimary = (GetProp(hWnd, _T("AxisPrimary")) != NULL);

			COLORREF crBorder, crFill, crText;
			if (bPrimary)
			{
				crBorder = DarkAccentColor();
				crFill = bPressed ? AxisClr(AXC_ACCENT_PRESSED) : (bHover ? AxisClr(AXC_ACCENT_HOVER) : DarkAccentColor());
				crText = AxisClr(AXC_ON_ACCENT);
			}
			else
			{
				crBorder = (bHover || bFocus) ? DarkAccentColor() : AxisClr(AXC_BORDER);
				crFill = bPressed ? AxisClr(AXC_BUTTON_PRESSED) : (bHover ? AxisClr(AXC_BUTTON_HOVER) : AxisClr(AXC_BUTTON));
				crText = !bEnabled ? AxisClr(AXC_DISABLED) : ((bHover || bFocus) ? AxisAccentTextColor() : DarkTextColor());
			}

			// Fill the corners outside the rounded rect with the parent background
			// (tab controls return no brush, so use the page color there).
			TCHAR szParentClass[32] = { 0 };
			GetClassName(GetParent(hWnd), szParentClass, 32);
			if (_tcsicmp(szParentClass, _T("SysTabControl32")) == 0)
			{
				HBRUSH hPage = CreateSolidBrush(DarkPageBkColor());
				FillRect(hdc, &rc, hPage);
				DeleteObject(hPage);
			}
			else
			{
				HBRUSH hParentBg = (HBRUSH) ::SendMessage(GetParent(hWnd), WM_CTLCOLORBTN, (WPARAM) hdc, (LPARAM) hWnd);
				if (hParentBg != NULL)
					FillRect(hdc, &rc, hParentBg);
			}

			HBRUSH hFill = CreateSolidBrush(crFill);
			HPEN hPen = CreatePen(PS_SOLID, 1, crBorder);
			HGDIOBJ hOldBrush = SelectObject(hdc, hFill);
			HGDIOBJ hOldPen = SelectObject(hdc, hPen);
			RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			DeleteObject(hFill);
			DeleteObject(hPen);

			TCHAR szText[256];
			GetWindowText(hWnd, szText, 256);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, crText);
			HFONT hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
			HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
			DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			if (hOldFont)
				SelectObject(hdc, hOldFont);

			if (bFocus)
			{
				RECT rcFocus = rc;
				InflateRect(&rcFocus, -3, -3);
				DrawFocusRect(hdc, &rcFocus);
			}

			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_MOUSEMOVE:
		if (GetProp(hWnd, pszHoverProp) == NULL)
		{
			SetProp(hWnd, pszHoverProp, (HANDLE) 1);
			TRACKMOUSEEVENT tme;
			memset(&tme, 0, sizeof(tme));
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_MOUSELEAVE:
		RemoveProp(hWnd, pszHoverProp);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ENABLE:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			InvalidateRect(hWnd, NULL, FALSE);
			return lRes;
		}
	case WM_NCDESTROY:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			RemoveWindowSubclass(hWnd, AxisButtonSubclassProc, uIdSubclass);
			return lRes;
		}
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Owner-drawn group box: rounded border with the label cut into the top edge.
static LRESULT CALLBACK AxisGroupBoxSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rc;
			GetClientRect(hWnd, &rc);

			// The frame overlaps its sibling controls; exclude them so they are not painted over.
			int iSaved = SaveDC(hdc);
			HWND hParent = GetParent(hWnd);
			for (HWND hSib = GetWindow(hParent, GW_CHILD); hSib != NULL; hSib = GetWindow(hSib, GW_HWNDNEXT))
			{
				if (hSib == hWnd || !IsWindowVisible(hSib))
					continue;
				RECT rcSib;
				GetWindowRect(hSib, &rcSib);
				MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT) &rcSib, 2);
				RECT rcHit;
				if (IntersectRect(&rcHit, &rcSib, &rc))
				{
					// Do not exclude enclosing frames.
					if (rcSib.left <= rc.left && rcSib.top <= rc.top && rcSib.right >= rc.right && rcSib.bottom >= rc.bottom)
						continue;
					ExcludeClipRect(hdc, rcSib.left, rcSib.top, rcSib.right, rcSib.bottom);
				}
			}
			HBRUSH hBg = CreateSolidBrush(DarkPageBkColor());
			FillRect(hdc, &rc, hBg);
			DeleteObject(hBg);
			RestoreDC(hdc, iSaved);

			TCHAR szText[256];
			int nTextLen = GetWindowText(hWnd, szText, 256);
			HFONT hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
			HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
			SIZE sz;
			sz.cx = 0; sz.cy = 14;
			if (nTextLen > 0)
				GetTextExtentPoint32(hdc, szText, nTextLen, &sz);

			RECT rcBorder = rc;
			rcBorder.top += sz.cy / 2;

			HPEN hPen = CreatePen(PS_SOLID, 1, AxisClr(AXC_BORDER));
			HGDIOBJ hOldPen = SelectObject(hdc, hPen);
			HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
			RoundRect(hdc, rcBorder.left, rcBorder.top, rcBorder.right, rcBorder.bottom, 10, 10);
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);

			if (nTextLen > 0)
			{
				RECT rcLabel;
				rcLabel.left = rc.left + 10;
				rcLabel.top = rc.top;
				rcLabel.right = rcLabel.left + sz.cx + 8;
				rcLabel.bottom = rc.top + sz.cy;
				HBRUSH hLabelBg = CreateSolidBrush(DarkPageBkColor());
				FillRect(hdc, &rcLabel, hLabelBg);
				DeleteObject(hLabelBg);
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, AxisAccentTextColor());
				RECT rcText = rcLabel;
				rcText.left += 4;
				DrawText(hdc, szText, nTextLen, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
			}
			if (hOldFont)
				SelectObject(hdc, hOldFont);

			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_NCDESTROY:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			RemoveWindowSubclass(hWnd, AxisGroupBoxSubclassProc, uIdSubclass);
			return lRes;
		}
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Owner-drawn tab control (the native theme has no dark tabs); selected tab is underlined in the accent color.
static LRESULT CALLBACK AxisTabSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			HBRUSH hPage = CreateSolidBrush(DarkPageBkColor());
			FillRect(hdc, &rcClient, hPage);
			DeleteObject(hPage);

			HFONT hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
			HFONT hOldFont = hFont ? (HFONT) SelectObject(hdc, hFont) : NULL;
			SetBkMode(hdc, TRANSPARENT);
			int iSel = TabCtrl_GetCurSel(hWnd);
			int iCount = TabCtrl_GetItemCount(hWnd);
			RECT rcRow = { 0, 0, 0, 0 };
			for (int i = 0; i < iCount; i++)
			{
				RECT rc;
				TabCtrl_GetItemRect(hWnd, i, &rc);
				UnionRect(&rcRow, &rcRow, &rc);
				TCHAR szText[128] = { 0 };
				TCITEM item;
				item.mask = TCIF_TEXT;
				item.pszText = szText;
				item.cchTextMax = 127;
				TabCtrl_GetItem(hWnd, i, &item);
				bool bSel = (i == iSel);
				HBRUSH hBr = CreateSolidBrush(bSel ? DarkFieldBkColor() : DarkPageBkColor());
				FillRect(hdc, &rc, hBr);
				DeleteObject(hBr);
				if (bSel)
				{
					RECT rcLine = { rc.left, rc.bottom - 2, rc.right, rc.bottom };
					HBRUSH hGold = CreateSolidBrush(DarkAccentColor());
					FillRect(hdc, &rcLine, hGold);
					DeleteObject(hGold);
				}
				SetTextColor(hdc, bSel ? AxisAccentTextColor() : DarkTextColor());
				DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
			}
			// Separator line below the tab row
			RECT rcSep = { rcClient.left, rcRow.bottom, rcClient.right, rcRow.bottom + 1 };
			HBRUSH hSep = CreateSolidBrush(AxisClr(AXC_SEPARATOR));
			FillRect(hdc, &rcSep, hSep);
			DeleteObject(hSep);
			if (hOldFont)
				SelectObject(hdc, hOldFont);
			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, AxisTabSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}
// Owner-drawn check boxes, radio buttons and disabled labels (native drawing ignores text colors).

static COLORREF AxisDimTextColor() { return AxisClr(AXC_DIM); }

static int AxisScale(HDC hdc, int iPixels)
{
	return MulDiv(iPixels, GetDeviceCaps(hdc, LOGPIXELSY), 96);
}

// Draws a check box or radio circle in rcBox; iState 0 = unchecked, 1 = checked, 2 = indeterminate.
static void DrawAxisCheckMark(HDC hdc, const RECT & rcBox, int iState, bool bEnabled, bool bRadio, COLORREF crFill)
{
	COLORREF crBorder = bEnabled ? (iState ? DarkAccentColor() : AxisClr(AXC_CHECKBORDER)) : AxisClr(AXC_CHECKBORDER_OFF);
	HPEN hPen = CreatePen(PS_SOLID, 1, crBorder);
	HBRUSH hFill = CreateSolidBrush((iState && !bRadio && bEnabled) ? DarkAccentColor() : crFill);
	HGDIOBJ hOldPen = SelectObject(hdc, hPen);
	HGDIOBJ hOldBrush = SelectObject(hdc, hFill);
	if (bRadio)
		Ellipse(hdc, rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
	else
		RoundRect(hdc, rcBox.left, rcBox.top, rcBox.right, rcBox.bottom, 3, 3);
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hFill);
	DeleteObject(hPen);

	int w = rcBox.right - rcBox.left;
	if (bRadio && iState)
	{
		RECT rcDot = rcBox;
		InflateRect(&rcDot, -(w / 4 + 1), -(w / 4 + 1));
		HBRUSH hDot = CreateSolidBrush(bEnabled ? DarkAccentColor() : AxisDimTextColor());
		HGDIOBJ hOldDot = SelectObject(hdc, hDot);
		HGDIOBJ hNoPen = SelectObject(hdc, GetStockObject(NULL_PEN));
		Ellipse(hdc, rcDot.left, rcDot.top, rcDot.right + 1, rcDot.bottom + 1);
		SelectObject(hdc, hNoPen);
		SelectObject(hdc, hOldDot);
		DeleteObject(hDot);
	}
	else if (iState == 1)
	{
		// Check mark
		COLORREF crMark = bEnabled ? AxisClr(AXC_ON_ACCENT) : AxisDimTextColor();
		HPEN hMark = CreatePen(PS_SOLID, max(2, w / 7), crMark);
		HGDIOBJ hOldMark = SelectObject(hdc, hMark);
		POINT pts[3] = {
			{ rcBox.left + w * 22 / 100, rcBox.top + w * 52 / 100 },
			{ rcBox.left + w * 42 / 100, rcBox.top + w * 72 / 100 },
			{ rcBox.left + w * 78 / 100, rcBox.top + w * 30 / 100 } };
		Polyline(hdc, pts, 3);
		SelectObject(hdc, hOldMark);
		DeleteObject(hMark);
	}
	else if (iState == 2)
	{
		RECT rcInd = rcBox;
		InflateRect(&rcInd, -(w / 4), -(w / 4));
		HBRUSH hInd = CreateSolidBrush(bEnabled ? DarkAccentColor() : AxisDimTextColor());
		FillRect(hdc, &rcInd, hInd);
		DeleteObject(hInd);
	}
}

static void PaintAxisCheckButton(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	HWND hParent = GetParent(hWnd);
	HBRUSH hBk = (HBRUSH) ::SendMessage(hParent, WM_CTLCOLORSTATIC, (WPARAM) hdc, (LPARAM) hWnd);
	if (hBk == NULL)
		hBk = (HBRUSH) GetStockObject(BLACK_BRUSH);
	FillRect(hdc, &rc, hBk);

	LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	LONG_PTR lType = lStyle & 0x0F;
	bool bRadio = (lType == BS_RADIOBUTTON || lType == BS_AUTORADIOBUTTON);
	bool bEnabled = IsWindowEnabled(hWnd) != FALSE;
	LRESULT lCheck = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
	int iState = (lCheck == BST_CHECKED) ? 1 : (lCheck == BST_INDETERMINATE ? 2 : 0);

	int iBox = AxisScale(hdc, 14);
	if (iBox > rc.bottom - rc.top)
		iBox = rc.bottom - rc.top;
	bool bBoxRight = (lStyle & BS_LEFTTEXT) != 0;
	RECT rcBox;
	rcBox.top = rc.top + ((rc.bottom - rc.top) - iBox) / 2;
	rcBox.bottom = rcBox.top + iBox;
	rcBox.left = bBoxRight ? rc.right - iBox : rc.left;
	rcBox.right = rcBox.left + iBox;
	DrawAxisCheckMark(hdc, rcBox, iState, bEnabled, bRadio, DarkFieldBkColor());

	TCHAR szText[256] = { 0 };
	GetWindowText(hWnd, szText, 255);
	RECT rcText = rc;
	int iGap = AxisScale(hdc, 6);
	if (bBoxRight)
		rcText.right = rcBox.left - iGap;
	else
		rcText.left = rcBox.right + iGap;
	HFONT hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
	HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, bEnabled ? DarkTextColor() : AxisDimTextColor());
	UINT uFormat = DT_VCENTER | DT_HIDEPREFIX | ((lStyle & BS_MULTILINE) ? DT_WORDBREAK : DT_SINGLELINE | DT_END_ELLIPSIS);
	if (bBoxRight)
		uFormat |= DT_RIGHT;
	if (lStyle & BS_MULTILINE)
	{
		// Center multi-line text vertically.
		RECT rcCalc = rcText;
		int iHeight = DrawText(hdc, szText, -1, &rcCalc, uFormat | DT_CALCRECT);
		rcText.top += ((rcText.bottom - rcText.top) - iHeight) / 2;
	}
	DrawText(hdc, szText, -1, &rcText, uFormat);
	if (GetFocus() == hWnd && szText[0] != 0)
	{
		RECT rcFocus = rcText;
		DrawText(hdc, szText, -1, &rcFocus, uFormat | DT_CALCRECT);
		if (!(lStyle & BS_MULTILINE))
		{
			int iH = rcFocus.bottom - rcFocus.top;
			rcFocus.top = rc.top + ((rc.bottom - rc.top) - iH) / 2;
			rcFocus.bottom = rcFocus.top + iH;
		}
		InflateRect(&rcFocus, 2, 1);
		SetTextColor(hdc, AxisAccentTextColor());
		DrawFocusRect(hdc, &rcFocus);
	}
	if (hOldFont)
		SelectObject(hdc, hOldFont);
}

// Subclass procedure for owner-drawn check boxes and radio buttons.
static LRESULT CALLBACK AxisCheckSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			PaintAxisCheckButton(hWnd, hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_PRINTCLIENT:
		PaintAxisCheckButton(hWnd, (HDC) wParam);
		return 0;
	case BM_SETCHECK:
	case BM_SETSTATE:
	case BM_CLICK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_ENABLE:
	case WM_SETTEXT:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_UPDATEUISTATE:
		{
			// The button paints itself natively on these messages; repaint it right away.
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			if (IsWindow(hWnd))
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
			return lRes;
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, AxisCheckSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Draws a disabled static label in the dimmed text color.
static void PaintAxisDisabledStatic(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	HBRUSH hBk = (HBRUSH) ::SendMessage(GetParent(hWnd), WM_CTLCOLORSTATIC, (WPARAM) hdc, (LPARAM) hWnd);
	if (hBk != NULL)
		FillRect(hdc, &rc, hBk);
	LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	LONG_PTR lType = lStyle & SS_TYPEMASK;
	UINT uFormat = DT_EXPANDTABS;
	if (lType == SS_CENTER)
		uFormat |= DT_CENTER;
	else if (lType == SS_RIGHT)
		uFormat |= DT_RIGHT;
	if (lType == SS_LEFTNOWORDWRAP || lType == SS_SIMPLE || (lStyle & SS_CENTERIMAGE))
		uFormat |= DT_SINGLELINE;
	else
		uFormat |= DT_WORDBREAK;
	if (lStyle & SS_CENTERIMAGE)
		uFormat |= DT_VCENTER;
	if (lStyle & SS_NOPREFIX)
		uFormat |= DT_NOPREFIX;
	if ((lStyle & SS_ELLIPSISMASK) == SS_ENDELLIPSIS)
		uFormat |= DT_END_ELLIPSIS;
	else if ((lStyle & SS_ELLIPSISMASK) == SS_PATHELLIPSIS)
		uFormat |= DT_PATH_ELLIPSIS;
	TCHAR szText[512] = { 0 };
	GetWindowText(hWnd, szText, 511);
	HFONT hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
	HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, AxisDimTextColor());
	DrawText(hdc, szText, -1, &rc, uFormat);
	if (hOldFont)
		SelectObject(hdc, hOldFont);
}

// Subclass procedure for static labels (custom drawing only while disabled).
static LRESULT CALLBACK AxisStaticSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_PAINT:
		if (!IsWindowEnabled(hWnd))
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			PaintAxisDisabledStatic(hWnd, hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
		break;
	case WM_ENABLE:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			InvalidateRect(hWnd, NULL, TRUE);
			return lRes;
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, AxisStaticSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Themed check box state images for tree views with TVS_CHECKBOXES (1 = unchecked, 2 = checked).
void ApplyAxisTreeCheckboxes(HWND hTree)
{
	if (hTree == NULL || !(GetWindowLong(hTree, GWL_STYLE) & TVS_CHECKBOXES))
		return;
	HDC hdcScreen = GetDC(hTree);
	int iSize = AxisScale(hdcScreen, 16);
	HIMAGELIST hList = ImageList_Create(iSize, iSize, ILC_COLOR32, 3, 0);
	for (int i = 0; i < 3; i++)
	{
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, iSize, iSize);
		HGDIOBJ hOld = SelectObject(hdcMem, hBmp);
		RECT rcAll = { 0, 0, iSize, iSize };
		HBRUSH hBk = CreateSolidBrush(DarkFieldBkColor());
		FillRect(hdcMem, &rcAll, hBk);
		DeleteObject(hBk);
		if (i > 0)
		{
			RECT rcBox = rcAll;
			InflateRect(&rcBox, -AxisScale(hdcScreen, 2), -AxisScale(hdcScreen, 2));
			DrawAxisCheckMark(hdcMem, rcBox, i == 2 ? 1 : 0, true, false, DarkFieldBkColor());
		}
		SelectObject(hdcMem, hOld);
		DeleteDC(hdcMem);
		ImageList_Add(hList, hBmp, NULL);
		DeleteObject(hBmp);
	}
	ReleaseDC(hTree, hdcScreen);
	HIMAGELIST hOldList = TreeView_SetImageList(hTree, hList, TVSIL_STATE);
	if (hOldList)
		ImageList_Destroy(hOldList);
}

// Draws the border (WS_BORDER / WS_EX_CLIENTEDGE) of list and tree controls in theme colors.
static LRESULT CALLBACK AxisBorderSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_NCPAINT:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			HDC hdc = GetWindowDC(hWnd);
			if (hdc)
			{
				RECT rc;
				GetWindowRect(hWnd, &rc);
				OffsetRect(&rc, -rc.left, -rc.top);
				HBRUSH hBr = CreateSolidBrush(AxisClr(AXC_SEPARATOR));
				FrameRect(hdc, &rc, hBr);
				DeleteObject(hBr);
				if (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
				{
					// Two-pixel edge: inner line in the field color
					InflateRect(&rc, -1, -1);
					HBRUSH hIn = CreateSolidBrush(DarkFieldBkColor());
					FrameRect(hdc, &rc, hIn);
					DeleteObject(hIn);
				}
				ReleaseDC(hWnd, hdc);
			}
			return lRes;
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, AxisBorderSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}
// Owner-drawn list view column header.
static LRESULT CALLBACK AxisHeaderSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			HBRUSH hBk = CreateSolidBrush(AxisClr(AXC_HEADERBK));
			FillRect(hdc, &rcClient, hBk);
			DeleteObject(hBk);
			HFONT hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
			HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, DarkTextColor());
			HBRUSH hLine = CreateSolidBrush(AxisClr(AXC_SEPARATOR));
			int iCount = Header_GetItemCount(hWnd);
			for (int i = 0; i < iCount; i++)
			{
				RECT rc;
				Header_GetItemRect(hWnd, i, &rc);
				TCHAR szText[128] = { 0 };
				HDITEM hdi;
				hdi.mask = HDI_TEXT | HDI_FORMAT;
				hdi.pszText = szText;
				hdi.cchTextMax = 127;
				Header_GetItem(hWnd, i, &hdi);
				RECT rcText = rc;
				InflateRect(&rcText, -6, 0);
				UINT uFormat = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
				if ((hdi.fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
					uFormat |= DT_RIGHT;
				else if ((hdi.fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
					uFormat |= DT_CENTER;
				DrawText(hdc, szText, -1, &rcText, uFormat);
				RECT rcDiv = { rc.right - 1, rc.top + 3, rc.right, rc.bottom - 3 };
				FillRect(hdc, &rcDiv, hLine);
			}
			RECT rcBottom = { rcClient.left, rcClient.bottom - 1, rcClient.right, rcClient.bottom };
			FillRect(hdc, &rcBottom, hLine);
			DeleteObject(hLine);
			if (hOldFont)
				SelectObject(hdc, hOldFont);
			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, AxisHeaderSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Subclasses the header of a list view for owner drawing.
void ApplyAxisListHeader(HWND hList)
{
	HWND hHeader = (HWND) ::SendMessage(hList, LVM_GETHEADER, 0, 0);
	if (hHeader)
	{
		SetWindowSubclass(hHeader, AxisHeaderSubclassProc, 1, 0);
		InvalidateRect(hHeader, NULL, TRUE);
	}
}
void AxisTranslateWindow(HWND hWnd);

// Owner-drawn list box (double-buffered) with an accent bar on selected items.
static void PaintAxisListBox(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	int w = rc.right - rc.left, h = rc.bottom - rc.top;
	if (w <= 0 || h <= 0)
		return;
	HDC hMem = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, w, h);
	HGDIOBJ hOldBmp = SelectObject(hMem, hBmp);
	HBRUSH hBk = CreateSolidBrush(DarkFieldBkColor());
	FillRect(hMem, &rc, hBk);
	DeleteObject(hBk);
	HFONT hFont = (HFONT) SendMessage(hWnd, WM_GETFONT, 0, 0);
	HGDIOBJ hOldFont = hFont ? SelectObject(hMem, hFont) : NULL;
	SetBkMode(hMem, TRANSPARENT);
	LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	bool bMulti = (lStyle & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0;
	bool bEnabled = IsWindowEnabled(hWnd) != FALSE;
	int nCount = (int) SendMessage(hWnd, LB_GETCOUNT, 0, 0);
	int iCur = (int) SendMessage(hWnd, LB_GETCURSEL, 0, 0);
	for (int i = (int) SendMessage(hWnd, LB_GETTOPINDEX, 0, 0); i >= 0 && i < nCount; i++)
	{
		RECT ri;
		if (SendMessage(hWnd, LB_GETITEMRECT, i, (LPARAM) &ri) == LB_ERR || ri.top >= rc.bottom)
			break;
		bool bSel = bMulti ? (SendMessage(hWnd, LB_GETSEL, i, 0) > 0) : (i == iCur);
		if (bSel)
		{
			HBRUSH hSel = CreateSolidBrush(AxisClr(AXC_BUTTON_HOVER));
			FillRect(hMem, &ri, hSel);
			DeleteObject(hSel);
			RECT rBar = ri;
			rBar.right = rBar.left + 3;
			HBRUSH hBar = CreateSolidBrush(DarkAccentColor());
			FillRect(hMem, &rBar, hBar);
			DeleteObject(hBar);
		}
		SetTextColor(hMem, !bEnabled ? AxisClr(AXC_DIM) : (bSel ? AxisAccentTextColor() : DarkTextColor()));
		int nLen = (int) SendMessage(hWnd, LB_GETTEXTLEN, i, 0);
		if (nLen > 0 && nLen < 4096)
		{
			CString cs;
			SendMessage(hWnd, LB_GETTEXT, i, (LPARAM) cs.GetBuffer(nLen + 1));
			cs.ReleaseBuffer();
			RECT rt = ri;
			rt.left += 7;
			rt.right -= 3;
			DrawText(hMem, cs, -1, &rt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
		}
	}
	if (hOldFont)
		SelectObject(hMem, hOldFont);
	BitBlt(hdc, 0, 0, w, h, hMem, 0, 0, SRCCOPY);
	SelectObject(hMem, hOldBmp);
	DeleteObject(hBmp);
	DeleteDC(hMem);
}

// Subclass procedure for list boxes; repaints after messages that draw the selection natively.
static LRESULT CALLBACK AxisListBoxSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			PaintAxisListBox(hWnd, hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_PRINTCLIENT:
		PaintAxisListBox(hWnd, (HDC) wParam);
		return 0;
	case WM_MOUSEMOVE:
		if (!(wParam & MK_LBUTTON))
			break;
		// Drag selection: fall through and handle like a click.
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_KEYDOWN:
	case WM_CHAR:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ENABLE:
	case WM_VSCROLL:
	case WM_MOUSEWHEEL:
	case LB_SETCURSEL:
	case LB_SETSEL:
	case LB_SETTOPINDEX:
	case LB_SELECTSTRING:
	case LB_ADDSTRING:
	case LB_INSERTSTRING:
	case LB_DELETESTRING:
	case LB_RESETCONTENT:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			if (IsWindow(hWnd))
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
			return lRes;
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, AxisListBoxSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Applies the theme and translation to one child control.
static BOOL CALLBACK ApplyDarkModeChildProc(HWND hWnd, LPARAM /*lParam*/)
{
	SetWindowTheme(hWnd, AxisLightTheme() ? L"Explorer" : L"DarkMode_Explorer", NULL);
	AxisTranslateWindow(hWnd);

	// List, tree and rich edit controls do not use WM_CTLCOLOR*; set their colors directly.
	TCHAR szClass[32];
	if (GetClassName(hWnd, szClass, 32) > 0)
	{
		if (((GetWindowLong(hWnd, GWL_STYLE) & WS_BORDER) || (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_CLIENTEDGE)) && (_tcsicmp(szClass, _T("ListBox")) == 0
			|| _tcsicmp(szClass, _T("SysTreeView32")) == 0 || _tcsicmp(szClass, _T("SysListView32")) == 0
			|| _tcsnicmp(szClass, _T("RichEdit"), 8) == 0))
		{
			SetWindowSubclass(hWnd, AxisBorderSubclassProc, 3, 0);
			SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		}
		if (_tcsicmp(szClass, _T("SysListView32")) == 0)
		{
			::SendMessage(hWnd, LVM_SETBKCOLOR, 0, (LPARAM) DarkFieldBkColor());
			::SendMessage(hWnd, LVM_SETTEXTBKCOLOR, 0, (LPARAM) DarkFieldBkColor());
			::SendMessage(hWnd, LVM_SETTEXTCOLOR, 0, (LPARAM) DarkTextColor());
			ApplyAxisListHeader(hWnd);
		}
		else if (_tcsicmp(szClass, _T("SysTreeView32")) == 0)
		{
			::SendMessage(hWnd, TVM_SETBKCOLOR, 0, (LPARAM) DarkFieldBkColor());
			::SendMessage(hWnd, TVM_SETTEXTCOLOR, 0, (LPARAM) DarkTextColor());
			ApplyAxisTreeCheckboxes(hWnd);
		}
		else if (_tcsnicmp(szClass, _T("RichEdit"), 8) == 0 || _tcsicmp(szClass, _T("RICHEDIT50W")) == 0)
		{
			::SendMessage(hWnd, EM_SETBKGNDCOLOR, 0, (LPARAM) DarkFieldBkColor());
		}
		else if (_tcsicmp(szClass, _T("ListBox")) == 0)
		{
			if (!(GetWindowLong(hWnd, GWL_STYLE) & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)))
			{
				SetWindowSubclass(hWnd, AxisListBoxSubclassProc, 4, 0);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		else if (_tcsicmp(szClass, _T("Button")) == 0)
		{
			LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
			LONG_PTR lType = lStyle & 0x0F; // BS_TYPEMASK
			bool bPlainPush = (lType == BS_PUSHBUTTON || lType == BS_DEFPUSHBUTTON)
				&& !(lStyle & BS_ICON) && !(lStyle & BS_BITMAP);
			if (bPlainPush)
				SetWindowSubclass(hWnd, AxisButtonSubclassProc, 1, 0);
			else if (lType == BS_GROUPBOX)
				SetWindowSubclass(hWnd, AxisGroupBoxSubclassProc, 1, 0);
			else if (lType == BS_CHECKBOX || lType == BS_AUTOCHECKBOX || lType == BS_3STATE
				|| lType == BS_AUTO3STATE || lType == BS_RADIOBUTTON || lType == BS_AUTORADIOBUTTON)
			{
				// Themed check boxes ignore SetTextColor; remove the theme and draw them ourselves.
				SetWindowTheme(hWnd, L"", L"");
				SetWindowSubclass(hWnd, AxisCheckSubclassProc, 1, 0);
			}
		}
		else if (_tcsicmp(szClass, _T("Static")) == 0)
		{
			LONG_PTR lType = GetWindowLongPtr(hWnd, GWL_STYLE) & SS_TYPEMASK;
			if (lType == SS_LEFT || lType == SS_CENTER || lType == SS_RIGHT || lType == SS_LEFTNOWORDWRAP || lType == SS_SIMPLE)
				SetWindowSubclass(hWnd, AxisStaticSubclassProc, 1, 0);
		}
		else if (_tcsicmp(szClass, _T("SysTabControl32")) == 0)
		{
			SetWindowSubclass(hWnd, AxisTabSubclassProc, 1, 0);
			// Translate the tab captions.
			if (AxisTranslating())
			{
				int iCount = TabCtrl_GetItemCount(hWnd);
				for (int i = 0; i < iCount; i++)
				{
					TCHAR szTab[256] = { 0 };
					TCITEM tci = { 0 };
					tci.mask = TCIF_TEXT;
					tci.pszText = szTab;
					tci.cchTextMax = 255;
					if (!TabCtrl_GetItem(hWnd, i, &tci))
						continue;
					LPCTSTR pszTr = AxisTr(szTab);
					if (_tcscmp(pszTr, szTab) != 0)
					{
						tci.pszText = (LPTSTR) pszTr;
						TabCtrl_SetItem(hWnd, i, &tci);
					}
				}
			}
		}
		else if (_tcsicmp(szClass, _T("Edit")) == 0 || _tcsicmp(szClass, _T("ComboBox")) == 0)
		{
			// DarkMode_CFD is the dark theme for edits and combo boxes.
			SetWindowTheme(hWnd, AxisLightTheme() ? L"Explorer" : L"DarkMode_CFD", NULL);
		}
	}
	return TRUE;
}

// Applies the theme to a single control created at runtime.
void ApplyDarkModeToControl(HWND hWnd)
{
	if (hWnd)
		ApplyDarkModeChildProc(hWnd, 0);
}

// Applies the theme to a window and all of its children.
void ApplyDarkMode(HWND hWndRoot)
{
	if (hWndRoot == NULL)
		return;
	SetWindowTheme(hWndRoot, AxisLightTheme() ? L"Explorer" : L"DarkMode_Explorer", NULL);
	EnumChildWindows(hWndRoot, ApplyDarkModeChildProc, 0);
}

// Sets a dark or light title bar on a top-level window to match the theme.
void ApplyDarkTitleBar(HWND hWnd)
{
	if (hWnd == NULL)
		return;
	BOOL bDarkTitle = AxisLightTheme() ? FALSE : TRUE;
	DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkTitle, sizeof(bDarkTitle));
}

// Dark and light palettes, indexed by AxisColorId (stdafx.h).
static const COLORREF g_aAxisDark[AXC_COUNT] = {
	RGB(0x1b, 0x17, 0x20), RGB(0x2c, 0x26, 0x32), RGB(0xca, 0xbf, 0xd4), RGB(0xe0, 0xb5, 0x63),	// page, field, text, accent
	RGB(0xe0, 0xb5, 0x63), RGB(0xe6, 0xbc, 0x70), RGB(0xc9, 0x9a, 0x45),						// accent text, hover, pressed
	RGB(0x22, 0x1a, 0x0c), RGB(0x15, 0x11, 0x19), RGB(0x2a, 0x25, 0x30), RGB(0x3a, 0x33, 0x41),	// on accent, sidebar, divider, border
	RGB(0x44, 0x3e, 0x4a), RGB(0x24, 0x1f, 0x2b), RGB(0x2c, 0x26, 0x32), RGB(0x3a, 0x2c, 0x14),	// separator, button, hover, pressed
	RGB(0x6b, 0x62, 0x74), RGB(0x7c, 0x75, 0x86), RGB(0x75, 0x6a, 0x80), RGB(0x8a, 0x80, 0x94),	// disabled, dim, muted, muted 2
	RGB(0xf4, 0xee, 0xe0), RGB(0x1e, 0x19, 0x24), RGB(0x2a, 0x25, 0x30),						// heading, card, header background
	RGB(0x8a, 0x82, 0x94), RGB(0x55, 0x50, 0x5c),												// check box border on/off
	RGB(0x6f, 0xcf, 0x8e), RGB(0xe6, 0xa6, 0x72), RGB(0xf2, 0x8b, 0x82), RGB(0x8a, 0xb4, 0xf8), RGB(0x9f, 0xb4, 0xc9),	// ok, warning, error, notice, notice 2
	RGB(0x1c, 0x30, 0x22), RGB(0x3a, 0x5c, 0x44), RGB(0xbf, 0xe9, 0xcb), RGB(0x4a, 0x33, 0x1a), RGB(0x1f, 0x33, 0x44),	// ok background/border/text, warning/notice background
	RGB(0xf0, 0x7a, 0x8c),																		// heart (donate)
	RGB(0xe6, 0xc0, 0x6a), RGB(0xe4, 0xde, 0xea), RGB(0x8a, 0xb4, 0xf8), RGB(0xe0, 0x8a, 0xe0), RGB(0x7c, 0xc4, 0x86),	// script syntax: header, text, trigger, variable, comment
};
static const COLORREF g_aAxisLight[AXC_COUNT] = {
	RGB(0xf3, 0xf1, 0xf5), RGB(0xff, 0xff, 0xff), RGB(0x2b, 0x25, 0x31), RGB(0xd9, 0xa4, 0x41),
	RGB(0x8f, 0x62, 0x12), RGB(0xe2, 0xb3, 0x5a), RGB(0xc1, 0x8f, 0x2f),
	RGB(0x24, 0x1a, 0x08), RGB(0xe9, 0xe5, 0xed), RGB(0xd9, 0xd3, 0xde), RGB(0xc6, 0xbf, 0xcd),
	RGB(0xd3, 0xcd, 0xd9), RGB(0xfb, 0xfa, 0xfc), RGB(0xef, 0xeb, 0xf3), RGB(0xf3, 0xe4, 0xc4),
	RGB(0xa3, 0x9b, 0xab), RGB(0x8d, 0x85, 0x97), RGB(0x7a, 0x70, 0x84), RGB(0x7a, 0x70, 0x84),
	RGB(0x1d, 0x18, 0x22), RGB(0xff, 0xff, 0xff), RGB(0xeb, 0xe7, 0xef),
	RGB(0x8a, 0x82, 0x94), RGB(0xc4, 0xbe, 0xc9),
	RGB(0x2e, 0x8b, 0x57), RGB(0xb5, 0x65, 0x1d), RGB(0xc0, 0x39, 0x2b), RGB(0x2f, 0x6f, 0xb0), RGB(0x4f, 0x6f, 0x8f),
	RGB(0xe3, 0xf4, 0xe8), RGB(0x9c, 0xcf, 0xad), RGB(0x1f, 0x6b, 0x3c), RGB(0xfb, 0xe9, 0xd4), RGB(0xe1, 0xec, 0xf7),
	RGB(0xd9, 0x48, 0x62),
	RGB(0x8f, 0x62, 0x12), RGB(0x2b, 0x25, 0x31), RGB(0x2f, 0x6f, 0xb0), RGB(0x9c, 0x3a, 0x9c), RGB(0x3a, 0x7d, 0x44),
};
static bool g_bAxisLight = false;

// Reads the theme from the registry ("Theme" = "light" or "dark", default dark).
// Runs before LoadIni, so it checks the user key first, then the machine key.
void AxisLoadTheme()
{
	CString csTheme = Main->GetRegistryString("Theme", "", hRegLocation ? hRegLocation : HKEY_CURRENT_USER);
	if (csTheme.IsEmpty() && !hRegLocation)
		csTheme = Main->GetRegistryString("Theme", "", HKEY_LOCAL_MACHINE);
	g_bAxisLight = (csTheme == "light");
}

bool AxisLightTheme() { return g_bAxisLight; }

COLORREF AxisClr(int iColor)
{
	if (iColor < 0 || iColor >= AXC_COUNT)
		return RGB(0xff, 0x00, 0xff);
	return g_bAxisLight ? g_aAxisLight[iColor] : g_aAxisDark[iColor];
}

COLORREF AxisAccentTextColor() { return AxisClr(AXC_ACCENT_TEXT); }

// Returns the per-user server profile folder (no admin rights needed).
CString AxisUserProfileDir(bool bCreate)
{
	TCHAR szPath[MAX_PATH];
	if (FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)))
		return Main->m_csRootDirectory + _T("Profile");
	CString csDir;
	csDir.Format(_T("%s\\AxisX\\Profile"), szPath);
	if (bCreate)
		SHCreateDirectoryEx(NULL, csDir, NULL);
	return csDir;
}

// Maps default preview background colors to the theme; custom colors are kept.
void AxisMapPreviewBg(DWORD & dwColor)
{
	if (dwColor == RGB(0xff, 0xff, 0xff) || dwColor == RGB(0xf9, 0xf9, 0xfb) || dwColor == RGB(0x2c, 0x26, 0x32))
		dwColor = DarkFieldBkColor();
}
COLORREF DarkPageBkColor()  { return AxisClr(AXC_PAGE); }
COLORREF DarkFieldBkColor() { return AxisClr(AXC_FIELD); }
COLORREF DarkTextColor()    { return AxisClr(AXC_TEXT); }
COLORREF DarkAccentColor()  { return AxisClr(AXC_ACCENT); }

bool IsLocalProfile(CString csProfile)
{
	if ((csProfile == "<Axis Profile>")||(csProfile == "<None>"))
		return false;
	CString csKey;
	csKey.Format("%s\\%s",REGKEY_PROFILE, csProfile);
	CString csType = Main->GetRegistryString("Type", "Local", hRegLocation, csKey);
	if(csType == "Local")
		return true;
	return false;
}


/////////////////////////////////////////////////////////////////////////////
// CFolderDialog

// Function name	: CFolderDialog::CFolderDialog
// Description	    : Constructor
// Return type		:
// Argument         : CString* pPath ; represent string where selected folder wil be saved
CFolderDialog::CFolderDialog(CString* pPath, CString csTitle) : CFolderPickerDialog(NULL, 0, NULL)
{
	m_pPath = pPath;
	m_Title = csTitle;
}

INT_PTR CFolderDialog::DoModal()
{
	m_ofn.lpstrTitle = m_Title;
	INT_PTR nResult = CFolderPickerDialog::DoModal();
	if (nResult == IDOK && m_pPath != NULL)
		*m_pPath = GetPathName();
	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
// Popup dialog theming: a CBT hook subclasses every new top-level dialog except the main window.

static HHOOK g_hAxisDialogHook = NULL;

// True for a plain message box (only static and button children).
static bool AxisIsPlainMessageBox(HWND hWnd)
{
	bool bHasButton = false;
	for (HWND hChild = GetWindow(hWnd, GW_CHILD); hChild != NULL; hChild = GetWindow(hChild, GW_HWNDNEXT))
	{
		TCHAR szClass[32] = { 0 };
		GetClassName(hChild, szClass, 32);
		if (_tcsicmp(szClass, _T("Button")) == 0)
			bHasButton = true;
		else if (_tcsicmp(szClass, _T("Static")) != 0)
			return false;
	}
	return bHasButton;
}

// Subclass procedure for popup dialogs and message boxes: theme, title bar, translation and colors.
static LRESULT CALLBACK AxisPopupSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
	static CBrush s_pageBrush(DarkPageBkColor());
	static CBrush s_fieldBrush(DarkFieldBkColor());
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			CWnd * pMain = AfxGetMainWnd();
			if (pMain != NULL && pMain->GetSafeHwnd() == hWnd)
			{
				RemoveWindowSubclass(hWnd, AxisPopupSubclassProc, uIdSubclass);
				return lRes;
			}
			if (CWnd::FromHandlePermanent(hWnd) == NULL)
			{
				// Non-MFC windows: theme plain message boxes only, and only in the dark theme.
				if (!AxisLightTheme() && AxisIsPlainMessageBox(hWnd))
				{
					SetProp(hWnd, _T("AxisMsgBox"), (HANDLE) 1);
					ApplyDarkMode(hWnd);
					ApplyDarkTitleBar(hWnd);
					InvalidateRect(hWnd, NULL, TRUE);
					return lRes;
				}
				RemoveWindowSubclass(hWnd, AxisPopupSubclassProc, uIdSubclass);
				return lRes;
			}
			ApplyDarkMode(hWnd);
			if (!(GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
				ApplyDarkTitleBar(hWnd);
			AxisTranslateWindow(hWnd);
			return lRes;
		}
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		::SetBkMode((HDC) wParam, TRANSPARENT);
		::SetTextColor((HDC) wParam, DarkTextColor());
		return (LRESULT) s_pageBrush.GetSafeHandle();
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
		::SetTextColor((HDC) wParam, DarkTextColor());
		::SetBkColor((HDC) wParam, DarkFieldBkColor());
		return (LRESULT) s_fieldBrush.GetSafeHandle();
	case WM_ERASEBKGND:
		if (GetProp(hWnd, _T("AxisMsgBox")))
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect((HDC) wParam, &rc, (HBRUSH) s_pageBrush.GetSafeHandle());
			return 1;
		}
		break;
	case WM_PAINT:
		if (GetProp(hWnd, _T("AxisMsgBox")))
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH) s_pageBrush.GetSafeHandle());
			EndPaint(hWnd, &ps);
			return 0;
		}
		break;
	case WM_NCDESTROY:
		RemoveProp(hWnd, _T("AxisMsgBox"));
		RemoveWindowSubclass(hWnd, AxisPopupSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// CBT hook: subclasses new dialog windows for theming.
static LRESULT CALLBACK AxisDialogCbtHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_CREATEWND)
	{
		CBT_CREATEWND * pCreate = (CBT_CREATEWND *) lParam;
		LPCREATESTRUCT pcs = pCreate->lpcs;
		bool bDialogClass = ((ULONG_PTR) pcs->lpszClass == 0x8002);	// WC_DIALOG atom
		if (!bDialogClass && !IS_INTRESOURCE(pcs->lpszClass))
			bDialogClass = (_tcsicmp(pcs->lpszClass, _T("#32770")) == 0);
		if (bDialogClass && !(pcs->style & WS_CHILD))
			SetWindowSubclass((HWND) wParam, AxisPopupSubclassProc, 2, 0);
		else if (bDialogClass && pcs->hwndParent != NULL)
		{
			// Pages of popup property sheets; main window pages theme themselves (CDockingPage).
			CWnd * pMain = AfxGetMainWnd();
			HWND hRoot = GetAncestor(pcs->hwndParent, GA_ROOT);
			if (pMain != NULL && pMain->GetSafeHwnd() != NULL && hRoot != pMain->GetSafeHwnd())
				SetWindowSubclass((HWND) wParam, AxisPopupSubclassProc, 2, 0);
		}
	}
	return CallNextHookEx(g_hAxisDialogHook, nCode, wParam, lParam);
}

void InstallDarkDialogHook()
{
	if (g_hAxisDialogHook == NULL)
		g_hAxisDialogHook = SetWindowsHookEx(WH_CBT, AxisDialogCbtHook, NULL, GetCurrentThreadId());
}

// Marks a button as the page's primary action (accent colored).
void AxisMarkPrimary(CWnd * pParent, UINT uID)
{
	CWnd * pButton = pParent ? pParent->GetDlgItem(uID) : NULL;
	if (pButton && pButton->GetSafeHwnd())
	{
		SetProp(pButton->GetSafeHwnd(), _T("AxisPrimary"), (HANDLE) 1);
		pButton->Invalidate();
	}
}

// Returns true if a UO client window is found.
bool AxisIsClientRunning()
{
	EnumWindows(EnumWindowsProc, 1);
	return hwndUOClient != NULL && IsWindow(hwndUOClient);
}

// Remembers a recently added item/NPC (newest first, at most 12).
void AxisRememberRecent(LPCTSTR pszLabel, LPCTSTR pszCommand)
{
	CStringArray aRaw;
	Main->GetRegistryMultiSz("RecentAdds", &aRaw, hRegLocation, REGKEY_AXIS);
	CString csLabel = pszLabel;
	csLabel.Remove('|');
	CString csEntry = csLabel + "|" + pszCommand;
	for (int i = (int) aRaw.GetSize() - 1; i >= 0; i--)
		if (aRaw[i].Mid(aRaw[i].Find('|') + 1).CompareNoCase(pszCommand) == 0)
			aRaw.RemoveAt(i);
	aRaw.InsertAt(0, csEntry);
	while (aRaw.GetSize() > 12)
		aRaw.RemoveAt(aRaw.GetSize() - 1);
	Main->PutRegistryMultiSz("RecentAdds", &aRaw, hRegLocation, REGKEY_AXIS);
}

// Opens the donation page.
void AxisOpenDonatePage()
{
	ShellExecute(NULL, _T("open"), AXIS_DONATE_URL, NULL, NULL, SW_SHOWNORMAL);
}

// UI translation: source strings (.rc dialogs and AXT("...")) are German. AxisLoadTranslations()
// reads Language\<lang>.ui.txt ("German<TAB>Translation" lines, Windows-1252); AxisTr() falls back to German.
static CMapStringToString g_mapAxisTr;

void AxisLoadTranslations(LPCTSTR pszLanguage)
{
	g_mapAxisTr.RemoveAll();
	if (pszLanguage == NULL || _tcsicmp(pszLanguage, _T("deu")) == 0)
		return;
	CString csFile;
	csFile.Format(_T("%sLanguage\\%s.ui.txt"), (LPCTSTR) Main->m_csRootDirectory, pszLanguage);
	CStdioFile f;
	if (!f.Open(csFile, CFile::modeRead | CFile::shareDenyNone | CFile::typeText))
		return;
	g_mapAxisTr.InitHashTable(2039);
	CString csLine;
	while (f.ReadString(csLine))
	{
		if (csLine.IsEmpty() || csLine[0] == '#')
			continue;
		int iTab = csLine.Find('\t');
		if (iTab <= 0)
			continue;
		CString csKey = csLine.Left(iTab);
		CString csValue = csLine.Mid(iTab + 1);
		// "\n" in the table stands for a line break.
		csKey.Replace(_T("\\n"), _T("\n"));
		csValue.Replace(_T("\\n"), _T("\n"));
		csValue.TrimRight();
		if (!csValue.IsEmpty())
			g_mapAxisTr.SetAt(csKey, csValue);
	}
	f.Close();
}

LPCTSTR AxisTr(LPCTSTR pszGerman)
{
	if (pszGerman == NULL || g_mapAxisTr.IsEmpty())
		return pszGerman;
	CMapStringToString::CPair * pPair = g_mapAxisTr.PLookup(pszGerman);
	return pPair ? (LPCTSTR) pPair->value : pszGerman;
}

bool AxisTranslating()
{
	return !g_mapAxisTr.IsEmpty();
}

// Translates a control's window text.
void AxisTranslateWindow(HWND hWnd)
{
	if (!AxisTranslating() || hWnd == NULL)
		return;
	TCHAR szText[1024] = { 0 };
	if (GetWindowText(hWnd, szText, 1023) <= 0)
		return;
	LPCTSTR pszTr = AxisTr(szText);
	if (pszTr != szText && _tcscmp(pszTr, szText) != 0)
		SetWindowText(hWnd, pszTr);
}

// Sets the translated cue banner of an edit control.
void AxisSetCue(HWND hEdit, LPCTSTR pszGerman)
{
	if (hEdit == NULL)
		return;
	LPCTSTR psz = AxisTr(pszGerman);
	WCHAR wsz[512] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, psz, -1, wsz, 511);
	::SendMessage(hEdit, EM_SETCUEBANNER, TRUE, (LPARAM) wsz);
}