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

#if !defined(AFX_AXISXDLG_H__6A6E37E0_EB0F_4888_AB33_38E7B560103B__INCLUDED_)
#define AFX_AXISXDLG_H__6A6E37E0_EB0F_4888_AB33_38E7B560103B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AxisXDlg.h : header file
//

BOOL CALLBACK HideButtonsProc(HWND hWnd, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// CAxisXDlg

class CAxisXDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CAxisXDlg)

// Construction
public:
	CAxisXDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CAxisXDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAxisXDlg)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdateTip();
	NOTIFYICONDATA m_nid;
	 CAxisXDlg();
	HICON m_hIcon;
	virtual ~CAxisXDlg();

	// Sidebar navigation used in place of the property-sheet tab strip.
	CListCtrl m_ctlNav;
	CFont m_fntNavGroup;
	CFont m_fntNavItem;
	CImageList m_ilNavRowHeight;
	int m_iNavRowHeight;	// Current sidebar row height (24..34, fitted to the window height)
	void FitSidebarRows();
	bool m_bSyncingNav; // guards OnNavSelChanged() while SyncSidebarSelection() runs
	CImage m_imgBrandLogo; // optional custom logo for the sidebar brand area
	void ApplyCustomBranding();
	void BuildSidebar();
	void RebuildSidebarItems();
	void RepositionLayout();
	void LayoutActivePage();	// Sizes the active page to fill the resizable window
	bool m_bLayoutSized;
	bool m_bFirstRunSettings;	// open the settings once the window exists (first start)
	CSize m_szNatural;
	void SyncSidebarSelection();
	void RestoreFromMiniBar(CPropertyPage* pPage);	// Restores the window from the mini bar or tray
	void ShowPage(int iIndex);	// Switches pages without intermediate repaints
	void ShowPage(CPropertyPage* pPage) { ShowPage(GetPageIndex(pPage)); }

	// Footer icon buttons under the nav list; only the visible entries of
	// g_axisFooterActions, compacted by BuildSidebar().
	int m_iFooterHover;
	int m_nFooterVisible;
	int m_aFooterAction[8];
	int m_aFooterIcon[8];
	LPCTSTR m_aFooterTip[8];
	int FooterHitTest(CPoint point);
	CRect GetFooterSlotRect(const CRect& rcClient, int iSlot);
	CToolTipCtrl m_tipFooter;
	void UpdateFooterTooltips();

	// Status line below the page: last sent command or error message.
	CString m_csStatus;
	COLORREF m_crStatus;
	CRect GetStatusRect();
	void SetStatus(LPCTSTR pszText, COLORREF crText);

	// Generated message map functions
protected:
	bool m_bInit;
	//{{AFX_MSG(CAxisXDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNavSelChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNavSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnNavResync(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCheckLabels(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCheckPopupLabels(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPressPageButton(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNavCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSettingsGeneral();
	afx_msg void OnSettingsPaths();
	afx_msg void OnSettingsItem();
	afx_msg void OnSettingsTravel();
	afx_msg void OnSettingsSpawn();
	afx_msg void OnSettingsOverridePaths();
	void ShowProfileMenu();
	afx_msg void OnOpenProfileOption();
	afx_msg void OnUnloadProfile();
	afx_msg void OnLoadDefProfile();
	afx_msg void OnLoadLastProfile();
	afx_msg void OnHelp();
	afx_msg void OnAboutDlg();
	afx_msg void OnClose();
	//afx_msg void OnReadAnim();

public:
	afx_msg void OnOpenSettingsPage(int iPage);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AXISXDLG_H__6A6E37E0_EB0F_4888_AB33_38E7B560103B__INCLUDED_)
