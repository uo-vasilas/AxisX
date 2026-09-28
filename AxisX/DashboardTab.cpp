/*

 **********************************************************************
 *
 * Axis X - Dashboard/overview page.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 **********************************************************************

*/

#include "stdafx.h"
#include "AxisX.h"
#include "DashboardTab.h"
#include "AxisXDlg.h"
#include "RemoteConsole.h"
#include "Updater.h"

IMPLEMENT_DYNCREATE(CDashboardTab, CDockingPage)

// Background color shared by the dashboard cards and the controls inside them.
static COLORREF DashCardBkColor() { return AxisClr(AXC_CARD); }

// Paints the connection status static as a colored pill, reading the live
// state from Main->m_pRConsole.
static LRESULT CALLBACK ConnStatusSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
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

			bool bConnected = (Main->m_pRConsole != NULL) && Main->m_pRConsole->IsConnected();
			COLORREF crBg     = bConnected ? AxisClr(AXC_OK_BK) : AxisClr(AXC_BUTTON_HOVER);
			COLORREF crBorder = bConnected ? AxisClr(AXC_OK_BORDER) : AxisClr(AXC_BORDER);
			COLORREF crDot    = bConnected ? AxisClr(AXC_OK) : AxisClr(AXC_MUTED2);
			COLORREF crText   = bConnected ? AxisClr(AXC_OK_TEXT) : DarkTextColor();

			// Fill the corners first; WM_ERASEBKGND does not clear them.
			HBRUSH hPageBg = CreateSolidBrush(DarkPageBkColor());
			FillRect(hdc, &rc, hPageBg);
			DeleteObject(hPageBg);

			HBRUSH hBrush = CreateSolidBrush(crBg);
			HPEN hPen = CreatePen(PS_SOLID, 1, crBorder);
			HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);
			HGDIOBJ hOldPen = SelectObject(hdc, hPen);
			RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, rc.bottom - rc.top, rc.bottom - rc.top);
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			DeleteObject(hBrush);
			DeleteObject(hPen);

			int cy = (rc.top + rc.bottom) / 2;
			int dotX = rc.left + 15;
			int dotR = 3;
			HBRUSH hDotBrush = CreateSolidBrush(crDot);
			HGDIOBJ hOldBrush2 = SelectObject(hdc, hDotBrush);
			HGDIOBJ hOldPen2 = SelectObject(hdc, GetStockObject(NULL_PEN));
			Ellipse(hdc, dotX - dotR, cy - dotR, dotX + dotR, cy + dotR);
			SelectObject(hdc, hOldBrush2);
			SelectObject(hdc, hOldPen2);
			DeleteObject(hDotBrush);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, crText);
			HFONT hFont = (HFONT) SendMessage(hWnd, WM_GETFONT, 0, 0);
			HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
			RECT rcText = rc;
			rcText.left += 26;
			DrawText(hdc, bConnected ? AXT("Verbunden") : AXT("Nicht verbunden"), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			if (hOldFont)
				SelectObject(hdc, hOldFont);

			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_NCDESTROY:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			RemoveWindowSubclass(hWnd, ConnStatusSubclassProc, uIdSubclass);
			return lRes;
		}
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// Paints a dashboard groupbox as a rounded card with its label inside the
// top-left corner.
static LRESULT CALLBACK DashCardSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
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

			HBRUSH hBrush = CreateSolidBrush(DashCardBkColor());
			HPEN hPen = CreatePen(PS_SOLID, 1, AxisClr(AXC_BORDER));
			HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);
			HGDIOBJ hOldPen = SelectObject(hdc, hPen);
			RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 10, 10);
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			DeleteObject(hBrush);
			DeleteObject(hPen);

			TCHAR szText[128];
			GetWindowText(hWnd, szText, 128);
			SetBkMode(hdc, TRANSPARENT);

			// All-caps captions are small stat labels; mixed case ones are panel titles.
			CString csText = szText;
			CString csUpper = csText;
			csUpper.MakeUpper();
			bool bPanelTitle = (csText != csUpper);

			static CFont s_fontPanelTitle;
			if (s_fontPanelTitle.GetSafeHandle() == NULL)
				s_fontPanelTitle.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

			HFONT hFont;
			if (bPanelTitle)
			{
				SetTextColor(hdc, AxisClr(AXC_HEAD));
				hFont = (HFONT) s_fontPanelTitle.GetSafeHandle();
			}
			else
			{
				SetTextColor(hdc, AxisClr(AXC_MUTED));
				hFont = (HFONT) SendMessage(hWnd, WM_GETFONT, 0, 0);
			}
			HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
			RECT rcText = rc;
			rcText.left += 18;
			rcText.top += 14;
			DrawText(hdc, szText, -1, &rcText, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
			if (hOldFont)
				SelectObject(hdc, hOldFont);

			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_NCDESTROY:
		{
			LRESULT lRes = DefSubclassProc(hWnd, msg, wParam, lParam);
			RemoveWindowSubclass(hWnd, DashCardSubclassProc, uIdSubclass);
			return lRes;
		}
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

static BOOL CALLBACK ApplyDashCardStyleProc(HWND hWnd, LPARAM /*lParam*/)
{
	TCHAR szClass[32];
	if (GetClassName(hWnd, szClass, 32) > 0 && _tcsicmp(szClass, _T("Button")) == 0)
	{
		LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
		if ((lStyle & 0x0F) == BS_GROUPBOX) // BS_TYPEMASK
			SetWindowSubclass(hWnd, DashCardSubclassProc, 2, 0);
	}
	return TRUE;
}

// Quick-access rows: label + what happens on activation.
//  action 0 = SendToUO command (pszArg is the command, without the prefix)
//  action 1 = switch the sidebar to another page (pszArg unused, page index in nPage)
enum { QA_COMMAND = 0, QA_GOTOPAGE = 1 };
struct CDashQuickAction { LPCTSTR pszLabel; int action; LPCTSTR pszArg; int nPage; int icon; };
static const CDashQuickAction g_dashQuickActions[] = {
	{ _T("Item erstellen (Items-Seite)"),   QA_GOTOPAGE, _T(""),             5, 0 },  // Items page - plus
	{ _T("NPC beschw\xF6ren (Spawns-Seite)"), QA_GOTOPAGE, _T(""),             3, 1 },  // Spawn page - person
	{ _T("Teleport - Ziel w\xE4hlen"),        QA_COMMAND,  _T("tele"),         0, 2 },  // target
	{ _T("Unverwundbar an/aus"),            QA_COMMAND,  _T("invulnerable"), 0, 3 },  // shield
};

// Draws a small quick-action icon.
static void DrawQuickIcon(HDC hdc, int iconType, const RECT& rc, COLORREF color)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	HGDIOBJ hOldPen = SelectObject(hdc, hPen);
	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
	int cx = (rc.left + rc.right) / 2;
	int cy = (rc.top + rc.bottom) / 2;

	switch (iconType)
	{
	case 0: // plus - spawn item
		MoveToEx(hdc, cx, rc.top, NULL);
		LineTo(hdc, cx, rc.bottom);
		MoveToEx(hdc, rc.left, cy, NULL);
		LineTo(hdc, rc.right, cy);
		break;
	case 1: // person - spawn NPC
		Ellipse(hdc, cx - 3, rc.top, cx + 3, rc.top + 6);
		MoveToEx(hdc, cx - 5, rc.bottom, NULL);
		LineTo(hdc, cx - 3, rc.top + 8);
		LineTo(hdc, cx + 3, rc.top + 8);
		LineTo(hdc, cx + 5, rc.bottom);
		LineTo(hdc, cx - 5, rc.bottom);
		break;
	case 2: // target - teleport
		Ellipse(hdc, rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom - 1);
		MoveToEx(hdc, cx, rc.top + 1, NULL);
		LineTo(hdc, cx, rc.bottom - 1);
		MoveToEx(hdc, rc.left + 1, cy, NULL);
		LineTo(hdc, rc.right - 1, cy);
		break;
	case 4: // arrow - custom quick action
		MoveToEx(hdc, rc.left + 3, rc.top + 2, NULL);
		LineTo(hdc, rc.right - 3, cy);
		LineTo(hdc, rc.left + 3, rc.bottom - 2);
		break;
	case 3: // shield - toggle invulnerability
		{
			POINT pts[5] = {
				{ cx, rc.top }, { rc.right, rc.top + 3 }, { rc.right, cy + 2 },
				{ cx, rc.bottom }, { rc.left, cy + 2 },
			};
			Polyline(hdc, pts, 5);
			MoveToEx(hdc, rc.left, cy + 2, NULL);
			LineTo(hdc, rc.left, rc.top + 3);
			LineTo(hdc, cx, rc.top);
		}
		break;
	}

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

// Strips the configurable staff login prefix ("Staff Prefix" registry value)
// and capitalizes the name for the greeting.
static CString GmDisplayName(const CString& csAccount)
{
	static CString s_csPrefix = Main->GetRegistryString(_T("Staff Prefix"), _T("+staff_"));
	CString csName = csAccount;
	if (!s_csPrefix.IsEmpty() && csName.Left(s_csPrefix.GetLength()).CompareNoCase(s_csPrefix) == 0)
		csName = csName.Mid(s_csPrefix.GetLength());
	if (!csName.IsEmpty())
		csName.SetAt(0, (TCHAR) _totupper(csName[0]));
	return csName;
}

CDashboardTab::CDashboardTab() : CDockingPage(CDashboardTab::IDD, CMsg("IDS_DASHBOARD"))
{
	m_bRemoteConsoleCreated = false;
	m_bClientFound = false;
}

CDashboardTab::~CDashboardTab()
{
}

void CDashboardTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DASH_LOG, m_ceLog);
	DDX_Control(pDX, IDC_DASH_QUICKLIST, m_ctlQuick);
	DDX_Control(pDX, IDC_DASH_CONNSTATUS, m_ceConnStatus);
	DDX_Control(pDX, IDC_DASH_RECENT, m_ctlRecent);
	DDX_Control(pDX, IDC_DASH_QALABEL, m_ceQaLabel);
	DDX_Control(pDX, IDC_DASH_QACMD, m_ceQaCmd);
}

BEGIN_MESSAGE_MAP(CDashboardTab, CDockingPage)
	ON_BN_CLICKED(IDC_DASH_STARTCLIENT, OnStartClient)
	ON_BN_CLICKED(IDC_DASH_CONNECT, OnToggleConnect)
	ON_NOTIFY(NM_DBLCLK, IDC_DASH_QUICKLIST, OnQuickActivate)
	ON_NOTIFY(LVN_KEYDOWN, IDC_DASH_QUICKLIST, OnQuickActivate)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DASH_QUICKLIST, OnQuickCustomDraw)
	ON_NOTIFY(NM_DBLCLK, IDC_DASH_RECENT, OnRecentActivate)
	ON_NOTIFY(LVN_KEYDOWN, IDC_DASH_RECENT, OnRecentActivate)
	ON_BN_CLICKED(IDC_DASH_QAADD, OnQuickAdd)
	ON_BN_CLICKED(IDC_DASH_QADEL, OnQuickDel)
	ON_BN_CLICKED(IDC_DASH_DONATE, OnDonate)
	ON_BN_CLICKED(IDC_DASH_UPDATE, OnUpdate)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// Card background for statics inside cards; accent color for the stat values.
HBRUSH CDashboardTab::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDockingPage::OnCtlColor(pDC, pWnd, nCtlColor);
	if (nCtlColor == CTLCOLOR_STATIC && pWnd != NULL)
	{
		int nID = pWnd->GetDlgCtrlID();
		static CBrush s_cardBrush(DashCardBkColor());
		if (nID != IDC_DASH_GREETING && nID != IDC_DASH_SUBTITLE && nID != IDC_DASH_CONNSTATUS && nID != IDC_DASH_UPDATEINFO)
		{
			pDC->SetBkColor(DashCardBkColor());
			hbr = (HBRUSH) s_cardBrush.GetSafeHandle();
		}
		if (nID == IDC_DASH_ITEMCOUNT || nID == IDC_DASH_NPCCOUNT || nID == IDC_DASH_CONNPORT || nID == IDC_DASH_PROFILE || nID == IDC_DASH_UPDATEINFO)
			pDC->SetTextColor(AxisAccentTextColor());
		else if (nID == IDC_DASH_CLIENT)
			pDC->SetTextColor(m_bClientFound ? AxisClr(AXC_OK) : AxisClr(AXC_MUTED2));
		else if (nID == IDC_DASH_GREETING)
			pDC->SetTextColor(AxisClr(AXC_HEAD));
	}
	return hbr;
}

BOOL CDashboardTab::OnInitDialog()
{
	CDockingPage::OnInitDialog();
	AxisMarkPrimary(this, IDC_DASH_STARTCLIENT);	// Primary action

	static CFont s_fontGreeting;
	if (s_fontGreeting.GetSafeHandle() == NULL)
		s_fontGreeting.CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, _T("Constantia"));
	if (GetDlgItem(IDC_DASH_GREETING))
		GetDlgItem(IDC_DASH_GREETING)->SetFont(&s_fontGreeting);

	if (GetDlgItem(IDC_DASH_CONNSTATUS))
		SetWindowSubclass(GetDlgItem(IDC_DASH_CONNSTATUS)->GetSafeHwnd(), ConnStatusSubclassProc, 1, 0);

	static CFont s_fontBig;
	if (s_fontBig.GetSafeHandle() == NULL)
		s_fontBig.CreateFont(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
	if (GetDlgItem(IDC_DASH_ITEMCOUNT)) GetDlgItem(IDC_DASH_ITEMCOUNT)->SetFont(&s_fontBig);
	if (GetDlgItem(IDC_DASH_NPCCOUNT))  GetDlgItem(IDC_DASH_NPCCOUNT)->SetFont(&s_fontBig);

	// Smaller font for values that can be long, like host:port.
	static CFont s_fontConn;
	if (s_fontConn.GetSafeHandle() == NULL)
		s_fontConn.CreateFont(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
	if (GetDlgItem(IDC_DASH_CONNPORT))  GetDlgItem(IDC_DASH_CONNPORT)->SetFont(&s_fontConn);
	if (GetDlgItem(IDC_DASH_CLIENT))    GetDlgItem(IDC_DASH_CLIENT)->SetFont(&s_fontConn);
	if (GetDlgItem(IDC_DASH_PROFILE))   GetDlgItem(IDC_DASH_PROFILE)->SetFont(&s_fontConn);

	static CFont s_fontLog;
	if (s_fontLog.GetSafeHandle() == NULL)
		s_fontLog.CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, _T("Consolas"));
	m_ceLog.SetFont(&s_fontLog);

	EnumChildWindows(m_hWnd, ApplyDashCardStyleProc, 0);

	// Match the card fill instead of the default field color.
	m_ceLog.SetBackgroundColor(FALSE, DashCardBkColor());
	m_ctlQuick.SetBkColor(DashCardBkColor());
	m_ctlQuick.SetTextBkColor(DashCardBkColor());

	m_ctlQuick.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	CRect rcQuick;
	m_ctlQuick.GetClientRect(&rcQuick);
	m_ctlQuick.InsertColumn(0, "", LVCFMT_LEFT, rcQuick.Width());	// Full width, no horizontal scrollbar
	LoadQuickActions();
	FillQuickList();

	// Recently created items/NPCs
	m_ctlRecent.SetBkColor(DashCardBkColor());
	m_ctlRecent.SetTextBkColor(DashCardBkColor());
	m_ctlRecent.SetTextColor(DarkTextColor());
	m_ctlRecent.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	CRect rcRecent;
	m_ctlRecent.GetClientRect(&rcRecent);
	m_ctlRecent.InsertColumn(0, "", LVCFMT_LEFT, rcRecent.Width());
	FillRecent();
	AxisSetCue(m_ceQaLabel.GetSafeHwnd(), "Beschriftung");
	AxisSetCue(m_ceQaCmd.GetSafeHwnd(), "Befehl, z.B. .where");
	SetTimer(1, 3000, NULL);	// Poll client status

	RefreshStats();
	if (g_axisUpdate.bAvailable)
		ShowUpdate();	// the check finished before this page was created

	// Park initial focus on the list so no button shows a focus outline.
	m_ctlQuick.SetFocus();
	return FALSE;
}

BOOL CDashboardTab::OnSetActive()
{
	RefreshStats();
	if (m_ctlRecent.GetSafeHwnd())
		FillRecent();
	return CDockingPage::OnSetActive();
}

void CDashboardTab::RefreshStats()
{
	if (!m_hWnd)
		return;

	bool bConnected = (Main->m_pRConsole != NULL) && Main->m_pRConsole->IsConnected();

	SYSTEMTIME st;
	GetLocalTime(&st);
	CString csGreeting = (st.wHour < 11) ? AXT("Guten Morgen") : (st.wHour < 18) ? AXT("Guten Tag") : AXT("Guten Abend");
	// The name comes from the Remote Console login.
	if (bConnected && !Main->m_pRConsole->m_csAccount.IsEmpty())
		csGreeting.AppendFormat(_T(", %s"), GmDisplayName(Main->m_pRConsole->m_csAccount));
	SetDlgItemText(IDC_DASH_GREETING, csGreeting);

	CString csProfile = Main->m_csCurentProfile;
	if (csProfile == "")
		csProfile = CMsg("IDS_NONE");
	CString csSubtitle;
	csSubtitle.Format(AXT("Axis X - dein Werkzeug f\xFCr die Spielwelt"));
	SetDlgItemText(IDC_DASH_SUBTITLE, csSubtitle);
	SetDlgItemText(IDC_DASH_PROFILE, csProfile);
	m_bClientFound = AxisIsClientRunning();
	SetDlgItemText(IDC_DASH_CLIENT, m_bClientFound ? AXT("bereit") : AXT("nicht gefunden"));

	// The status pill reads its state when painting; just repaint it.
	if (GetDlgItem(IDC_DASH_CONNSTATUS))
		GetDlgItem(IDC_DASH_CONNSTATUS)->Invalidate();
	SetDlgItemText(IDC_DASH_CONNECT, bConnected ? AXT("Trennen") : AXT("Verbinden"));

	CString csCount;
	if (Main->m_pScripts != NULL)
	{
		csCount.Format(_T("%d"), (int) Main->m_pScripts->m_aItems.GetSize());
		SetDlgItemText(IDC_DASH_ITEMCOUNT, csCount);
		csCount.Format(_T("%d"), (int) Main->m_pScripts->m_aNPCs.GetSize());
		SetDlgItemText(IDC_DASH_NPCCOUNT, csCount);
	}
	else
	{
		SetDlgItemText(IDC_DASH_ITEMCOUNT, "0");
		SetDlgItemText(IDC_DASH_NPCCOUNT, "0");
	}
	CString csConn = AXT("nicht verbunden");
	if (bConnected)
		csConn.Format(_T("%s:%s"), Main->m_pRConsole->m_csAddress, Main->m_pRConsole->m_csPort);
	SetDlgItemText(IDC_DASH_CONNPORT, csConn);
}

void CDashboardTab::AddLogLine(CString csLine, COLORREF color, int iFormat)
{
	if (!m_ceLog.GetSafeHwnd())
		return;

	CHARFORMAT cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
	cf.dwEffects = 0;
	cf.yHeight = 160;
	cf.crTextColor = color;
	strcpy_s(cf.szFaceName, _T("Consolas"));

	long nInsertionPoint = m_ceLog.GetWindowTextLength();
	m_ceLog.SetSel(nInsertionPoint, -1);
	m_ceLog.SetSelectionCharFormat(cf);
	m_ceLog.ReplaceSel(csLine + _T("   "));

	// Severity tag with a colored background.
	LPCTSTR pszTag = (iFormat == 1) ? _T(" WARN ") : (iFormat == 2) ? _T(" NOTICE ") : _T(" INFO ");
	COLORREF crTagBg = (iFormat == 1) ? AxisClr(AXC_WARN_BK) : (iFormat == 2) ? AxisClr(AXC_NOTICE_BK) : AxisClr(AXC_BUTTON_HOVER);
	COLORREF crTagFg = (iFormat == 1) ? AxisClr(AXC_WARN) : (iFormat == 2) ? AxisClr(AXC_NOTICE2) : DarkTextColor();

	long nTagStart = m_ceLog.GetWindowTextLength();
	m_ceLog.SetSel(nTagStart, nTagStart);
	m_ceLog.ReplaceSel(pszTag);
	long nTagEnd = m_ceLog.GetWindowTextLength();

	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_FACE | CFM_SIZE | CFM_BOLD;
	cf2.dwEffects = CFE_BOLD;
	cf2.yHeight = 150;
	cf2.crTextColor = crTagFg;
	cf2.crBackColor = crTagBg;
	strcpy_s(cf2.szFaceName, _T("Segoe UI"));
	m_ceLog.SetSel(nTagStart, nTagEnd);
	m_ceLog.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf2);

	m_ceLog.SetSel(m_ceLog.GetWindowTextLength(), -1);
	m_ceLog.SetSelectionCharFormat(cf); // back to plain formatting for the newline
	m_ceLog.ReplaceSel(_T("\n"));
	m_ceLog.LineScroll(m_ceLog.GetLineCount());
}

// Connects to or disconnects from the Remote Console; on connect the console
// window is opened as well.
void CDashboardTab::OnToggleConnect()
{
	if (Main->m_pRConsole != NULL && Main->m_pRConsole->IsConnected())
	{
		if (m_bRemoteConsoleCreated)
		{
			m_rcDlg.DestroyWindow();
			m_rcDlg.m_rConsole = NULL; // avoid a dangling pointer if ~CDashboardTab() ever tears m_rcDlg down later
			m_bRemoteConsoleCreated = false;
		}
		delete Main->m_pRConsole; // ~CRemoteConsole() stops the worker thread and clears Main->m_pRConsole
	}
	else
	{
		if (Main->m_pRConsole != NULL)
			delete Main->m_pRConsole; // leftover from a cancelled/failed login attempt
		new CRemoteConsole(GetSafeHwnd()); // registers itself as Main->m_pRConsole; blocks on the login dialog

		if (Main->m_pRConsole != NULL && Main->m_pRConsole->IsConnected())
		{
			// Create only once; Esc/X merely hides the window.
			if (!m_bRemoteConsoleCreated)
			{
				m_rcDlg.Create(IDD_REMOTECONSOLEDLG, this);
				m_bRemoteConsoleCreated = true;
			}
			else
			{
				// Point the dialog at the new console and route its messages
				// back to the console window (OnInitDialog does this otherwise).
				if (m_rcDlg.SyncConsole() != NULL)
					m_rcDlg.m_rConsole->m_parentHWnd = m_rcDlg.GetSafeHwnd();
			}
			m_rcDlg.ShowWindow(SW_SHOW);
			m_rcDlg.SetForegroundWindow();
		}
	}
	RefreshStats();
}

void CDashboardTab::OnStartClient()
{
	CAxisXDlg* pMain = (CAxisXDlg*) AfxGetMainWnd();
	if (pMain)
	{
		// By object, not by index - indices shift while a page is undocked.
		pMain->ShowPage((CPropertyPage *) Main->m_pcppLauncherTab);
	}
}

// Owner-draws quick-action rows with an icon and selection highlight.
void CDashboardTab::OnQuickCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*) pNMHDR;
	switch (pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		{
			// Fill the whole client area; item rects only cover the column width.
			RECT rcClient;
			m_ctlQuick.GetClientRect(&rcClient);
			HBRUSH hBg = CreateSolidBrush(DashCardBkColor());
			FillRect(pLVCD->nmcd.hdc, &rcClient, hBg);
			DeleteObject(hBg);
		}
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT:
		{
			int iItem = (int) pLVCD->nmcd.dwItemSpec;
			HDC hdc = pLVCD->nmcd.hdc;

			RECT rc;
			m_ctlQuick.GetItemRect(iItem, &rc, LVIR_BOUNDS);

			BOOL bSelected = (m_ctlQuick.GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
			COLORREF crBg = bSelected ? AxisClr(AXC_BUTTON_HOVER) : DashCardBkColor();
			COLORREF crFg = bSelected ? AxisAccentTextColor() : DarkTextColor();

			HBRUSH hBg = CreateSolidBrush(crBg);
			FillRect(hdc, &rc, hBg);
			DeleteObject(hBg);

			int cy = (rc.top + rc.bottom) / 2;
			int nActions = sizeof(g_dashQuickActions) / sizeof(g_dashQuickActions[0]);
			int iconType = (iItem >= 0 && iItem < nActions) ? g_dashQuickActions[iItem].icon : 4;
			RECT rcIcon = { rc.left + 4, cy - 7, rc.left + 18, cy + 7 };
			DrawQuickIcon(hdc, iconType, rcIcon, crFg);

			TCHAR szText[256];
			m_ctlQuick.GetItemText(iItem, 0, szText, 256);
			RECT rcText = rc;
			rcText.left += 28;
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, crFg);
			HFONT hFont = (HFONT) m_ctlQuick.SendMessage(WM_GETFONT, 0, 0);
			HGDIOBJ hOldFont = hFont ? SelectObject(hdc, hFont) : NULL;
			DrawText(hdc, szText, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
			if (hOldFont)
				SelectObject(hdc, hOldFont);

			*pResult = CDRF_SKIPDEFAULT;
			return;
		}
	}
	*pResult = CDRF_DODEFAULT;
}

void CDashboardTab::OnQuickActivate(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	NMLVKEYDOWN* pKey = (NMLVKEYDOWN*) pNMHDR;
	if (pNMHDR->code == LVN_KEYDOWN && pKey->wVKey != VK_RETURN)
		return;

	int iSel = m_ctlQuick.GetNextItem(-1, LVNI_SELECTED);
	if (iSel < 0)
		return;

	int nActions = sizeof(g_dashQuickActions) / sizeof(g_dashQuickActions[0]);
	if (iSel >= nActions)
	{
		// Custom quick action: send the command as entered.
		int iCustom = iSel - nActions;
		if (iCustom < m_aQaCmd.GetSize())
			SendToUO(m_aQaCmd[iCustom]);
		return;
	}

	const CDashQuickAction& action = g_dashQuickActions[iSel];
	if (action.action == QA_COMMAND)
	{
		CString csCmd;
		csCmd.Format(_T("%s%s"), Main->m_csCommandPrefix, action.pszArg);
		if (!SendToUO(csCmd))
			Main->m_log.Add(1, AXT("Kein UO-Client gefunden - '%s' wurde nicht gesendet"), (LPCTSTR) csCmd);
	}
	else
	{
		CAxisXDlg* pMain = (CAxisXDlg*) AfxGetMainWnd();
		if (pMain)
		{
			// By object, not by index - indices shift while a page is undocked.
			CPropertyPage * pPage = (action.nPage == 5)
				? (CPropertyPage *) Main->m_pcppItemTab
				: (CPropertyPage *) Main->m_pcppSpawnTab;
			pMain->ShowPage(pPage);
		}
	}
}

// ---- Custom quick actions and recently created list ----

void CDashboardTab::LoadQuickActions()
{
	m_aQaLabel.RemoveAll();
	m_aQaCmd.RemoveAll();
	CStringArray aRaw;
	Main->GetRegistryMultiSz("DashQuickActions", &aRaw, hRegLocation, REGKEY_AXIS);
	for (int i = 0; i < aRaw.GetSize(); i++)
	{
		int iSep = aRaw[i].Find('|');
		if (iSep <= 0)
			continue;
		m_aQaLabel.Add(aRaw[i].Left(iSep));
		m_aQaCmd.Add(aRaw[i].Mid(iSep + 1));
	}
}

void CDashboardTab::SaveQuickActions()
{
	CStringArray aRaw;
	for (int i = 0; i < m_aQaLabel.GetSize(); i++)
		aRaw.Add(m_aQaLabel[i] + "|" + m_aQaCmd[i]);
	Main->PutRegistryMultiSz("DashQuickActions", &aRaw, hRegLocation, REGKEY_AXIS);
}

void CDashboardTab::FillQuickList()
{
	m_ctlQuick.DeleteAllItems();
	int nActions = sizeof(g_dashQuickActions) / sizeof(g_dashQuickActions[0]);
	for (int i = 0; i < nActions; i++)
		m_ctlQuick.InsertItem(i, AxisTr(g_dashQuickActions[i].pszLabel));
	for (int i = 0; i < m_aQaLabel.GetSize(); i++)
		m_ctlQuick.InsertItem(nActions + i, m_aQaLabel[i]);
}

void CDashboardTab::OnQuickAdd()
{
	CString csLabel, csCmd;
	m_ceQaLabel.GetWindowText(csLabel);
	m_ceQaCmd.GetWindowText(csCmd);
	csLabel.Trim();
	csCmd.Trim();
	csLabel.Remove('|');
	if (csLabel.IsEmpty() || csCmd.IsEmpty())
	{
		AxisSetStatus(AXT("F\xFCr eine Schnellaktion Beschriftung und Befehl eintragen."), 2);
		return;
	}
	m_aQaLabel.Add(csLabel);
	m_aQaCmd.Add(csCmd);
	SaveQuickActions();
	FillQuickList();
	m_ceQaLabel.SetWindowText(_T(""));
	m_ceQaCmd.SetWindowText(_T(""));
	AxisSetStatus(AXT("Schnellaktion angelegt: ") + csLabel, 1);
}

void CDashboardTab::OnQuickDel()
{
	int nActions = sizeof(g_dashQuickActions) / sizeof(g_dashQuickActions[0]);
	int iSel = m_ctlQuick.GetNextItem(-1, LVNI_SELECTED);
	if (iSel < nActions || iSel - nActions >= m_aQaLabel.GetSize())
	{
		AxisSetStatus(AXT("Erst eine eigene Schnellaktion in der Liste w\xE4hlen (die vier festen bleiben)."), 2);
		return;
	}
	m_aQaLabel.RemoveAt(iSel - nActions);
	m_aQaCmd.RemoveAt(iSel - nActions);
	SaveQuickActions();
	FillQuickList();
}

void CDashboardTab::FillRecent()
{
	m_ctlRecent.DeleteAllItems();
	CStringArray aRaw;
	Main->GetRegistryMultiSz("RecentAdds", &aRaw, hRegLocation, REGKEY_AXIS);
	for (int i = 0; i < aRaw.GetSize(); i++)
	{
		int iSep = aRaw[i].Find('|');
		if (iSep <= 0)
			continue;
		int iRow = m_ctlRecent.InsertItem(m_ctlRecent.GetItemCount(), aRaw[i].Left(iSep));
		m_ctlRecent.SetItemData(iRow, (DWORD_PTR) i);
	}
	if (m_ctlRecent.GetItemCount() == 0)
		m_ctlRecent.InsertItem(0, AXT("(noch nichts erstellt)"));
}

void CDashboardTab::OnRecentActivate(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	NMLVKEYDOWN* pKey = (NMLVKEYDOWN*) pNMHDR;
	if (pNMHDR->code == LVN_KEYDOWN && pKey->wVKey != VK_RETURN)
		return;
	int iSel = m_ctlRecent.GetNextItem(-1, LVNI_SELECTED);
	if (iSel < 0)
		return;
	CStringArray aRaw;
	Main->GetRegistryMultiSz("RecentAdds", &aRaw, hRegLocation, REGKEY_AXIS);
	int iIndex = (int) m_ctlRecent.GetItemData(iSel);
	if (iIndex < 0 || iIndex >= aRaw.GetSize())
		return;
	int iSep = aRaw[iIndex].Find('|');
	if (iSep > 0)
		SendToUO(aRaw[iIndex].Mid(iSep + 1));
}

void CDashboardTab::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		if (IsWindowVisible())
		{
			bool bFound = AxisIsClientRunning();
			if (bFound != m_bClientFound)
				RefreshStats();
		}
		return;
	}
	CDockingPage::OnTimer(nIDEvent);
}

void CDashboardTab::OnDonate()
{
	AxisOpenDonatePage();
}

void CDashboardTab::ShowUpdate()
{
	if (!m_hWnd || g_axisUpdate.csVersion.IsEmpty())
		return;
	CString csInfo;
	csInfo.Format(AXT("Neue Version %s verf\xFCgbar (installiert: %s)"), (LPCTSTR) g_axisUpdate.csVersion, (LPCTSTR) AxisCurrentVersion());
	SetDlgItemText(IDC_DASH_UPDATEINFO, csInfo);
	SetDlgItemText(IDC_DASH_UPDATE, AXT("Jetzt aktualisieren"));
	GetDlgItem(IDC_DASH_UPDATEINFO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_DASH_UPDATE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_DASH_UPDATE)->EnableWindow(TRUE);
}

// Downloads and starts the installer; settings and profiles are kept.
void CDashboardTab::OnUpdate()
{
	if (g_axisUpdate.csSetupUrl.IsEmpty())
	{
		ShellExecute(NULL, _T("open"), g_axisUpdate.csPageUrl.IsEmpty() ? AXIS_RELEASES_PAGE : (LPCTSTR) g_axisUpdate.csPageUrl, NULL, NULL, SW_SHOWNORMAL);
		return;
	}
	CString csAsk;
	csAsk.Format(AXT("Axis X %s herunterladen und installieren?\n\nAxis wird dazu beendet. Einstellungen und Profile bleiben erhalten."), (LPCTSTR) g_axisUpdate.csVersion);
	if (AfxMessageBox(csAsk, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	GetDlgItem(IDC_DASH_UPDATE)->EnableWindow(FALSE);
	SetDlgItemText(IDC_DASH_UPDATE, AXT("Wird geladen ..."));
	AxisStartUpdateDownload(AfxGetMainWnd()->GetSafeHwnd());
}
