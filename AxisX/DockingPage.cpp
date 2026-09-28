// DockingPage.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "DockingPage.h"
#include "AxisXDlg.h"


// CDockingPage dialog

IMPLEMENT_DYNAMIC(CDockingPage, CPropertyPage)

CDockingPage::CDockingPage(UINT id, CString csCaption)
	: CPropertyPage(id)
{
	template_id = id;
	m_psp.pszTitle = m_strCaption = csCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_bModifyDlgStylesAndPos = false;
	m_bAlwaysColor = false;
	csTitle = csCaption+" "+CMsg("IDS_TAB");
	m_dcDialogPage = NULL;
	m_bScaleReady = false;
	m_szBase = CSize(0, 0);
	m_iFontPct = 100;
	memset(&m_lfBase, 0, sizeof(m_lfBase));
}

CDockingPage::~CDockingPage()
{
	delete m_dcDialogPage;
}

void CDockingPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDockingPage, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	ON_MESSAGE(WM_APP + 95, OnCaptureLayout)
END_MESSAGE_MAP()

BOOL CDockingPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Without WS_CLIPCHILDREN the page paints over its controls until they are hovered.
	ModifyStyle(0, WS_CLIPCHILDREN);
	ApplyDarkMode(m_hWnd);

	// Base size is the template size; control positions are captured once the
	// derived page has finished its own OnInitDialog.
	CRect rcBase;
	GetClientRect(&rcBase);
	m_szBase = rcBase.Size();
	PostMessage(WM_APP + 95);

	if((m_bModifyDlgStylesAndPos == true) && m_hWnd)
	{
		SetWindowText(csTitle);
		CRect rectFrame, rectDlg;
		CWnd* pMainWnd = AfxGetMainWnd();
		if(pMainWnd != NULL)
		{
			pMainWnd->GetClientRect(rectFrame);
			pMainWnd->ClientToScreen(rectFrame);
			GetWindowRect(rectDlg);
			int nXPos = rectFrame.left + (rectFrame.Width() / 2) - (rectDlg.Width() / 2);
			int nYPos = rectFrame.top + (rectFrame.Height() / 2) - (rectDlg.Height() / 2);
			::SetWindowPos(m_hWnd, HWND_TOP, nXPos, nYPos, rectDlg.Width(), rectDlg.Height(), SWP_NOCOPYBITS);
		}
	}
	return TRUE;
}

void CDockingPage::OnRButtonDown(UINT nFlags, CPoint point) 
{
	UNREFERENCED_PARAMETER(nFlags);
	ClientToScreen(&point);
	HMENU hMenu = ::CreatePopupMenu();
	if (NULL != hMenu)
	{
		if (m_bModifyDlgStylesAndPos == true)
			::AppendMenu(hMenu, MF_STRING, 2, CMsg("IDS_DOCK"));
		else
			::AppendMenu(hMenu, MF_STRING, 1, CMsg("IDS_UNDOCK"));
		int sel = ::TrackPopupMenuEx(hMenu, 
				TPM_CENTERALIGN | TPM_RETURNCMD,
				point.x,
				point.y,
				m_hWnd,
				NULL);
		switch (sel)
		{
		case 1:
			{
				CPropertySheet * dlg = (CPropertySheet*)this->GetParent();
				dlg->RemovePage(this);
				m_dcDialogPage->m_bModifyDlgStylesAndPos = true;
				m_dcDialogPage->Create(template_id);
				// Page indices shifted; rebuild the sidebar rows.
				CAxisXDlg * pSheet = DYNAMIC_DOWNCAST(CAxisXDlg, dlg);
				if (pSheet != NULL)
					pSheet->RebuildSidebarItems();
			break;
			}
		case 2:
			{
			OnCancel();
			break;
			}
		}
		::DestroyMenu(hMenu);
	}
}

void CDockingPage::OnCancel()
{
	CPropertySheet * dlg = (CPropertySheet*)this->GetParent();
	dlg->AddPage(m_dcPropertyPage);
	FixTabs(template_id);
	dlg->SetActivePage(m_dcPropertyPage);
	// Page order restored; rebuild the sidebar rows.
	CAxisXDlg * pSheet = DYNAMIC_DOWNCAST(CAxisXDlg, dlg);
	if (pSheet != NULL)
		pSheet->RebuildSidebarItems();
	CDialog::OnCancel();
}

void CDockingPage::PreSubclassWindow() 
{
	if (m_bModifyDlgStylesAndPos == true)
	{
		if(m_hWnd != NULL)
		{
			LONG lStyle = GetWindowLong(m_hWnd, GWL_STYLE);
			lStyle &= ~WS_CHILD;
			lStyle &= ~WS_DISABLED;

			lStyle |= WS_POPUP;
			lStyle |= WS_VISIBLE;
			lStyle |= WS_SYSMENU;
			lStyle |= WS_MINIMIZEBOX;
			SetWindowLong(m_hWnd, GWL_STYLE, lStyle);  
		}
	}

	CPropertyPage::PreSubclassWindow();
}

HBRUSH CDockingPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	::SetBkMode(pDC->m_hDC,TRANSPARENT);

	// Dark colours for the parts SetWindowTheme() doesn't reach (page background,
	// static labels, edit boxes).
	static CBrush s_pageBrush(DarkPageBkColor());
	static CBrush s_fieldBrush(DarkFieldBkColor());
	switch (nCtlColor)
	{
	case CTLCOLOR_DLG:
		return (HBRUSH) s_pageBrush.GetSafeHandle();
	case CTLCOLOR_STATIC:
		pDC->SetTextColor(DarkTextColor());
		return (HBRUSH) s_pageBrush.GetSafeHandle();
	case CTLCOLOR_EDIT:
	case CTLCOLOR_LISTBOX:
		pDC->SetTextColor(DarkTextColor());
		pDC->SetBkColor(DarkFieldBkColor());
		return (HBRUSH) s_fieldBrush.GetSafeHandle();
	case CTLCOLOR_BTN:
	case CTLCOLOR_SCROLLBAR:
		pDC->SetTextColor(DarkTextColor());
		return (HBRUSH) s_pageBrush.GetSafeHandle();
	}

	// Everything else, including undocked pages, uses the dark page brush too.
	UNREFERENCED_PARAMETER(hbr);
	pDC->SetTextColor(DarkTextColor());
	return (HBRUSH) s_pageBrush.GetSafeHandle();
}

void CDockingPage::FixTabs(UINT iTab)
{
	CPropertySheet * dlg = (CPropertySheet*)this->GetParent();
	int iCurrent = 0;
	for(int ipages = 0; ipages < dlg->GetPageCount(); ipages++)
	{
		CPropertyPage * TestPage = dlg->GetPage(iCurrent);
		bool remove = false;

		switch (iTab)
		{
		case IDD_LOG_TAB:
			return;
		case IDD_DASHBOARD_TAB:
			{
				if(TestPage == Main->m_pcppGeneralTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_GENERAL_TAB:
			{
				if(TestPage == Main->m_pcppTravelTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_TRAVEL_TAB:
			{
				if(TestPage == Main->m_pcppSpawnTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_SPAWN_TAB:
			{
				if(TestPage == Main->m_pcppPlayerTweakTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_PLAYERTWEAK_TAB:
			{
				if(TestPage == Main->m_pcppItemTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_ITEMS_TAB:
			{
				if(TestPage == Main->m_pcppItemTweakTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_ITEMTWEAK_TAB:
			{
				if(TestPage == Main->m_pcppAccountTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_ACCOUNT_TAB:
			{
				if(TestPage == Main->m_pcppMiscTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_MISC_TAB:
			{
				if(TestPage == Main->m_pcppLauncherTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_LAUNCHER_TAB:
			{
				if(TestPage == Main->m_pcppCommandsTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_COMMANDS_TAB:
			{
				if(TestPage == Main->m_pcppReminderTab)
				{
					remove = true;
					break;
				}
			}
		case IDD_REMINDER_TAB:
			{
				if(TestPage == Main->m_pcppAxisLogTab)
				{
					remove = true;
					break;
				}
			}
		}
		
		if( remove )
		{
			dlg->RemovePage(TestPage);
			dlg->AddPage(TestPage);
		}
		else
			iCurrent++;
	}
}

BOOL CDockingPage::OnSetActive()
{
	// Only change the z-order when needed, to avoid flicker on page switches.
	bool bTopMost = (Main->m_pMainWnd->GetExStyle() & WS_EX_TOPMOST) != 0;
	if (bTopMost != (Main->m_dwAlwaysOnTop != 0))
		Main->m_pMainWnd->SetWindowPos(Main->m_dwAlwaysOnTop ? &CWnd::wndTopMost : &CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	// Keep the sidebar selection in sync however the page was switched.
	CAxisXDlg * pSheet = DYNAMIC_DOWNCAST(CAxisXDlg, GetParent());
	if (pSheet != NULL)
		pSheet->SyncSidebarSelection();
	return CPropertyPage::OnSetActive();
}

// ---- Proportional page scaling ----

LRESULT CDockingPage::OnCaptureLayout(WPARAM, LPARAM)
{
	if (m_bScaleReady || m_szBase.cx <= 0 || m_szBase.cy <= 0)
		return 0;
	CFont * pPageFont = GetFont();
	HFONT hPageFont = pPageFont ? (HFONT) pPageFont->GetSafeHandle() : NULL;
	if (pPageFont)
		pPageFont->GetLogFont(&m_lfBase);
	m_aScale.RemoveAll();
	for (CWnd * pChild = GetWindow(GW_CHILD); pChild != NULL; pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		CScaleItem item;
		item.hWnd = pChild->GetSafeHwnd();
		pChild->GetWindowRect(&item.rc);
		ScreenToClient(&item.rc);
		HFONT hFont = (HFONT) pChild->SendMessage(WM_GETFONT, 0, 0);
		item.bBaseFont = (hFont == NULL || hFont == hPageFont);
		// RichEdit resets the text colour on WM_SETFONT, so only move/resize it.
		TCHAR szClass[32];
		if (::GetClassName(item.hWnd, szClass, 32) > 0 && _tcsnicmp(szClass, _T("RichEdit"), 8) == 0)
			item.bBaseFont = false;
		m_aScale.Add(item);
	}
	m_bScaleReady = true;
	CRect rc;
	GetClientRect(&rc);
	if (rc.Size() != m_szBase)
		ApplyScale(rc.Width(), rc.Height());
	return 0;
}

// Captures control positions now instead of waiting for the posted message.
void CDockingPage::EnsureLayoutCaptured()
{
	if (!m_bScaleReady && GetSafeHwnd())
		SendMessage(WM_APP + 95);
}

void CDockingPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	if (m_bScaleReady && cx > 0 && cy > 0)
		ApplyScale(cx, cy);
}

void CDockingPage::ApplyScale(int cx, int cy)
{
	double sx = (double) cx / m_szBase.cx;
	double sy = (double) cy / m_szBase.cy;

	// Scale the font with the page, clamped to stay readable.
	double fs = min(sx, sy);
	if (fs < 0.75) fs = 0.75;
	if (fs > 1.5) fs = 1.5;
	int iPct = (int) (fs * 100 + 0.5);
	bool bFontChanged = (iPct != m_iFontPct) && m_lfBase.lfHeight != 0;
	if (bFontChanged)
	{
		m_fontScaled.DeleteObject();
		LOGFONT lf = m_lfBase;
		lf.lfHeight = MulDiv(m_lfBase.lfHeight, iPct, 100);
		m_fontScaled.CreateFontIndirect(&lf);
		m_iFontPct = iPct;
	}
	HFONT hFont = (HFONT) m_fontScaled.GetSafeHandle();
	if (iPct == 100 && GetFont())
		hFont = (HFONT) GetFont()->GetSafeHandle();

	HDWP hdwp = BeginDeferWindowPos((int) m_aScale.GetSize());
	for (int i = 0; i < m_aScale.GetSize(); i++)
	{
		const CScaleItem & item = m_aScale[i];
		if (!::IsWindow(item.hWnd))
			continue;
		if (bFontChanged && item.bBaseFont && hFont)
			::SendMessage(item.hWnd, WM_SETFONT, (WPARAM) hFont, FALSE);
		int x = (int) (item.rc.left * sx + 0.5);
		int y = (int) (item.rc.top * sy + 0.5);
		int w = (int) (item.rc.Width() * sx + 0.5);
		int h = (int) (item.rc.Height() * sy + 0.5);
		if (hdwp)
			hdwp = DeferWindowPos(hdwp, item.hWnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	if (hdwp)
		EndDeferWindowPos(hdwp);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}