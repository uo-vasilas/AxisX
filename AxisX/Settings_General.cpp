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

// Settings_General.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "Settings_General.h"
#include "Updater.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsGeneral property page

IMPLEMENT_DYNCREATE(CSettingsGeneral, CPropertyPage)

CSettingsGeneral::CSettingsGeneral() : CPropertyPage(CSettingsGeneral::IDD)
{
	//{{AFX_DATA_INIT(CSettingsGeneral)
	m_bAllowMultiple = FALSE;
	m_bAlwaysOnTop = FALSE;
	m_bSysClose = FALSE;
	m_bLoadDefault = FALSE;
	m_bDisableToolbar = FALSE;
	m_bCheckUpdates = TRUE;
	//}}AFX_DATA_INIT
}


CSettingsGeneral::~CSettingsGeneral()
{
}


void CSettingsGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsGeneral)
	DDX_Check(pDX, IDC_ALWAYS_ONTOP, m_bAlwaysOnTop);
	DDX_Check(pDX, IDC_SYSCLOSE, m_bSysClose);
	DDX_Check(pDX, IDC_LOADDEFAULT, m_bLoadDefault);
	DDX_Check(pDX, IDC_ALLOWMULTIPLE, m_bAllowMultiple);
	DDX_Check(pDX, IDC_DISABLE_TOOLBAR, m_bDisableToolbar);
	DDX_Check(pDX, IDC_CHECKUPDATES, m_bCheckUpdates);
	DDX_Control(pDX, IDC_STARTTAB, m_ccbStartTab);
	DDX_Control(pDX, IDC_LANGUAGE, m_ccbLanguage);
	DDX_Control(pDX, IDC_THEME, m_ccbTheme);
	DDX_Control(pDX, IDC_RESETSTING, m_cbResetSettings);
	DDX_Control(pDX, IDC_PREFIX, m_ceCommandPrefix);
	DDX_Control(pDX, IDC_UOTITLE, m_ceUOTitle);
	DDX_Control(pDX, IDC_CUSTOMLOGO, m_ceCustomLogo);
	DDX_Control(pDX, IDC_CUSTOMICON, m_ceCustomIcon);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(CSettingsGeneral)
	ON_BN_CLICKED(IDC_ALLOWMULTIPLE, OnAllowMultiple)
	ON_BN_CLICKED(IDC_ALWAYS_ONTOP, OnAlwaysOnTop)
	ON_BN_CLICKED(IDC_SYSCLOSE, OnSysClose)
	ON_BN_CLICKED(IDC_LOADDEFAULT, OnLoadDefault)
	ON_CBN_SELCHANGE(IDC_STARTTAB, OnSelchangeStartTab)
	ON_CBN_SELCHANGE(IDC_LANGUAGE, OnSelchangeLanguage)
	ON_CBN_SELCHANGE(IDC_THEME, OnSelchangeTheme)
	ON_BN_CLICKED(IDC_RESETSTING, OnResetSettings)
	ON_BN_CLICKED(IDC_RESET_GENERAL, OnResetTab)
	ON_EN_CHANGE(IDC_PREFIX, OnChangePrefix)
	ON_BN_CLICKED(IDC_DISABLE_TOOLBAR, OnDisableToolbar)
	ON_EN_CHANGE(IDC_UOTITLE, OnChangeUOTitle)
	ON_EN_KILLFOCUS(IDC_CUSTOMLOGO, OnKillfocusCustomLogo)
	ON_EN_KILLFOCUS(IDC_CUSTOMICON, OnKillfocusCustomIcon)
	ON_BN_CLICKED(IDC_CUSTOMLOGO_BROWSE, OnBrowseCustomLogo)
	ON_BN_CLICKED(IDC_CUSTOMICON_BROWSE, OnBrowseCustomIcon)
	ON_BN_CLICKED(IDC_CHECKUPDATES, OnCheckUpdates)
	ON_BN_CLICKED(IDC_CHECKUPDATES_NOW, OnCheckUpdatesNow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsGeneral message handlers

BOOL CSettingsGeneral::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	FillStartTabs();
	// Language names are shown in their own language.
	m_ccbLanguage.AddString("Deutsch");
	m_ccbLanguage.AddString("English");
	m_ccbLanguage.SetCurSel(Main->GetRegistryString("Language", "eng") == "deu" ? 0 : 1);
	// Theme: dark/light
	m_ccbTheme.AddString(AXT("Dunkel"));
	m_ccbTheme.AddString(AXT("Hell"));
	m_ccbTheme.SetCurSel(AxisLightTheme() ? 1 : 0);
	m_bAlwaysOnTop = (BOOL) Main->m_dwAlwaysOnTop;
	m_bSysClose = (BOOL) Main->m_dwSysClose;
	m_bAllowMultiple = (BOOL) Main->m_dwAllowMultiple;
	m_bLoadDefault = (BOOL) Main->m_dwLoadDefault;
	SelectStartTab(Main->m_dwStartTab);
	m_ceCommandPrefix.SetWindowText(Main->m_csCommandPrefix);
	m_bDisableToolbar = (BOOL) Main->m_dwDisableToolbar;
	m_ceUOTitle.SetWindowText(Main->m_csUOTitle);
	m_ceCustomLogo.SetWindowText(Main->m_csCustomLogo);
	m_ceCustomIcon.SetWindowText(Main->m_csCustomIcon);
	m_bCheckUpdates = Main->GetRegistryDword("CheckUpdates", 1) ? TRUE : FALSE;

	UpdateData(false);

	return TRUE;
}

void CSettingsGeneral::OnCheckUpdates()
{
	UpdateData();
	Main->PutRegistryDword("CheckUpdates", m_bCheckUpdates);
}

// The result shows up in the status line and, if there is an update, on the overview.
void CSettingsGeneral::OnCheckUpdatesNow()
{
	if (Main->m_pMainWnd == NULL)
		return;
	AxisSetStatus(AXT("Suche nach Updates ..."), 0);
	AxisStartUpdateCheck(Main->m_pMainWnd->GetSafeHwnd(), true);
}

// Stores a branding value (file path or URL) and applies it immediately.
static void ApplyBrandingSetting(LPCTSTR pszRegValue, CString & csTarget, const CString & csNewValue)
{
	if ( csNewValue == csTarget )
		return;
	Main->PutRegistryString(pszRegValue, csNewValue);
	csTarget = csNewValue;
	CAxisXDlg * pMain = DYNAMIC_DOWNCAST(CAxisXDlg, Main->m_pMainWnd);
	if ( pMain != NULL )
		pMain->ApplyCustomBranding();
}

void CSettingsGeneral::OnKillfocusCustomLogo()
{
	CString csValue;
	m_ceCustomLogo.GetWindowText(csValue);
	ApplyBrandingSetting(_T("Custom Logo"), Main->m_csCustomLogo, csValue);
}

void CSettingsGeneral::OnKillfocusCustomIcon()
{
	CString csValue;
	m_ceCustomIcon.GetWindowText(csValue);
	ApplyBrandingSetting(_T("Custom Icon"), Main->m_csCustomIcon, csValue);
}

void CSettingsGeneral::OnBrowseCustomLogo()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		"Images (*.png;*.bmp;*.jpg;*.jpeg;*.gif;*.ico)|*.png;*.bmp;*.jpg;*.jpeg;*.gif;*.ico|All Files (*.*)|*.*||", this);
	if ( dlg.DoModal() != IDOK )
		return;
	m_ceCustomLogo.SetWindowText(dlg.GetPathName());
	ApplyBrandingSetting(_T("Custom Logo"), Main->m_csCustomLogo, dlg.GetPathName());
}

void CSettingsGeneral::OnBrowseCustomIcon()
{
	CFileDialog dlg(TRUE, "ico", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		"Icons (*.ico)|*.ico|All Files (*.*)|*.*||", this);
	if ( dlg.DoModal() != IDOK )
		return;
	m_ceCustomIcon.SetWindowText(dlg.GetPathName());
	ApplyBrandingSetting(_T("Custom Icon"), Main->m_csCustomIcon, dlg.GetPathName());
}

void CSettingsGeneral::OnAllowMultiple()
{
	UpdateData();
	Main->PutRegistryDword("AllowMultipleInstances", m_bAllowMultiple);
	Main->m_dwAllowMultiple = m_bAllowMultiple;
}

void CSettingsGeneral::OnAlwaysOnTop()
{
	UpdateData();
	Main->PutRegistryDword("AlwaysOnTop", m_bAlwaysOnTop);
	Main->m_dwAlwaysOnTop = m_bAlwaysOnTop;

	Main->m_pMainWnd->SetWindowPos(Main->m_dwAlwaysOnTop ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CSettingsGeneral::OnSysClose()
{
	UpdateData();
	Main->PutRegistryDword("SysClose", m_bSysClose);
	Main->m_dwSysClose = m_bSysClose;
}

void CSettingsGeneral::OnLoadDefault()
{
	UpdateData();
	Main->PutRegistryDword("LoadDefault", m_bLoadDefault);
	Main->m_dwLoadDefault = m_bLoadDefault;
}

void CSettingsGeneral::OnSelchangeStartTab() 
{
	UpdateData();
	int iSel = m_ccbStartTab.GetCurSel();
	if (iSel != CB_ERR)
		iSel = (int) m_ccbStartTab.GetItemData(iSel);	// Item data holds the stored page index
	Main->PutRegistryDword("StartTab", iSel);
	Main->m_dwStartTab = iSel;
}

void CSettingsGeneral::OnDisableToolbar()
{
	UpdateData();
	Main->PutRegistryDword("DisableToolbar", m_bDisableToolbar);
	Main->m_dwDisableToolbar = m_bDisableToolbar;
}

void CSettingsGeneral::OnChangePrefix()
{
	CString csPrefix;
	m_ceCommandPrefix.GetWindowText(csPrefix);
	Main->PutRegistryString("CommandPrefix", csPrefix);
	Main->m_csCommandPrefix = csPrefix;
}

void CSettingsGeneral::OnChangeUOTitle()
{
	CString csTitle;
	m_ceUOTitle.GetWindowText(csTitle);
	Main->PutRegistryString("UOTitle", csTitle);
	Main->m_csUOTitle = csTitle;
}

void CSettingsGeneral::OnResetTab()
{
	if ( AfxMessageBox( "Are you sure you want to reset the General settings to default?", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL )
		return;

	Main->LoadIni(0,"AlwaysOnTop");
	m_bAlwaysOnTop = (BOOL) Main->m_dwAlwaysOnTop;
	Main->PutRegistryDword("AlwaysOnTop", m_bAlwaysOnTop);
	Main->LoadIni(0,"DisableToolbar");
	m_bDisableToolbar = (BOOL) Main->m_dwDisableToolbar;
	Main->PutRegistryDword("DisableToolbar", m_bDisableToolbar);
	Main->LoadIni(0,"SysClose");
	m_bSysClose = (BOOL) Main->m_dwSysClose;
	Main->PutRegistryDword("SysClose", m_bSysClose);
	Main->LoadIni(0,"AllowMultiple");
	m_bAllowMultiple = (BOOL) Main->m_dwAllowMultiple;
	Main->PutRegistryDword("AllowMultipleInstances", m_bAllowMultiple);
	Main->LoadIni(0,"LoadDefault");
	m_bLoadDefault = (BOOL) Main->m_dwLoadDefault;
	Main->PutRegistryDword("LoadDefault", m_bLoadDefault);
	Main->LoadIni(0,"StartTab");
	SelectStartTab(Main->m_dwStartTab);
	Main->PutRegistryDword("StartTab", Main->m_dwStartTab);
	Main->LoadIni(0,"CommandPrefix");
	m_ceCommandPrefix.SetWindowText(Main->m_csCommandPrefix);
	Main->PutRegistryString("CommandPrefix", Main->m_csCommandPrefix);
	Main->LoadIni(0,"UOTitle");
	m_ceUOTitle.SetWindowText(Main->m_csUOTitle);
	Main->PutRegistryString("UOTitle", Main->m_csUOTitle);
	m_bCheckUpdates = TRUE;
	Main->PutRegistryDword("CheckUpdates", 1);

	UpdateData(false);
}

void CSettingsGeneral::OnResetSettings()
{
	if ( AfxMessageBox( "Are you sure you want to reset all settings to default?", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL )
		return;

	//Only load default settings from Axis.ini
	Main->LoadIni(0);

	//General Tab
	m_bAlwaysOnTop = (BOOL) Main->m_dwAlwaysOnTop;
	Main->PutRegistryDword("AlwaysOnTop", m_bAlwaysOnTop);
	m_bDisableToolbar = (BOOL) Main->m_dwDisableToolbar;
	Main->PutRegistryDword("DisableToolbar", m_bDisableToolbar);
	m_bSysClose = (BOOL) Main->m_dwSysClose;
	Main->PutRegistryDword("SysClose", m_bSysClose);
	m_bAllowMultiple = (BOOL) Main->m_dwAllowMultiple;
	Main->PutRegistryDword("AllowMultipleInstances", m_bAllowMultiple);
	m_bLoadDefault = (BOOL) Main->m_dwLoadDefault;
	Main->PutRegistryDword("LoadDefault", m_bLoadDefault);
	SelectStartTab(Main->m_dwStartTab);
	Main->PutRegistryDword("StartTab", Main->m_dwStartTab);
	m_ceCommandPrefix.SetWindowText(Main->m_csCommandPrefix);
	Main->PutRegistryString("CommandPrefix", Main->m_csCommandPrefix);
	m_ceUOTitle.SetWindowText(Main->m_csUOTitle);
	Main->PutRegistryString("UOTitle", Main->m_csUOTitle);
	m_bCheckUpdates = TRUE;
	Main->PutRegistryDword("CheckUpdates", 1);

	//Item Tab
	Main->m_pcppSetItem->m_bRoomView = (BOOL) Main->m_dwRoomView;
	Main->PutRegistryDword("RoomView", Main->m_dwRoomView);
	Main->m_pcppSetItem->m_bShowItems = (BOOL) Main->m_dwShowItems;
	Main->PutRegistryDword("ShowItems", Main->m_dwShowItems);
	Main->m_pcppSetItem->m_cbRoomView.EnableWindow(Main->m_dwShowItems? true : false);
	Main->m_pcppSetItem->m_dwItemBGColor = Main->m_dwItemBGColor;
	Main->PutRegistryDword("ItemBGColor", Main->m_dwItemBGColor);

	//Travel Tab
	Main->m_pcppSetTravel->m_bDrawStatics = (BOOL) Main->m_dwDrawStatics;
	Main->PutRegistryDword("DrawStatics", Main->m_dwDrawStatics);
	Main->m_pcppSetTravel->m_bDrawDifs = (BOOL) Main->m_dwDrawDifs;
	Main->PutRegistryDword("DrawDifs", Main->m_dwDrawDifs);
	Main->m_pcppSetTravel->m_bShowSpawnpoints = (BOOL) Main->m_dwShowSpawnpoints;
	Main->PutRegistryDword("ShowSpawnpoints", Main->m_dwShowSpawnpoints);
	Main->m_pcppSetTravel->m_bShowMap = (BOOL) Main->m_dwShowMap;
	Main->PutRegistryDword("ShowMap", Main->m_dwShowMap);
	Main->m_pcppSetTravel->m_dwNPCSpawnColor = Main->m_dwNPCSpawnColor;
	Main->PutRegistryDword("NPCSpawnColor", Main->m_dwNPCSpawnColor);
	Main->m_pcppSetTravel->m_dwItemSpawnColor = Main->m_dwItemSpawnColor;
	Main->PutRegistryDword("ItemSpawnColor", Main->m_dwItemSpawnColor);
	
	Main->m_pcppSetTravel->m_csItemColor.EnableWindow(Main->m_dwShowSpawnpoints && Main->m_dwShowMap? true : false);
	Main->m_pcppSetTravel->m_csNpcColor.EnableWindow(Main->m_dwShowSpawnpoints && Main->m_dwShowMap? true : false);
	Main->m_pcppSetTravel->m_stNPCCol.EnableWindow(Main->m_dwShowSpawnpoints && Main->m_dwShowMap? true : false);
	Main->m_pcppSetTravel->m_stItemCol.EnableWindow(Main->m_dwShowSpawnpoints && Main->m_dwShowMap? true : false);
	Main->m_pcppSetTravel->m_cbDrawStatics.EnableWindow(Main->m_dwShowMap? true : false);
	Main->m_pcppSetTravel->m_cbDrawDifs.EnableWindow(Main->m_dwShowMap? true : false);
	Main->m_pcppSetTravel->m_cbShowSpawnpoints.EnableWindow(Main->m_dwShowMap? true : false);

	//Spawn Tab
	Main->m_pcppSetSpawn->m_bShowNPCs = (BOOL) Main->m_dwShowNPCs;
	Main->PutRegistryDword("ShowNPCs", Main->m_dwShowNPCs);
	Main->m_pcppSetSpawn->m_dwSpawnBGColor = Main->m_dwSpawnBGColor;
	Main->PutRegistryDword("SpawnBGColor", Main->m_dwSpawnBGColor);

	//Override Paths Tab
	for(int i = 0; i < VERFILE_QTY; i++)
	{
		CString csPath;
		csPath.Format(_T("%s%s"), csMulPath, GetMulFileName(i));
		if(Main->m_pcppSetOverridePaths->m_clcPathList)
			Main->m_pcppSetOverridePaths->m_clcPathList.SetItemText(i, 1, csPath);
		Main->SetMulPath(i, csPath);
		Main->DeleteRegistryValue(GetMulFileName(i), REGKEY_OVERRIDEPATH, hRegLocation);
	}
	Main->m_pcppSetOverridePaths->m_cbResetPath.EnableWindow(false);

	//Paths Tab
	Main->m_pcppSetPaths->m_bSameAsClient = (BOOL) Main->m_dwSameAsClient;
	Main->PutRegistryDword("SameAsClient", Main->m_dwSameAsClient);
	CString csDefUOPath = Main->GetRegistryString("ExePath", "", HKEY_LOCAL_MACHINE, "SOFTWARE\\Origin Worlds Online\\Ultima Online\\1.0");
	if ( csDefUOPath != "" )
	{
		csUOPath = csDefUOPath;
		Main->PutRegistryString("Default Client", csUOPath);
		if (Main->m_pcppSetPaths->m_csDefaultClient)
			Main->m_pcppSetPaths->m_csDefaultClient.SetWindowText(csUOPath);

		if (Main->m_dwSameAsClient)
		{
			csMulPath = csUOPath;
			csMulPath = csUOPath.Left(csUOPath.ReverseFind('\\')+1);
			Main->PutRegistryString("Default MulPath", csMulPath);
			if (Main->m_pcppSetPaths->m_csDefaultMulPath)
				Main->m_pcppSetPaths->m_csDefaultMulPath.SetWindowText(csMulPath);
			Main->InitializeMulPaths();
		}
	}
	Main->m_pcppSetPaths->m_cbMulBrowse.EnableWindow(Main->m_dwSameAsClient ? false : true);
	Main->m_pcppSetPaths->m_csDefaultMulPath.EnableWindow(Main->m_dwSameAsClient ? false : true);

	UpdateData(false);
}

// Fills the start page list in sidebar order. The stored value is the page index
// used by CAxisXApp::InitInstance.
void CSettingsGeneral::FillStartTabs()
{
	static const struct { LPCTSTR pszName; int iIndex; } aTabs[] =
	{
		{ _T("\xDC" "bersicht"), 12 }, { _T("Items"), 4 }, { _T("Item bearbeiten"), 5 }, { _T("Spawns"), 2 },
		{ _T("Reisen"), 1 }, { _T("Charakter bearbeiten"), 3 }, { _T("Konto"), 6 }, { _T("GM-Befehle"), 0 },
		{ _T("Eigene Befehle"), 9 }, { _T("Zauber & Kl\xE4nge"), 7 }, { _T("Launcher"), 8 }, { _T("Protokoll"), 11 },
	};
	m_ccbStartTab.ResetContent();
	for (int i = 0; i < (int)(sizeof(aTabs) / sizeof(aTabs[0])); i++)
	{
		int iItem = m_ccbStartTab.AddString(AxisTr(aTabs[i].pszName));
		m_ccbStartTab.SetItemData(iItem, aTabs[i].iIndex);
	}
}

void CSettingsGeneral::SelectStartTab(DWORD dwIndex)
{
	m_ccbStartTab.SetCurSel(0);
	for (int i = 0; i < m_ccbStartTab.GetCount(); i++)
		if ((DWORD) m_ccbStartTab.GetItemData(i) == dwIndex)
			m_ccbStartTab.SetCurSel(i);
}

void CSettingsGeneral::OnSelchangeLanguage()
{
	int iSel = m_ccbLanguage.GetCurSel();
	if (iSel == CB_ERR)
		return;
	Main->PutRegistryString("Language", iSel == 0 ? "deu" : "eng");
	AfxMessageBox(iSel == 0 ? "Sprache ge\xE4ndert - wird beim n\xE4" "chsten Start von Axis \xFC" "bernommen."
		: "Language changed - takes effect the next time Axis starts.", MB_OK | MB_ICONINFORMATION);
}

void CSettingsGeneral::OnSelchangeTheme()
{
	int iSel = m_ccbTheme.GetCurSel();
	if (iSel == CB_ERR)
		return;
	Main->PutRegistryString("Theme", iSel == 1 ? "light" : "dark");
	AfxMessageBox(AXT("Design ge\xE4ndert - wird beim n\xE4" "chsten Start von Axis \xFC" "bernommen."), MB_OK | MB_ICONINFORMATION);
}