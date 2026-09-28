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

// LogTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "LogTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogTab property page

IMPLEMENT_DYNCREATE(CLogTab, CDockingPage)

CLogTab::CLogTab() : CDockingPage(CLogTab::IDD,CMsg("IDS_LOGS"))
{
	//{{AFX_DATA_INIT(CLogTab)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bVisible = false;
	m_nSyncedLines = 0;
}

CLogTab::~CLogTab()
{
}

void CLogTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogTab)
	DDX_Control(pDX, IDC_FRAMELOG, m_ceLog);
	DDX_Control(pDX, IDC_LOGLEVEL, m_ccbLevel);
	DDX_Control(pDX, IDC_LOGFILTER, m_ceFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogTab, CDockingPage)
	//{{AFX_MSG_MAP(CLogTab)
	ON_CBN_SELCHANGE(IDC_LOGLEVEL, OnFilterChanged)
	ON_EN_CHANGE(IDC_LOGFILTER, OnFilterChanged)
	ON_BN_CLICKED(IDC_LOGCOPY, OnCopy)
	ON_BN_CLICKED(IDC_LOGCLEAR, OnClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogTab message handlers

BOOL CLogTab::OnInitDialog()
{
	m_dcPropertyPage = Main->m_pcppAxisLogTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CLogTab;
	Main->m_pcppAxisLogTab->m_dcCurrentPage = this;
	m_bAlwaysColor = true;

	CDockingPage::OnInitDialog();

	// Level and text filter.
	m_ccbLevel.AddString(AXT("Alles"));
	m_ccbLevel.AddString(AXT("Nur Warnungen"));
	m_ccbLevel.AddString(AXT("Nur Hinweise"));
	m_ccbLevel.SetCurSel(0);
	AxisSetCue(m_ceFilter.GetSafeHwnd(), "Text filtern ...");

	if((m_bModifyDlgStylesAndPos == true) && m_hWnd)
		Refill();

	return TRUE;
}

// Returns true if the line matches the level and text filter.
bool CLogTab::PassesFilter(const CLogArray * pLog)
{
	if (!m_ccbLevel.GetSafeHwnd())
		return true;
	int iLevel = m_ccbLevel.GetCurSel();
	if (iLevel == 1 && pLog->m_iLevel != 1)
		return false;
	if (iLevel == 2 && pLog->m_iLevel != 2)
		return false;
	if (!m_csFilter.IsEmpty())
	{
		CString csLine = pLog->m_csLine;
		csLine.MakeLower();
		if (csLine.Find(m_csFilter) == -1)
			return false;
	}
	return true;
}

void CLogTab::Refill()
{
	m_ceLog.SetRedraw(FALSE); // batch fill without a repaint per line
	m_ceLog.SetWindowText("");
	POSITION pos = Main->m_olLog.GetHeadPosition();
	long nInsertionPoint = 0;
	while (pos != NULL)
	{
		CLogArray * pLog = (CLogArray *) Main->m_olLog.GetNext(pos);
		if (!PassesFilter(pLog))
			continue;
		m_ceLog.SetSel(nInsertionPoint, -1);
		m_ceLog.SetSelectionCharFormat(pLog->m_cf);
		m_ceLog.ReplaceSel(pLog->m_csLine);
		nInsertionPoint = m_ceLog.GetWindowTextLength();
	}
	m_ceLog.SetSel(nInsertionPoint, -1);
	m_ceLog.SetRedraw(TRUE);
	m_ceLog.Invalidate();
	m_ceLog.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
	m_nSyncedLines = Main->m_olLog.GetCount();
}

void CLogTab::AddText(CString csText, COLORREF color, int iLevel)
{
	csText += AXT("\n");
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
	cf.dwEffects = 0;
	cf.yHeight = 160;
	cf.crTextColor = color;
	strcpy_s(cf.szFaceName, _T("Consolas"));

	CLogArray * pLogLine = new (CLogArray);
	pLogLine->m_csLine = csText;
	pLogLine->m_cf = cf;
	pLogLine->m_iLevel = iLevel;
	if (Main->m_olLog.IsEmpty())
		Main->m_olLog.AddHead(pLogLine);
	else
		Main->m_olLog.AddTail(pLogLine);

	if (m_ceLog)
	{
		// Append and scroll to the bottom.
		if (PassesFilter(pLogLine))
		{
			long nInsertionPoint = m_ceLog.GetWindowTextLength();
			m_ceLog.SetSel(nInsertionPoint, -1);
			m_ceLog.SetSelectionCharFormat(cf);
			m_ceLog.ReplaceSel(csText);
			m_ceLog.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
		}
		m_nSyncedLines++;
	}
}

void CLogTab::OnFilterChanged()
{
	m_ceFilter.GetWindowText(m_csFilter);
	m_csFilter.Trim();
	m_csFilter.MakeLower();
	Refill();
}

// Copies the visible lines to the clipboard.
void CLogTab::OnCopy()
{
	CString csText;
	m_ceLog.GetWindowText(csText);
	csText.Replace("\r\n", "\n");
	csText.Replace("\n", "\r\n");
	if (!OpenClipboard())
		return;
	EmptyClipboard();
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, csText.GetLength() + 1);
	if (hMem)
	{
		memcpy(GlobalLock(hMem), (LPCTSTR) csText, csText.GetLength() + 1);
		GlobalUnlock(hMem);
		SetClipboardData(CF_TEXT, hMem);
	}
	CloseClipboard();
	AxisSetStatus(AXT("Protokoll in die Zwischenablage kopiert."), 1);
}

void CLogTab::OnClear()
{
	while (!Main->m_olLog.IsEmpty())
		delete (CLogArray *) Main->m_olLog.RemoveHead();
	Refill();
}

BOOL CLogTab::OnSetActive()
{
	// Rebuild only if lines arrived while the control was not in sync.
	if ( m_nSyncedLines != Main->m_olLog.GetCount() )
		Refill();
	m_ceLog.SetOptions(ECOOP_OR,ECO_SAVESEL);
	return CDockingPage::OnSetActive();
}
