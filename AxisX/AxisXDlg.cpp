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

// AxisXDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "AxisXDlg.h"
#include "stdio.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"
#include "DashboardTab.h"
#include "Updater.h"
#include <uxtheme.h>
#pragma comment(lib, "UxTheme.lib")

// Sidebar navigation control id and layout constants (see BuildSidebar()).
#define IDC_AXIS_SIDEBAR 9001
static const int SIDEBAR_WIDTH = 232;
static const int SIDEBAR_MARGIN = 16;
static const int SIDEBAR_BRAND_HEIGHT = 56;
static const int SIDEBAR_FOOTER_HEIGHT = 44;
static const int STATUS_HEIGHT = 26;	// Status line below the page

// Sidebar colours; the background is a shade darker than the page.
static COLORREF SidebarBkColor() { return AxisClr(AXC_SIDEBAR); }
static COLORREF SidebarDividerColor() { return AxisClr(AXC_DIVIDER); }

// Footer icon buttons under the nav list. action is dispatched in
// OnLButtonDown(); visible=false hides the entry.
struct CAxisFooterEntry { int action; int icon; LPCTSTR tip; bool visible; };
static const CAxisFooterEntry g_axisFooterActions[] = {
	{ 0, 5,  _T("Einstellungen"), true },
	{ 1, 14, _T("Profile"), true },
	{ 2, 4,  _T("Dokumentation"), false }, // hidden until the docs are up to date
	{ 3, 7,  _T("\xDC" "ber Axis X"), true },
	{ 5, 17, _T("Axis X unterst\xFCtzen (Ko-fi)"), true },	// Ko-fi donation link
	{ 4, 6,  _T("Beenden"), true },
};

// Sidebar layout: groups the property-sheet pages by their AddPage() index
// (0=Dashboard, 1=General, 2=Travel, 3=Spawn, 4=PlayerTweak, 5=Items,
// 6=ItemTweak, 7=Account, 8=Misc, 9=Launcher, 10=Commands, 11=Reminder, 12=Log).
// icon is a DrawNavIcon() index; visible=false hides the entry.
struct CAxisNavEntry { int group; int page; int icon; bool visible; };
static const CAxisNavEntry g_axisNavLayout[] = {
	{ 0, 0, 0, true },                                                        // Overview: Dashboard
	{ 1, 5, 2, true }, { 1, 6, 16, true }, { 1, 3, 15, true }, { 1, 2, 9, true }, // World: Items, Item Tweak, Spawn, Travel
	{ 2, 4, 1, true }, { 2, 7, 10, true },                                    // Players: Player Tweak, Account
	{ 3, 1, 5, true }, { 3, 10, 3, true }, { 3, 8, 11, true }, { 3, 9, 12, true }, // Tools: General, Commands, Misc, Launcher
	{ 4, 12, 4, true }, { 4, 11, 13, false },                                 // System: Log, Reminder (hidden)
};
static LPCTSTR g_axisNavGroupNames[] = {
	_T("\xDC""BERSICHT"),
	_T("WELT"),
	_T("SPIELER"),
	_T("WERKZEUGE"),
	_T("SYSTEM"),
};

static int FindNavIcon(int pageIndex)
{
	int nEntries = sizeof(g_axisNavLayout) / sizeof(g_axisNavLayout[0]);
	for (int e = 0; e < nEntries; e++)
		if (g_axisNavLayout[e].page == pageIndex)
			return g_axisNavLayout[e].icon;
	return -1;
}

// Maps an original AddPage() index to its page object; sheet indices shift
// while pages are undocked.
static CPropertyPage * NavPageFromOriginalIndex(int idx)
{
	switch (idx)
	{
	case 0:  return Main->m_pcppDashboardTab;
	case 1:  return Main->m_pcppGeneralTab;
	case 2:  return Main->m_pcppTravelTab;
	case 3:  return Main->m_pcppSpawnTab;
	case 4:  return Main->m_pcppPlayerTweakTab;
	case 5:  return Main->m_pcppItemTab;
	case 6:  return Main->m_pcppItemTweakTab;
	case 7:  return Main->m_pcppAccountTab;
	case 8:  return Main->m_pcppMiscTab;
	case 9:  return Main->m_pcppLauncherTab;
	case 10: return Main->m_pcppCommandsTab;
	case 11: return Main->m_pcppReminderTab;
	case 12: return Main->m_pcppAxisLogTab;
	}
	return NULL;
}

// Returns the visible sidebar pages in order (icon + page), used by the mini bar.
int AxisGetNavPages(int* aIcon, CPropertyPage** aPage, int nMax)
{
	int n = 0;
	int nEntries = sizeof(g_axisNavLayout) / sizeof(g_axisNavLayout[0]);
	for (int e = 0; e < nEntries && n < nMax; e++)
	{
		if (!g_axisNavLayout[e].visible)
			continue;
		CPropertyPage* pPage = NavPageFromOriginalIndex(g_axisNavLayout[e].page);
		if (pPage == NULL)
			continue;
		aIcon[n] = g_axisNavLayout[e].icon;
		aPage[n] = pPage;
		n++;
	}
	return n;
}

// Sidebar row lParam: -1 = group header, otherwise current sheet index (low byte)
// and original page index (next byte).
#define NAV_LPARAM(currentIdx, originalIdx) ((LPARAM)(((originalIdx) << 8) | ((currentIdx) & 0xFF)))
#define NAV_CURRENT_INDEX(lp)  ((int)((lp) & 0xFF))
#define NAV_ORIGINAL_INDEX(lp) ((int)(((lp) >> 8) & 0xFF))

// Draws a small line-art icon; shared by the sidebar, footer and mini bar.
void DrawNavIcon(HDC hdc, int iconType, const RECT& rc, COLORREF color)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	HGDIOBJ hOldPen = SelectObject(hdc, hPen);
	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

	int cx = (rc.left + rc.right) / 2;
	int cy = (rc.top + rc.bottom) / 2;

	switch (iconType)
	{
	case 0: // grid - Dashboard
		Rectangle(hdc, rc.left, rc.top, cx - 1, cy - 1);
		Rectangle(hdc, cx + 1, rc.top, rc.right, cy - 1);
		Rectangle(hdc, rc.left, cy + 1, cx - 1, rc.bottom);
		Rectangle(hdc, cx + 1, cy + 1, rc.right, rc.bottom);
		break;
	case 1: // person - Player Tweak
		Ellipse(hdc, cx - 3, rc.top, cx + 3, rc.top + 6);
		MoveToEx(hdc, cx - 5, rc.bottom, NULL);
		LineTo(hdc, cx - 3, rc.top + 8);
		LineTo(hdc, cx + 3, rc.top + 8);
		LineTo(hdc, cx + 5, rc.bottom);
		LineTo(hdc, cx - 5, rc.bottom);
		break;
	case 2: // hexagon (boxed item) - Items
		{
			int qy = (rc.bottom - rc.top) / 4;
			POINT pts[6] = {
				{ cx, rc.top },
				{ rc.right, rc.top + qy },
				{ rc.right, rc.bottom - qy },
				{ cx, rc.bottom },
				{ rc.left, rc.bottom - qy },
				{ rc.left, rc.top + qy },
			};
			Polygon(hdc, pts, 6);
			MoveToEx(hdc, rc.left, rc.top + qy, NULL);
			LineTo(hdc, cx, rc.top + 2 * qy);
			LineTo(hdc, rc.right, rc.top + qy);
			MoveToEx(hdc, cx, rc.top + 2 * qy, NULL);
			LineTo(hdc, cx, rc.bottom);
		}
		break;
	case 3: // terminal - Commands
		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
		MoveToEx(hdc, rc.left + 3, rc.top + 4, NULL);
		LineTo(hdc, cx - 1, cy);
		LineTo(hdc, rc.left + 3, rc.bottom - 4);
		break;
	case 4: // document - Log, Documentation
		Rectangle(hdc, rc.left + 2, rc.top, rc.right - 2, rc.bottom);
		MoveToEx(hdc, rc.left + 4, cy - 3, NULL);
		LineTo(hdc, rc.right - 4, cy - 3);
		MoveToEx(hdc, rc.left + 4, cy + 2, NULL);
		LineTo(hdc, rc.right - 4, cy + 2);
		break;
	case 5: // gear - Settings
		Ellipse(hdc, rc.left + 3, rc.top + 3, rc.right - 3, rc.bottom - 3);
		Ellipse(hdc, cx - 2, cy - 2, cx + 2, cy + 2);
		MoveToEx(hdc, cx, rc.top, NULL);
		LineTo(hdc, cx, rc.top + 3);
		MoveToEx(hdc, cx, rc.bottom - 3, NULL);
		LineTo(hdc, cx, rc.bottom);
		MoveToEx(hdc, rc.left, cy, NULL);
		LineTo(hdc, rc.left + 3, cy);
		MoveToEx(hdc, rc.right - 3, cy, NULL);
		LineTo(hdc, rc.right, cy);
		break;
	case 6: // power - Exit
		Arc(hdc, rc.left, rc.top + 2, rc.right, rc.bottom, rc.left + 2, rc.top + 2, rc.right - 2, rc.top + 2);
		MoveToEx(hdc, cx, rc.top - 1, NULL);
		LineTo(hdc, cx, cy);
		break;
	case 7: // info - About
		Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom);
		MoveToEx(hdc, cx, cy - 1, NULL);
		LineTo(hdc, cx, rc.bottom - 3);
		MoveToEx(hdc, cx, rc.top + 3, NULL);
		LineTo(hdc, cx, rc.top + 4);
		break;
	case 9: // globe (circle + cross) - Travel
		Ellipse(hdc, rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom - 1);
		MoveToEx(hdc, cx, rc.top + 1, NULL);
		LineTo(hdc, cx, rc.bottom - 1);
		MoveToEx(hdc, rc.left + 1, cy, NULL);
		LineTo(hdc, rc.right - 1, cy);
		break;
	case 10: // lock - Account
		MoveToEx(hdc, cx - 4, rc.top + 8, NULL);
		LineTo(hdc, cx - 4, rc.top + 3);
		LineTo(hdc, cx - 2, rc.top + 1);
		LineTo(hdc, cx + 2, rc.top + 1);
		LineTo(hdc, cx + 4, rc.top + 3);
		LineTo(hdc, cx + 4, rc.top + 8);
		Rectangle(hdc, rc.left + 1, rc.top + 7, rc.right - 1, rc.bottom);
		MoveToEx(hdc, cx, rc.top + 10, NULL);
		LineTo(hdc, cx, rc.top + 13);
		break;
	case 11: // three dots - Misc
		{
			HGDIOBJ hOldBrushDots = SelectObject(hdc, GetStockObject(NULL_BRUSH));
			HBRUSH hDotBrush = CreateSolidBrush(color);
			SelectObject(hdc, hDotBrush);
			Ellipse(hdc, rc.left, cy - 1, rc.left + 3, cy + 2);
			Ellipse(hdc, cx - 1, cy - 1, cx + 2, cy + 2);
			Ellipse(hdc, rc.right - 3, cy - 1, rc.right, cy + 2);
			SelectObject(hdc, hOldBrushDots);
			DeleteObject(hDotBrush);
		}
		break;
	case 12: // play triangle - Launcher
		{
			POINT pts[3] = { { rc.left + 2, rc.top }, { rc.left + 2, rc.bottom }, { rc.right - 1, cy } };
			Polygon(hdc, pts, 3);
		}
		break;
	case 13: // bell - Reminder
		Arc(hdc, rc.left + 2, rc.top, rc.right - 2, cy + 2, rc.left + 2, cy + 2, rc.right - 2, cy + 2);
		MoveToEx(hdc, rc.left + 1, cy + 2, NULL);
		LineTo(hdc, rc.right - 1, cy + 2);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Ellipse(hdc, cx - 1, rc.bottom - 3, cx + 1, rc.bottom - 1);
		break;
	case 14: // swap - Profiles
		Arc(hdc, rc.left, rc.top, rc.right, cy + 4, rc.right - 1, rc.top + 2, rc.left + 1, rc.top + 2);
		MoveToEx(hdc, rc.left + 4, rc.top, NULL);
		LineTo(hdc, rc.left + 1, rc.top + 3);
		LineTo(hdc, rc.left + 5, rc.top + 4);
		Arc(hdc, rc.left, cy - 4, rc.right, rc.bottom, rc.left + 1, rc.bottom - 2, rc.right - 1, rc.bottom - 2);
		MoveToEx(hdc, rc.right - 4, rc.bottom, NULL);
		LineTo(hdc, rc.right - 1, rc.bottom - 3);
		LineTo(hdc, rc.right - 5, rc.bottom - 4);
		break;
	case 15: // crystal - Spawn
		{
			POINT pts[4] = { { cx, rc.top }, { rc.right, cy }, { cx, rc.bottom }, { rc.left, cy } };
			Polygon(hdc, pts, 4);
			MoveToEx(hdc, cx, rc.top, NULL);
			LineTo(hdc, cx, rc.bottom);
		}
		break;
	case 17: // heart - Ko-fi
		{
			int w = rc.right - rc.left;
			POINT pts[10] = {
				{ cx, rc.bottom - 1 },
				{ rc.left, cy - 1 },
				{ rc.left, rc.top + w / 4 },
				{ rc.left + w / 5, rc.top + 1 },
				{ cx - w / 8, rc.top + 1 },
				{ cx, rc.top + w / 4 },
				{ cx + w / 8, rc.top + 1 },
				{ rc.right - w / 5, rc.top + 1 },
				{ rc.right, rc.top + w / 4 },
				{ rc.right, cy - 1 },
			};
			Polygon(hdc, pts, 10);
		}
		break;
	case 18: // window with arrow - restore (mini bar)
		Rectangle(hdc, rc.left, rc.top + 4, rc.right - 4, rc.bottom);
		MoveToEx(hdc, cx - 1, cy + 1, NULL);
		LineTo(hdc, rc.right, rc.top - 1);
		MoveToEx(hdc, rc.right - 5, rc.top, NULL);
		LineTo(hdc, rc.right, rc.top);
		LineTo(hdc, rc.right, rc.top + 5);
		break;
	case 16: // open box - Item Tweak
		{
			int qy = (rc.bottom - rc.top) / 4;
			POINT pts[6] = {
				{ cx, rc.top + 3 },
				{ rc.right, rc.top + qy + 3 },
				{ rc.right, rc.bottom - qy },
				{ cx, rc.bottom },
				{ rc.left, rc.bottom - qy },
				{ rc.left, rc.top + qy + 3 },
			};
			Polygon(hdc, pts, 6);
			MoveToEx(hdc, cx, rc.top + 3, NULL);
			LineTo(hdc, rc.left + 1, rc.top - 2);
		}
		break;
	}

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAxisXDlg

IMPLEMENT_DYNAMIC(CAxisXDlg, CPropertySheet)

CAxisXDlg::CAxisXDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bSyncingNav = false;
	m_iFooterHover = -1;
	m_bLayoutSized = false;
	m_bFirstRunSettings = false;
	m_szNatural = CSize(0, 0);
	m_nFooterVisible = 0;
	m_crStatus = DarkTextColor();
}

CAxisXDlg::CAxisXDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInit = false;
	m_bSyncingNav = false;
	m_iFooterHover = -1;
	m_bLayoutSized = false;
	m_bFirstRunSettings = false;
	m_szNatural = CSize(0, 0);
	m_nFooterVisible = 0;
	m_crStatus = DarkTextColor();
}

CAxisXDlg::~CAxisXDlg()
{
	m_nid.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}


BEGIN_MESSAGE_MAP(CAxisXDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CAxisXDlg)
	ON_WM_CREATE()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_AXIS_SIDEBAR, OnNavSelChanging)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_AXIS_SIDEBAR, OnNavSelChanged)
	ON_MESSAGE(WM_APP + 96, OnNavResync)
	ON_MESSAGE(WM_APP + 97, OnCheckLabels)
	ON_MESSAGE(WM_APP + 98, OnCheckPopupLabels)
	ON_MESSAGE(WM_APP + 99, OnPressPageButton)
	ON_MESSAGE(WM_AXIS_UPDATE_CHECKED, OnUpdateChecked)
	ON_MESSAGE(WM_AXIS_UPDATE_DOWNLOAD, OnUpdateDownload)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_AXIS_SIDEBAR, OnNavCustomDraw)
	ON_COMMAND(ID_SETTINGS_GENERAL, OnSettingsGeneral)
	ON_COMMAND(ID_SETTINGS_FILEPATHS, OnSettingsPaths)
	ON_COMMAND(ID_SETTINGS_ITEMTAB, OnSettingsItem)
	ON_COMMAND(ID_SETTINGS_TRAVELTAB, OnSettingsTravel)
	ON_COMMAND(ID_SETTINGS_SPAWNTAB, OnSettingsSpawn)
	ON_COMMAND(ID_SETTINGS_OVERRIDEPATHS, OnSettingsOverridePaths)
	ON_COMMAND(ID_PROFILES_OPTION, OnOpenProfileOption)
	ON_COMMAND(ID_PROFILES_UNLOAD, OnUnloadProfile)
	ON_COMMAND(ID_PROFILES_LOADDEFAULT, OnLoadDefProfile)
	ON_COMMAND(ID_PROFILES_LOADLASTPROFILE, OnLoadLastProfile)
	ON_COMMAND(ID_HELP_DOCUMENTATION, OnHelp)
	ON_COMMAND(ID_HELP_ABOUTAXISX, OnAboutDlg)
	ON_COMMAND(ID_EXIT_CLOSEAXISX, OnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAxisXDlg message handlers

BOOL CAxisXDlg::OnInitDialog() 
{

	BOOL bResult = CPropertySheet::OnInitDialog();

	// The menu is not attached; it stays loaded because UpdateProfileMenu() edits it.
	Main->pDefMenu.LoadMenu(IDR_MENU1);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		int iCount = pSysMenu->GetMenuItemCount();
		for (int i = 0; i < iCount; i++)
		{
			CString csItem;
			pSysMenu->GetMenuString(i, csItem, MF_BYPOSITION);
			if (csItem.Find("Ma&ximize") != -1)
				pSysMenu->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
			if (csItem.Find("&Size") != -1)
				pSysMenu->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}

	Main->UpdateProfileMenu();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	ApplyCustomBranding();			// Optional custom icon/logo
	
	int x, y;
	x = -1;
	y = -1;
	CString csX, csY;
	csX = Main->m_csPosition.SpanExcluding(",");
	if ( csX != "" )
		x = atoi(csX);
	if ( Main->m_csPosition.Find(",") != -1 )
		csY = Main->m_csPosition.Mid(Main->m_csPosition.Find(",") + 1 );
	if ( csY != "" )
		y = atoi(csY);
	int X, Y;
	X = GetSystemMetrics(SM_CXFULLSCREEN)-20;
	Y = GetSystemMetrics(SM_CYFULLSCREEN)-20;
	CRect rectDlg;
	GetWindowRect(rectDlg);
	if ( x >= 0 && y >= 0 && x <= X && y <= Y )
		this->SetWindowPos( NULL, x, y, rectDlg.Width(), rectDlg.Height()+23, SWP_NOZORDER );

	BuildSidebar();

	// First start: open the settings over the main window. Opened earlier, the popup theming
	// cannot tell settings pages from main-window pages yet and leaves them light.
	if (m_bFirstRunSettings)
		PostMessage(WM_COMMAND, ID_SETTINGS_GENERAL);

	// On every start; the result arrives as WM_AXIS_UPDATE_CHECKED.
	if (AxisUpdateCheckDue())
		AxisStartUpdateCheck(m_hWnd, false);

	m_bInit = true;
	return bResult;
}

// Relays mouse messages to the footer tooltip control.
BOOL CAxisXDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_tipFooter.GetSafeHwnd())
		m_tipFooter.RelayEvent(pMsg);
	return CPropertySheet::PreTranslateMessage(pMsg);
}

// Replaces the tab strip with a grouped sidebar list and footer.
void CAxisXDlg::BuildSidebar()
{
	ApplyDarkTitleBar(m_hWnd);
	// Keep OnEraseBkgnd from painting over the active page.
	ModifyStyle(0, WS_CLIPCHILDREN);

	CTabCtrl* pTab = GetTabControl();
	if (pTab)
		pTab->ShowWindow(SW_HIDE);

	m_fntNavGroup.CreateFont(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
	m_fntNavItem.CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

	CRect rcClient;
	GetClientRect(&rcClient);

	m_ctlNav.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS,
		CRect(0, SIDEBAR_BRAND_HEIGHT, SIDEBAR_WIDTH, rcClient.Height() - SIDEBAR_FOOTER_HEIGHT), this, IDC_AXIS_SIDEBAR);
	// Disable visual styles so the theme's selection highlight doesn't override custom draw.
	SetWindowTheme(m_ctlNav.GetSafeHwnd(), L"", L"");
	m_ctlNav.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_ctlNav.SetFont(&m_fntNavItem);
	m_ctlNav.InsertColumn(0, "", LVCFMT_LEFT, SIDEBAR_WIDTH - 4);
	m_ctlNav.SetBkColor(SidebarBkColor());
	m_ctlNav.SetTextBkColor(SidebarBkColor());
	m_ctlNav.SetTextColor(DarkTextColor());

	// The row height comes from the small image list's height; its bitmap is never drawn.
	m_iNavRowHeight = 34;
	CBitmap bmpRowHeight, bmpRowHeightMask;
	bmpRowHeight.CreateBitmap(1, 34, 1, 32, NULL);
	bmpRowHeightMask.CreateBitmap(1, 34, 1, 1, NULL);
	m_ilNavRowHeight.Create(1, 34, ILC_COLOR32 | ILC_MASK, 1, 0);
	m_ilNavRowHeight.Add(&bmpRowHeight, &bmpRowHeightMask);
	m_ctlNav.SetImageList(&m_ilNavRowHeight, LVSIL_SMALL);

	RebuildSidebarItems();

	// Collect the visible footer entries.
	m_nFooterVisible = 0;
	int nFooterTotal = sizeof(g_axisFooterActions) / sizeof(g_axisFooterActions[0]);
	for (int f = 0; f < nFooterTotal; f++)
	{
		if (!g_axisFooterActions[f].visible)
			continue;
		m_aFooterAction[m_nFooterVisible] = g_axisFooterActions[f].action;
		m_aFooterIcon[m_nFooterVisible] = g_axisFooterActions[f].icon;
		m_aFooterTip[m_nFooterVisible] = AxisTr(g_axisFooterActions[f].tip);
		m_nFooterVisible++;
	}

	m_tipFooter.Create(this, TTS_ALWAYSTIP);
	m_tipFooter.SetMaxTipWidth(200);
	m_tipFooter.Activate(TRUE);

	RepositionLayout();
}

// Sets the initial window size once, then lays out the sidebar and active page.
void CAxisXDlg::RepositionLayout()
{
	CPropertyPage* pActive = GetActivePage();
	if (!pActive || !pActive->GetSafeHwnd())
		return;

	// The natural size (page template beside the sidebar) is the start size and
	// the basis for the minimum size.
	if (!m_bLayoutSized)
	{
		// Use the page's template size; the sheet may already have resized the page.
		CRect rcPage;
		pActive->GetWindowRect(&rcPage);
		int pageW = rcPage.Width();
		int pageH = rcPage.Height();
		CDockingPage * pDock = DYNAMIC_DOWNCAST(CDockingPage, pActive);
		if (pDock && pDock->m_szBase.cx > 0)
		{
			pageW = max(pageW, (int) pDock->m_szBase.cx);
			pageH = max(pageH, (int) pDock->m_szBase.cy);
		}

		// A fixed minimum height keeps the sidebar list from being clipped.
		const int SIDEBAR_MIN_HEIGHT = 470;
		int navContentHeight = SIDEBAR_BRAND_HEIGHT + SIDEBAR_FOOTER_HEIGHT;
		if (m_ctlNav.GetSafeHwnd() && m_ctlNav.GetItemCount() > 0)
		{
			CRect rcRow;
			m_ctlNav.GetItemRect(0, &rcRow, LVIR_BOUNDS);
			navContentHeight += m_ctlNav.GetItemCount() * rcRow.Height();
		}
		int totalW = SIDEBAR_WIDTH + SIDEBAR_MARGIN + pageW + SIDEBAR_MARGIN;
		int totalH = max(max(SIDEBAR_MIN_HEIGHT, navContentHeight), pageH + (2 * SIDEBAR_MARGIN) + STATUS_HEIGHT);

		ModifyStyle(0, WS_THICKFRAME | WS_MAXIMIZEBOX, SWP_FRAMECHANGED);
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		CRect rcClientBefore;
		GetClientRect(&rcClientBefore);
		int extraW = rcWnd.Width() - rcClientBefore.Width();
		int extraH = rcWnd.Height() - rcClientBefore.Height();
		m_szNatural = CSize(totalW + extraW, totalH + extraH);
		m_bLayoutSized = true;

		// Restore the last used size, otherwise use the natural size.
		int iW = (int) Main->GetRegistryDword("Window Width", 0);
		int iH = (int) Main->GetRegistryDword("Window Height", 0);
		if (iW < m_szNatural.cx * 7 / 10 || iH < m_szNatural.cy * 7 / 10)
		{
			iW = m_szNatural.cx;
			iH = m_szNatural.cy;
		}
		SetWindowPos(NULL, 0, 0, iW, iH, SWP_NOZORDER | SWP_NOMOVE);
		if (Main->GetRegistryDword("Window Maximized", 0))
			PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
	}
	LayoutActivePage();
	UpdateFooterTooltips();
}

// Sizes the active page to fill the area right of the sidebar, above the status line.
void CAxisXDlg::LayoutActivePage()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	if (m_ctlNav.GetSafeHwnd())
	{
		m_ctlNav.SetWindowPos(NULL, 0, SIDEBAR_BRAND_HEIGHT, SIDEBAR_WIDTH, rcClient.Height() - SIDEBAR_BRAND_HEIGHT - SIDEBAR_FOOTER_HEIGHT, SWP_NOZORDER);
		FitSidebarRows();
	}
	CPropertyPage* pActive = GetActivePage();
	if (!pActive || !pActive->GetSafeHwnd())
		return;
	int x = SIDEBAR_WIDTH + SIDEBAR_MARGIN;
	int y = SIDEBAR_MARGIN;
	int w = rcClient.Width() - x - SIDEBAR_MARGIN;
	int h = rcClient.Height() - 2 * SIDEBAR_MARGIN - STATUS_HEIGHT;
	if (w > 50 && h > 50)
		pActive->SetWindowPos(NULL, x, y, w, h, SWP_NOZORDER);
}

// Shrinks sidebar rows (down to 24 px) to avoid scrollbars and fits the column to the list width.
void CAxisXDlg::FitSidebarRows()
{
	if (!m_ctlNav.GetSafeHwnd() || m_ctlNav.GetItemCount() == 0)
		return;
	CRect rcNav;
	m_ctlNav.GetWindowRect(&rcNav);
	int iRow = rcNav.Height() / m_ctlNav.GetItemCount();
	if (iRow > 34) iRow = 34;
	if (iRow < 24) iRow = 24;
	if (iRow != m_iNavRowHeight)
	{
		// Set the new image list before destroying the old one, which the list still holds.
		HIMAGELIST hOld = m_ilNavRowHeight.Detach();
		CBitmap bmpRow, bmpRowMask;
		bmpRow.CreateBitmap(1, iRow, 1, 32, NULL);
		bmpRowMask.CreateBitmap(1, iRow, 1, 1, NULL);
		m_ilNavRowHeight.Create(1, iRow, ILC_COLOR32 | ILC_MASK, 1, 0);
		m_ilNavRowHeight.Add(&bmpRow, &bmpRowMask);
		m_ctlNav.SetImageList(&m_ilNavRowHeight, LVSIL_SMALL);
		if (hOld)
			ImageList_Destroy(hOld);
		m_iNavRowHeight = iRow;
		// Force a resize so the list recalculates its scrollbars.
		CRect rcList;
		m_ctlNav.GetWindowRect(&rcList);
		ScreenToClient(&rcList);
		m_ctlNav.SetWindowPos(NULL, rcList.left, rcList.top, rcList.Width(), rcList.Height() - 1, SWP_NOZORDER | SWP_NOACTIVATE);
		m_ctlNav.SetWindowPos(NULL, rcList.left, rcList.top, rcList.Width(), rcList.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		m_ctlNav.EnsureVisible(0, FALSE);
	}
	CRect rcCli;
	m_ctlNav.GetClientRect(&rcCli);
	if (rcCli.Width() > 0 && m_ctlNav.GetColumnWidth(0) != rcCli.Width())
		m_ctlNav.SetColumnWidth(0, rcCli.Width());
}

void CAxisXDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CPropertySheet::OnGetMinMaxInfo(lpMMI);
	if (m_szNatural.cx > 0)
	{
		// Minimum size is 70 % of the natural size.
		lpMMI->ptMinTrackSize.x = m_szNatural.cx * 7 / 10;
		lpMMI->ptMinTrackSize.y = m_szNatural.cy * 7 / 10;
	}
}
// Rectangle of a footer slot (0..m_nFooterVisible-1); used for painting and hit-testing.
CRect CAxisXDlg::GetFooterSlotRect(const CRect& rcClient, int iSlot)
{
	int nCount = max(1, m_nFooterVisible);
	int slotW = SIDEBAR_WIDTH / nCount;
	CRect rcFooter(0, rcClient.bottom - SIDEBAR_FOOTER_HEIGHT, SIDEBAR_WIDTH, rcClient.bottom);
	return CRect(rcFooter.left + iSlot * slotW, rcFooter.top, rcFooter.left + (iSlot + 1) * slotW, rcFooter.bottom);
}

// Updates the footer tooltip rectangles to match GetFooterSlotRect().
void CAxisXDlg::UpdateFooterTooltips()
{
	if (!m_tipFooter.GetSafeHwnd())
		return;
	CRect rc;
	GetClientRect(&rc);
	for (int i = 0; i < m_nFooterVisible; i++)
	{
		CRect rcSlot = GetFooterSlotRect(rc, i);
		if (m_tipFooter.GetToolCount() > (UINT) i)
			m_tipFooter.SetToolRect(this, 100 + i, rcSlot);
		else
			m_tipFooter.AddTool(this, m_aFooterTip[i], &rcSlot, 100 + i);
	}
}

void CAxisXDlg::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheet::OnSize(nType, cx, cy);
	if (nType == SIZE_MINIMIZED)
		return;
	LayoutActivePage();
	UpdateFooterTooltips();
	// Remember the window size for the next start.
	if (m_bLayoutSized)
	{
		Main->PutRegistryDword("Window Maximized", nType == SIZE_MAXIMIZED ? 1 : 0);
		if (nType == SIZE_RESTORED)
		{
			CRect rcWnd;
			GetWindowRect(&rcWnd);
			Main->PutRegistryDword("Window Width", rcWnd.Width());
			Main->PutRegistryDword("Window Height", rcWnd.Height());
		}
	}
	Invalidate();
}

// -1 if the point isn't over any footer icon slot.
int CAxisXDlg::FooterHitTest(CPoint point)
{
	CRect rc;
	GetClientRect(&rc);
	if (point.y < rc.bottom - SIDEBAR_FOOTER_HEIGHT || point.x >= SIDEBAR_WIDTH)
		return -1;
	for (int i = 0; i < m_nFooterVisible; i++)
		if (GetFooterSlotRect(rc, i).PtInRect(point))
			return i;
	return -1;
}

void CAxisXDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CPropertySheet::OnMouseMove(nFlags, point);

	int iHit = FooterHitTest(point);
	if (iHit != m_iFooterHover)
	{
		m_iFooterHover = iHit;
		CRect rc;
		GetClientRect(&rc);
		CRect rcFooter(0, rc.bottom - SIDEBAR_FOOTER_HEIGHT, SIDEBAR_WIDTH, rc.bottom);
		InvalidateRect(&rcFooter, TRUE);
	}
	if (iHit >= 0)
	{
		TRACKMOUSEEVENT tme;
		memset(&tme, 0, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CAxisXDlg::OnMouseLeave()
{
	if (m_iFooterHover != -1)
	{
		m_iFooterHover = -1;
		CRect rc;
		GetClientRect(&rc);
		CRect rcFooter(0, rc.bottom - SIDEBAR_FOOTER_HEIGHT, SIDEBAR_WIDTH, rc.bottom);
		InvalidateRect(&rcFooter, TRUE);
	}
}

void CAxisXDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPropertySheet::OnLButtonDown(nFlags, point);

	int iHit = FooterHitTest(point);
	if (iHit < 0)
		return;
	switch (m_aFooterAction[iHit])
	{
	case 0: OnOpenSettingsPage(0); break;
	case 1: ShowProfileMenu(); break;
	case 2: OnHelp(); break;
	case 3: OnAboutDlg(); break;
	case 4: OnClose(); break;
	case 5: AxisOpenDonatePage(); break;
	}
}

// Applies the optional custom window icon and sidebar logo; falls back to the
// built-in branding when unset or not loadable.
void CAxisXDlg::ApplyCustomBranding()
{
	CString csIcon = Main->ResolveBrandingFile(Main->m_csCustomIcon, _T("custom_icon.ico"));
	if ( !csIcon.IsEmpty() )
	{
		HICON hIcon = (HICON) LoadImage(NULL, csIcon, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
		if ( hIcon != NULL )
		{
			m_hIcon = hIcon;
			SetIcon(m_hIcon, TRUE);
			SetIcon(m_hIcon, FALSE);
		}
		else
			Main->m_log.Add(1, AXT("WARNING: Unable to load custom icon %s"), (LPCTSTR) csIcon);
	}

	if ( !m_imgBrandLogo.IsNull() )
		m_imgBrandLogo.Destroy();
	CString csLogo = Main->ResolveBrandingFile(Main->m_csCustomLogo, _T("custom_logo.img"));
	if ( !csLogo.IsEmpty() )
	{
		if ( FAILED(m_imgBrandLogo.Load(csLogo)) )
			Main->m_log.Add(1, AXT("WARNING: Unable to load custom logo %s"), (LPCTSTR) csLogo);
	}

	if ( GetSafeHwnd() )
		Invalidate();
}

// Shows the Profiles submenu as a popup at the cursor.
void CAxisXDlg::ShowProfileMenu()
{
	Main->UpdateProfileMenu();
	CMenu * pSub = Main->pDefMenu.GetSubMenu(1);
	if (pSub == NULL)
	{
		OnOpenProfileOption();
		return;
	}
	CPoint pt;
	GetCursorPos(&pt);
	pSub->TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, this);
}

// Paints the sheet background, sidebar brand header, footer and status line.
BOOL CAxisXDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(&rc);

	CRect rcSidebar(rc.left, rc.top, SIDEBAR_WIDTH, rc.bottom);
	CRect rcContent(SIDEBAR_WIDTH, rc.top, rc.right, rc.bottom);
	static CBrush s_sidebarBrush(SidebarBkColor());
	static CBrush s_pageBrush(DarkPageBkColor());
	pDC->FillRect(&rcSidebar, &s_sidebarBrush);
	pDC->FillRect(&rcContent, &s_pageBrush);

	if (!m_imgBrandLogo.IsNull())
	{
		// Custom logo, scaled down to fit (aspect ratio preserved).
		int iMaxW = SIDEBAR_WIDTH - 24;
		int iMaxH = SIDEBAR_BRAND_HEIGHT - 14;
		int iW = m_imgBrandLogo.GetWidth();
		int iH = m_imgBrandLogo.GetHeight();
		if (iW > 0 && iH > 0)
		{
			double fScale = min((double) iMaxW / iW, (double) iMaxH / iH);
			if (fScale > 1.0)
				fScale = 1.0;
			int iDrawW = (int)(iW * fScale);
			int iDrawH = (int)(iH * fScale);
			int iOldMode = pDC->SetStretchBltMode(HALFTONE);
			m_imgBrandLogo.Draw(pDC->GetSafeHdc(), 14, (SIDEBAR_BRAND_HEIGHT - iDrawH) / 2, iDrawW, iDrawH);
			pDC->SetStretchBltMode(iOldMode);
		}
	}
	else
	{
		DrawIconEx(pDC->GetSafeHdc(), 14, (SIDEBAR_BRAND_HEIGHT - 28) / 2, m_hIcon, 28, 28, 0, NULL, DI_NORMAL);

		static CFont s_fontBrand;
		if (s_fontBrand.GetSafeHandle() == NULL)
			s_fontBrand.CreateFont(19, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, _T("Constantia"));
		CFont* pOldFont = pDC->SelectObject(&s_fontBrand);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(AxisClr(AXC_HEAD));
		CRect rcText(52, 0, SIDEBAR_WIDTH - 6, SIDEBAR_BRAND_HEIGHT);
		pDC->DrawText(_T("Axis X"), &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
		pDC->SelectObject(pOldFont);
	}

	// Divider under the header and between sidebar and content.
	CPen pen(PS_SOLID, 1, SidebarDividerColor());
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->MoveTo(0, SIDEBAR_BRAND_HEIGHT - 1);
	pDC->LineTo(rc.right, SIDEBAR_BRAND_HEIGHT - 1);
	pDC->MoveTo(SIDEBAR_WIDTH, SIDEBAR_BRAND_HEIGHT - 1);
	pDC->LineTo(SIDEBAR_WIDTH, rc.bottom);
	pDC->SelectObject(pOldPen);

	// Footer icon row under the nav list.
	CRect rcFooter(0, rc.bottom - SIDEBAR_FOOTER_HEIGHT, SIDEBAR_WIDTH, rc.bottom);
	CPen penFooter(PS_SOLID, 1, SidebarDividerColor());
	CPen* pOldPenFooter = pDC->SelectObject(&penFooter);
	pDC->MoveTo(rcFooter.left, rcFooter.top);
	pDC->LineTo(rcFooter.right, rcFooter.top);
	pDC->SelectObject(pOldPenFooter);

	for (int i = 0; i < m_nFooterVisible; i++)
	{
		CRect rcSlot = GetFooterSlotRect(rc, i);
		bool bHover = (i == m_iFooterHover);
		if (bHover)
		{
			CRect rcPill = rcSlot;
			rcPill.DeflateRect(6, 6);
			HBRUSH hHoverBg = CreateSolidBrush(AxisClr(AXC_BUTTON_HOVER));
			FillRect(pDC->GetSafeHdc(), &rcPill, hHoverBg);
			DeleteObject(hHoverBg);
		}
		CRect rcIcon(0, 0, 15, 15);
		CPoint ptCenter = rcSlot.CenterPoint();
		rcIcon.OffsetRect(ptCenter.x - 7, ptCenter.y - 7);
		// The Ko-fi heart uses its own colour.
		COLORREF crIcon = (m_aFooterIcon[i] == 17) ? AxisClr(AXC_HEART) : DarkTextColor();
		DrawNavIcon(pDC->GetSafeHdc(), m_aFooterIcon[i], rcIcon, bHover ? AxisAccentTextColor() : crIcon);
	}

	// Status line
	CRect rcStatus = GetStatusRect();
	CPen penStatus(PS_SOLID, 1, SidebarDividerColor());
	CPen* pOldPenStatus = pDC->SelectObject(&penStatus);
	pDC->MoveTo(rcStatus.left, rcStatus.top);
	pDC->LineTo(rcStatus.right, rcStatus.top);
	pDC->SelectObject(pOldPenStatus);
	if (!m_csStatus.IsEmpty())
	{
		CFont* pOldFont = pDC->SelectObject(&m_fntNavItem);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(m_crStatus);
		CRect rcText = rcStatus;
		rcText.top += 1;
		pDC->DrawText(m_csStatus, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
		pDC->SelectObject(pOldFont);
	}

	return TRUE;
}

CRect CAxisXDlg::GetStatusRect()
{
	CRect rc;
	GetClientRect(&rc);
	return CRect(SIDEBAR_WIDTH + SIDEBAR_MARGIN, rc.bottom - STATUS_HEIGHT, rc.right - SIDEBAR_MARGIN, rc.bottom - 2);
}

void CAxisXDlg::SetStatus(LPCTSTR pszText, COLORREF crText)
{
	m_csStatus = pszText;
	m_crStatus = crText;
	if (GetSafeHwnd())
	{
		CRect rc = GetStatusRect();
		InvalidateRect(&rc, TRUE);
	}
}

// Sets the status line text. iKind: 0 = info, 1 = ok, 2 = warning, 3 = error.
void AxisSetStatus(LPCTSTR pszText, int iKind)
{
	CAxisXDlg* pDlg = DYNAMIC_DOWNCAST(CAxisXDlg, AfxGetMainWnd());
	if (pDlg == NULL)
		return;
	COLORREF cr = DarkTextColor();
	switch (iKind)
	{
	case 1: cr = AxisClr(AXC_OK); break;
	case 2: cr = AxisAccentTextColor(); break;
	case 3: cr = AxisClr(AXC_ERROR); break;
	}
	CString csText;
	csText.Format(_T("%s   %s"), (LPCTSTR) CTime::GetCurrentTime().Format(_T("%H:%M:%S")), pszText);
	pDlg->SetStatus(csText, cr);
}

// Prevents selecting group header rows (lParam == -1).
void CAxisXDlg::OnNavSelChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pNM = (NMLISTVIEW*) pNMHDR;
	*pResult = FALSE;
	if ((pNM->uNewState & LVIS_SELECTED) && m_ctlNav.GetItemData(pNM->iItem) == -1)
		*pResult = TRUE;
}

void CAxisXDlg::OnNavSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pNM = (NMLISTVIEW*) pNMHDR;
	*pResult = 0;
	if (m_bSyncingNav)
		return;
	// A click on a group header clears the selection; restore it afterwards.
	if ((pNM->uOldState & LVIS_SELECTED) && !(pNM->uNewState & LVIS_SELECTED))
	{
		PostMessage(WM_APP + 96);
		return;
	}
	if (!(pNM->uNewState & LVIS_SELECTED) || (pNM->uOldState & LVIS_SELECTED))
		return;
	LPARAM lp = (LPARAM) m_ctlNav.GetItemData(pNM->iItem);
	if (lp == -1)
		return;
	ShowPage(NAV_CURRENT_INDEX(lp));
}

// Switches pages with redraw disabled, so the page appears once at its final size.
void CAxisXDlg::ShowPage(int iIndex)
{
	if (iIndex < 0 || iIndex >= GetPageCount())
		return;
	if (iIndex == GetActiveIndex())
	{
		RepositionLayout();
		return;
	}
	SetRedraw(FALSE);
	SetActivePage(iIndex);
	CDockingPage * pDock = DYNAMIC_DOWNCAST(CDockingPage, GetActivePage());
	if (pDock && pDock->GetSafeHwnd())
		pDock->EnsureLayoutCaptured();
	RepositionLayout();
	SetRedraw(TRUE);

	// Redraw only the page area (including the status line), not the sidebar.
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcPageArea(SIDEBAR_WIDTH, 0, rcClient.right, rcClient.bottom);
	RedrawWindow(&rcPageArea, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
	// Repaint the sidebar selection, which changed while redraw was off.
	if (m_ctlNav.GetSafeHwnd())
		m_ctlNav.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

LRESULT CAxisXDlg::OnNavResync(WPARAM, LPARAM)
{
	if (m_ctlNav.GetSafeHwnd() && m_ctlNav.GetNextItem(-1, LVNI_SELECTED) == -1)
		SyncSidebarSelection();
	return 0;
}

// Rebuilds the sidebar rows; undocked pages are skipped. Group headers are plain
// rows (lParam == -1) because list-view groups need a Unicode build.
void CAxisXDlg::RebuildSidebarItems()
{
	if (!m_ctlNav.GetSafeHwnd())
		return;

	m_bSyncingNav = true; // rebuilding must not trigger OnNavSelChanged()
	m_ctlNav.SetRedraw(FALSE);
	m_ctlNav.DeleteAllItems();

	int nEntries = sizeof(g_axisNavLayout) / sizeof(g_axisNavLayout[0]);
	int nGroups = sizeof(g_axisNavGroupNames) / sizeof(g_axisNavGroupNames[0]);
	int nItem = 0;
	int nPageCount = GetPageCount();
	int nLastGroup = -1;
	for (int e = 0; e < nEntries; e++)
	{
		if (!g_axisNavLayout[e].visible)
			continue;

		CPropertyPage* pPage = NavPageFromOriginalIndex(g_axisNavLayout[e].page);
		if (pPage == NULL)
			continue;

		// The page's current index in the sheet (undocked pages are absent).
		int currentIndex = -1;
		for (int p = 0; p < nPageCount; p++)
		{
			if (GetPage(p) == pPage)
			{
				currentIndex = p;
				break;
			}
		}
		if (currentIndex < 0)
			continue;

		int group = g_axisNavLayout[e].group;
		if (group != nLastGroup && group >= 0 && group < nGroups)
		{
			LVITEM lviHead;
			memset(&lviHead, 0, sizeof(lviHead));
			lviHead.mask = LVIF_TEXT | LVIF_PARAM;
			lviHead.iItem = nItem;
			lviHead.pszText = (LPTSTR) AxisTr(g_axisNavGroupNames[group]);
			lviHead.lParam = -1;
			m_ctlNav.InsertItem(&lviHead);
			nItem++;
			nLastGroup = group;
		}

		LPCTSTR pszTitle = (pPage->m_psp.pszTitle) ? pPage->m_psp.pszTitle : _T("(untitled)");

		LVITEM lvi;
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem = nItem;
		CString csIndented;
		csIndented.Format(_T("   %s"), pszTitle);
		lvi.pszText = (LPTSTR)(LPCTSTR) csIndented;
		lvi.lParam = NAV_LPARAM(currentIndex, g_axisNavLayout[e].page);
		m_ctlNav.InsertItem(&lvi);
		nItem++;
	}

	m_ctlNav.SetRedraw(TRUE);
	m_ctlNav.Invalidate();
	m_bSyncingNav = false;

	SyncSidebarSelection();
}

// Selects the sidebar row of the active page.
void CAxisXDlg::SyncSidebarSelection()
{
	if (!m_ctlNav.GetSafeHwnd() || m_ctlNav.GetItemCount() == 0)
		return;
	int iActive = GetActiveIndex();
	m_bSyncingNav = true;
	for (int i = 0; i < m_ctlNav.GetItemCount(); i++)
	{
		LPARAM lp = (LPARAM) m_ctlNav.GetItemData(i);
		if (lp == -1)
			continue;
		UINT uState = (NAV_CURRENT_INDEX(lp) == iActive) ? (LVIS_SELECTED | LVIS_FOCUSED) : 0;
		m_ctlNav.SetItemState(i, uState, LVIS_SELECTED | LVIS_FOCUSED);
	}
	m_bSyncingNav = false;
}

void CAxisXDlg::OnNavCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*) pNMHDR;
	switch (pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT:
		{
			int iItem = (int) pLVCD->nmcd.dwItemSpec;
			HDC hdc = pLVCD->nmcd.hdc;
			int code = (int) m_ctlNav.GetItemData(iItem);

			RECT rc;
			m_ctlNav.GetItemRect(iItem, &rc, LVIR_BOUNDS);

			if (code == -1)
			{
				// Group header: muted text at the bottom of the row (the first one centred).
				HBRUSH hBg = CreateSolidBrush(SidebarBkColor());
				FillRect(hdc, &rc, hBg);
				DeleteObject(hBg);

				TCHAR szHeadText[256];
				m_ctlNav.GetItemText(iItem, 0, szHeadText, 256);
				::SelectObject(hdc, (HFONT) m_fntNavGroup.GetSafeHandle());
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, AxisClr(AXC_MUTED));

				RECT rcText = rc;
				rcText.left += 18;
				rcText.right -= 8;
				if (iItem == 0)
				{
					DrawText(hdc, szHeadText, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
				}
				else
				{
					rcText.bottom -= 7;
					DrawText(hdc, szHeadText, -1, &rcText, DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX);
				}

				*pResult = CDRF_SKIPDEFAULT;
				return;
			}

			// Page row: icon followed by the page title.
			BOOL bSelected = (m_ctlNav.GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
			COLORREF crBg = bSelected ? AxisClr(AXC_BUTTON_HOVER) : SidebarBkColor();
			COLORREF crFg = bSelected ? AxisAccentTextColor() : DarkTextColor();

			HBRUSH hBg = CreateSolidBrush(crBg);
			FillRect(hdc, &rc, hBg);
			DeleteObject(hBg);

			if (bSelected)
			{
				RECT rcAccent = rc;
				rcAccent.right = rcAccent.left + 3;
				HBRUSH hAccent = CreateSolidBrush(DarkAccentColor());
				FillRect(hdc, &rcAccent, hAccent);
				DeleteObject(hAccent);
			}

			int cy = (rc.top + rc.bottom) / 2;
			int iconType = FindNavIcon(NAV_ORIGINAL_INDEX(code));
			if (iconType < 0)
				iconType = 0;
			RECT rcIcon = { rc.left + 18, cy - 7, rc.left + 32, cy + 7 };
			DrawNavIcon(hdc, iconType, rcIcon, crFg);

			TCHAR szText[256];
			m_ctlNav.GetItemText(iItem, 0, szText, 256);
			LPCTSTR pszTrim = szText;
			while (*pszTrim == _T(' '))
				pszTrim++;

			RECT rcText = rc;
			rcText.left += 38;
			::SelectObject(hdc, (HFONT) m_fntNavItem.GetSafeHandle());
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, crFg);
			DrawText(hdc, pszTrim, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

			*pResult = CDRF_SKIPDEFAULT;
			return;
		}
	}
	*pResult = CDRF_DODEFAULT;
}

CAxisXDlg::CAxisXDlg()
{
	CAxisXDlg(CFMsg(CMsg("IDS_AXISTITLE"), Main->GetVersionTitle()));
}

int CAxisXDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_nid.cbSize = sizeof(m_nid);
	m_nid.hWnd = this->GetSafeHwnd();
	m_nid.uID = 1;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_USER;
	m_nid.hIcon = m_hIcon;
	CString csTip;
	csTip.Format("%s (%s)", Main->GetVersionTitle(), Main->m_csCurentProfile);
	strcpy_s(m_nid.szTip,sizeof(m_nid.szTip), csTip);
	Shell_NotifyIcon(NIM_ADD, &m_nid);
	m_bModeless = 1;
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	ModifyStyle(0, WS_MINIMIZEBOX);

	Main->m_dlgToolBar = new CAxisXLBar;
	Main->m_dlgToolBar->Create(IDD_TOOLBAR);
	Main->m_dlgToolBar->ShowWindow(SW_HIDE);
	
	return 0;
}


LRESULT CAxisXDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_USER)
	{
		if (lParam == WM_LBUTTONDOWN)
		{
			RestoreFromMiniBar(NULL);
			return TRUE;
		}
	}
	if (message == WM_SYSCOMMAND)
	{
		// Minimize to the mini bar unless it is disabled in the settings.
		if ((wParam & 0xFFF0) == SC_MINIMIZE && !Main->m_dwDisableToolbar && Main->m_dlgToolBar)
		{
			this->ShowWindow(SW_HIDE);
			((CAxisXLBar*) Main->m_dlgToolBar)->ShowBar();
			return TRUE;
		}
	}
	if (message == WM_DESTROY)
	{
		this->PressButton(PSBTN_OK);
		return TRUE;
	}
	return CPropertySheet::DefWindowProc(message, wParam, lParam);
}

// Restores the main window from the mini bar or tray, optionally on a given page.
// SW_SHOW keeps a maximized window maximized.
void CAxisXDlg::RestoreFromMiniBar(CPropertyPage* pPage)
{
	if (Main->m_dlgToolBar && Main->m_dlgToolBar->GetSafeHwnd())
		Main->m_dlgToolBar->ShowWindow(SW_HIDE);
	ShowWindow(SW_SHOW);
	if (IsIconic())
		ShowWindow(SW_RESTORE);
	SetForegroundWindow();
	if (pPage)
	{
		ShowPage(pPage);
		SyncSidebarSelection();
	}
}

void CAxisXDlg::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
	{
		if (Main->m_dwSysClose)
			this->PressButton(PSBTN_OK);
		else
			ShowWindow(SW_HIDE);
		return;
	}
	CPropertySheet::OnSysCommand(nID, lParam);
}

void CAxisXDlg::OnMove(int x, int y) 
{
	CPropertySheet::OnMove(x, y);

	if ( !m_bInit )
		return;

	WINDOWPLACEMENT place;
	this->GetWindowPlacement(&place);
	HKEY hKey;
	DWORD dwDisp;
	LONG lStatus = RegCreateKeyEx(hRegLocation, REGKEY_AXIS, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);
	if (lStatus == ERROR_SUCCESS)
	{
		Main->m_csPosition.Format("%ld,%ld", place.rcNormalPosition.left, place.rcNormalPosition.top);
		lStatus = RegSetValueEx( hKey, "Position", 0, REG_SZ, ((BYTE *) Main->m_csPosition.GetBuffer(Main->m_csPosition.GetLength())), Main->m_csPosition.GetLength() );
	}
	RegCloseKey( hKey );
}

void CAxisXDlg::UpdateTip()
{
	m_nid.cbSize = sizeof(m_nid);
	m_nid.hWnd = this->GetSafeHwnd();
	m_nid.uID = 1;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_USER;
	m_nid.hIcon = m_hIcon;
	CString csTip;
	csTip.Format("%s (%s)", Main->GetVersionTitle(), Main->m_csCurentProfile);
	strcpy_s(m_nid.szTip,sizeof(m_nid.szTip), csTip);
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}


//Settings Menu

void CAxisXDlg::OnOpenSettingsPage(int iPage)
{
	CSettingsDlg dlg(CMsg("IDS_SETTINGS"));
	dlg.m_psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	dlg.EnableStackedTabs( false );
	dlg.AddPage(Main->m_pcppSetGeneral);
	dlg.AddPage(Main->m_pcppSetPaths);
	dlg.AddPage(Main->m_pcppSetItem);
	dlg.AddPage(Main->m_pcppSetTravel);
	dlg.AddPage(Main->m_pcppSetSpawn);
	dlg.AddPage(Main->m_pcppSetOverridePaths);

	dlg.SetActivePage(iPage);
	dlg.DoModal();
}

void CAxisXDlg::OnSettingsGeneral()
{
	OnOpenSettingsPage(0);
}

void CAxisXDlg::OnSettingsPaths()
{
	OnOpenSettingsPage(1);
}

void CAxisXDlg::OnSettingsItem()
{
	OnOpenSettingsPage(2);
}

void CAxisXDlg::OnSettingsTravel()
{
	OnOpenSettingsPage(3);
}

void CAxisXDlg::OnSettingsSpawn()
{
	OnOpenSettingsPage(4);
}

void CAxisXDlg::OnSettingsOverridePaths()
{
	OnOpenSettingsPage(5);
}


//Profile Menu
void CAxisXDlg::OnOpenProfileOption()
{
	if (Main->m_dlgProfile)
		delete Main->m_dlgProfile;
	Main->m_dlgProfile = new CProfileDLG;
	Main->m_dlgProfile->Create(IDD_PROFILE_DLG);
}

void CAxisXDlg::OnUnloadProfile()
{
	Main->m_pScripts->UnloadProfile();
	Main->LoadIni(1);
	Main->LoadIni(2);
	Main->m_csCurentProfile = CMsg(IDS_NONE);
	UpdateTip();
	AfxBeginThread(LoadProfileThread,(LPVOID)0);
}

void CAxisXDlg::OnLoadDefProfile()
{
	Main->m_pScripts->UnloadProfile();
	Main->LoadIni(1);
	Main->LoadIni(2);
	Main->m_csCurentProfile = Main->GetRegistryString("Default Profile");
	UpdateTip();
	AfxBeginThread(LoadProfileThread,(LPVOID)1);
}

void CAxisXDlg::OnLoadLastProfile()
{
	Main->m_pScripts->UnloadProfile();
	Main->LoadIni(1);
	Main->LoadIni(2);
	Main->m_csCurentProfile = Main->GetRegistryString("Last Profile Loaded");
	UpdateTip();
	AfxBeginThread(LoadProfileThread,(LPVOID)0);
}


//Help Menu

void CAxisXDlg::OnHelp()
{
	HtmlHelp((DWORD_PTR)"AxisX.chm::/Welcome.htm", HH_DISPLAY_TOPIC);
}

void CAxisXDlg::OnAboutDlg() 
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CAxisXDlg::OnClose()
{
	PressButton(PSBTN_OK);
}

// Result of an update check: shows the update on the overview. Checks started
// by the user also report "up to date" and errors.
LRESULT CAxisXDlg::OnUpdateChecked(WPARAM wParam, LPARAM lParam)
{
	CString csText;
	if (wParam == 1)
	{
		if (Main->m_pcppDashboardTab != NULL)
			Main->m_pcppDashboardTab->ShowUpdate();
		csText.Format(AXT("Axis X %s ist verf\xFCgbar - \"Jetzt aktualisieren\" auf der \xDC" "bersicht."), (LPCTSTR) g_axisUpdate.csVersion);
		AxisSetStatus(csText, 2);
		Main->m_log.Add(2, "%s", (LPCTSTR) csText);
	}
	else if (lParam != 0)
	{
		if (wParam == 0)
		{
			csText.Format(AXT("Axis X ist aktuell (Version %s)."), (LPCTSTR) AxisCurrentVersion());
			AxisSetStatus(csText, 1);
		}
		else
		{
			csText.Format(AXT("Update-Suche fehlgeschlagen: %s"), (LPCTSTR) g_axisUpdate.csError);
			AxisSetStatus(csText, 3);
		}
	}
	return 0;
}

// Download progress; once the installer is verified it is started and Axis closes.
LRESULT CAxisXDlg::OnUpdateDownload(WPARAM wParam, LPARAM /*lParam*/)
{
	CString csText;
	if (wParam <= 100)
	{
		csText.Format(AXT("Update wird geladen ... %d %%"), (int) wParam);
		AxisSetStatus(csText, 0);
		return 0;
	}
	if (Main->m_pcppDashboardTab != NULL)
		Main->m_pcppDashboardTab->ShowUpdate();
	if (wParam == 101 && AxisRunUpdateInstaller())
	{
		PressButton(PSBTN_OK);
		return 0;
	}
	csText = (wParam == 101) ? CString(AXT("Der Installer wurde nicht gestartet."))
		: AXT("Das Update konnte nicht geladen werden: ") + g_axisUpdate.csError;
	AxisSetStatus(csText, 3);
	if (AfxMessageBox(csText + _T("\n\n") + AXT("Die Download-Seite im Browser \xF6" "ffnen?"), MB_YESNO | MB_ICONWARNING) == IDYES)
		ShellExecute(NULL, _T("open"), g_axisUpdate.csPageUrl.IsEmpty() ? AXIS_RELEASES_PAGE : (LPCTSTR) g_axisUpdate.csPageUrl, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

// Label check: returns true and a report line if a label's text does not fit its control.
static bool AxisLabelTooSmall(CWnd* pChild, LPCTSTR pszWhere, CString& csLine)
{
	if (!pChild->IsWindowVisible())
		return false;
	CString csText;
	pChild->GetWindowText(csText);
	if (csText.IsEmpty())
		return false;
	TCHAR szClass[32] = { 0 };
	::GetClassName(pChild->GetSafeHwnd(), szClass, 32);
	DWORD dwStyle = pChild->GetStyle();
	int iPadW = 0;
	bool bWrap = false, bCheckHeight = true;
	if (_tcsicmp(szClass, _T("Static")) == 0)
	{
		DWORD dwType = dwStyle & SS_TYPEMASK;
		if (dwType != SS_LEFT && dwType != SS_CENTER && dwType != SS_RIGHT && dwType != SS_LEFTNOWORDWRAP && dwType != SS_SIMPLE)
			return false;
		bWrap = (dwType == SS_LEFT || dwType == SS_CENTER || dwType == SS_RIGHT);
	}
	else if (_tcsicmp(szClass, _T("Button")) == 0)
	{
		DWORD dwType = dwStyle & 0x0F;
		if (dwType == BS_GROUPBOX)
		{
			iPadW = 24;
			bCheckHeight = false;
		}
		else if (dwType == BS_CHECKBOX || dwType == BS_AUTOCHECKBOX || dwType == BS_3STATE || dwType == BS_AUTO3STATE
			|| dwType == BS_RADIOBUTTON || dwType == BS_AUTORADIOBUTTON)
		{
			iPadW = 22;	// Box plus spacing
			bWrap = (dwStyle & BS_MULTILINE) != 0;
		}
		else if (dwType == BS_PUSHBUTTON || dwType == BS_DEFPUSHBUTTON)
			iPadW = 8;
		else
			return false;
	}
	else
		return false;

	CRect rc;
	pChild->GetClientRect(&rc);
	CClientDC dc(pChild);
	CFont* pFont = pChild->GetFont();
	CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
	CString csDraw = csText;
	csDraw.Replace(_T("&&"), _T("\x01"));
	csDraw.Remove(_T('&'));
	csDraw.Replace(_T('\x01'), _T('&'));
	int iAvailW = max(1, rc.Width() - iPadW);
	CRect rcNeed(0, 0, iAvailW, 0);
	dc.DrawText(csDraw, &rcNeed, DT_CALCRECT | DT_NOPREFIX | (bWrap ? DT_WORDBREAK : DT_SINGLELINE));
	if (pOldFont)
		dc.SelectObject(pOldFont);
	bool bTooWide = rcNeed.Width() > iAvailW + 1;
	bool bTooHigh = bCheckHeight && rcNeed.Height() > rc.Height() + 1;
	if (!bTooWide && !bTooHigh)
		return false;
	csText.Replace(_T("\r\n"), _T(" "));
	csText.Replace(_T("\n"), _T(" "));
	csLine.Format("%s\t%d\t%s\tbraucht %dx%d\that %dx%d\n", pszWhere, pChild->GetDlgCtrlID(),
		(LPCTSTR) csText, rcNeed.Width() + iPadW, rcNeed.Height(), rc.Width(), rc.Height());
	return true;
}

static void AxisWriteLabelReport(LPCTSTR pszFile, const CString& csOut)
{
	TCHAR szTemp[MAX_PATH];
	GetTempPath(MAX_PATH, szTemp);
	CStdioFile f;
	if (f.Open(CString(szTemp) + pszFile, CFile::modeCreate | CFile::modeWrite))
	{
		f.WriteString(csOut);
		f.Close();
	}
}

// Checks all visible descendants of a window.
static int AxisCheckLabelsDeep(CWnd* pParent, LPCTSTR pszWhere, CString& csOut)
{
	int nFound = 0;
	for (CWnd* pChild = pParent->GetWindow(GW_CHILD); pChild != NULL; pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		if (!pChild->IsWindowVisible())
			continue;
		CString csLine;
		if (AxisLabelTooSmall(pChild, pszWhere, csLine))
		{
			csOut += csLine;
			nFound++;
		}
		if (pChild->GetWindow(GW_CHILD))
			nFound += AxisCheckLabelsDeep(pChild, pszWhere, csOut);
	}
	return nFound;
}

// WM_APP+97: checks the labels on all pages and writes %TEMP%\axis_labels.txt.
LRESULT CAxisXDlg::OnCheckLabels(WPARAM, LPARAM)
{
	CString csOut, csLine;
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	csOut.Format("Fenster %dx%d\n", rcWnd.Width(), rcWnd.Height());
	int iOld = GetActiveIndex();
	int nFound = 0;
	for (int p = 0; p < GetPageCount(); p++)
	{
		ShowPage(p);
		CPropertyPage* pPage = GetPage(p);
		if (!pPage || !pPage->GetSafeHwnd())
			continue;
		CString csTitle = pPage->m_psp.pszTitle ? pPage->m_psp.pszTitle : _T("?");
		for (CWnd* pChild = pPage->GetWindow(GW_CHILD); pChild != NULL; pChild = pChild->GetWindow(GW_HWNDNEXT))
		{
			if (AxisLabelTooSmall(pChild, csTitle, csLine))
			{
				csOut += csLine;
				nFound++;
			}
		}
	}
	ShowPage(iOld);
	csLine.Format("%d zu knapp\n", nFound);
	csOut += csLine;
	AxisWriteLabelReport(_T("axis_labels.txt"), csOut);
	return nFound;
}

// WM_APP+98: checks the labels of all open dialogs except the main window and
// mini bar, and writes %TEMP%\axis_labels_popup.txt.
static BOOL CALLBACK AxisCollectPopups(HWND hWnd, LPARAM lParam)
{
	CArray<HWND, HWND>* pList = (CArray<HWND, HWND>*) lParam;
	if (::IsWindowVisible(hWnd))
		pList->Add(hWnd);
	return TRUE;
}

LRESULT CAxisXDlg::OnCheckPopupLabels(WPARAM, LPARAM)
{
	CArray<HWND, HWND> aWnd;
	EnumThreadWindows(GetCurrentThreadId(), AxisCollectPopups, (LPARAM) &aWnd);
	CString csOut;
	int nFound = 0, nWindows = 0;
	for (int i = 0; i < aWnd.GetSize(); i++)
	{
		HWND hWnd = aWnd[i];
		if (hWnd == m_hWnd || (Main->m_dlgToolBar && hWnd == Main->m_dlgToolBar->GetSafeHwnd()))
			continue;
		TCHAR szClass[32] = { 0 };
		::GetClassName(hWnd, szClass, 32);
		if (_tcsicmp(szClass, _T("#32770")) != 0)
			continue;	// Tooltips, IME windows, etc.
		CWnd* pWnd = CWnd::FromHandle(hWnd);
		CString csTitle;
		pWnd->GetWindowText(csTitle);
		CString csLine;
		csLine.Format("[%s]\n", (LPCTSTR) csTitle);
		csOut += csLine;
		nWindows++;
		nFound += AxisCheckLabelsDeep(pWnd, csTitle, csOut);
	}
	CString csEnd;
	csEnd.Format("%d Fenster, %d zu knapp\n", nWindows, nFound);
	csOut += csEnd;
	AxisWriteLabelReport(_T("axis_labels_popup.txt"), csOut);
	return nFound;
}

// WM_APP+99: shows page wParam and clicks button lParam on it. The click is
// posted so a modal dialog opens after this returns.
LRESULT CAxisXDlg::OnPressPageButton(WPARAM wParam, LPARAM lParam)
{
	ShowPage((int) wParam);
	CPropertyPage* pPage = GetActivePage();
	if (!pPage || !pPage->GetSafeHwnd())
		return 0;
	CWnd* pCtl = pPage->GetDlgItem((int) lParam);
	if (!pCtl)
		return 0;
	pPage->PostMessage(WM_COMMAND, MAKEWPARAM((int) lParam, BN_CLICKED), (LPARAM) pCtl->GetSafeHwnd());
	return 1;
}