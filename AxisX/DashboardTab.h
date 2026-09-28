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

#if !defined(AFX_DASHBOARDTAB_H__INCLUDED_)
#define AFX_DASHBOARDTAB_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DockingPage.h"
#include "RemoteConsoleDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CDashboardTab dialog

class CDashboardTab : public CDockingPage
{
	DECLARE_DYNCREATE(CDashboardTab)

// Construction
public:
	CDashboardTab();
	virtual ~CDashboardTab();

	// Appends a log line (called by CAxisLog::Add()). iFormat is the severity:
	// 0=info, 1=warning, 2=notice, shown as a colored tag after the line.
	void AddLogLine(CString csLine, COLORREF color, int iFormat);
	void RefreshStats();

// Dialog Data
	enum { IDD = IDD_DASHBOARD_TAB };
	CRichEditCtrl m_ceLog;
	CListCtrl m_ctlQuick;
	CStatic m_ceConnStatus;
	CListCtrl m_ctlRecent;		// Recently created items/NPCs
	CEdit m_ceQaLabel;			// Custom quick action
	CEdit m_ceQaCmd;
	CStringArray m_aQaLabel;
	CStringArray m_aQaCmd;
	bool m_bClientFound;

	// Remote console window; recreated on every connect so it never points
	// to an already-deleted CRemoteConsole.
	CRemoteConsoleDlg m_rcDlg;
	bool m_bRemoteConsoleCreated;

// Overrides
	virtual BOOL OnSetActive();

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStartClient();
	afx_msg void OnToggleConnect();
	afx_msg void OnQuickActivate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnQuickCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRecentActivate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnQuickAdd();
	afx_msg void OnQuickDel();
	afx_msg void OnDonate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void LoadQuickActions();
	void SaveQuickActions();
	void FillQuickList();
	void FillRecent();
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DASHBOARDTAB_H__INCLUDED_)
