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

// LauncherTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "LauncherTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemTab property page

IMPLEMENT_DYNCREATE(CLauncherTab, CDockingPage)

CLauncherTab::CLauncherTab() : CDockingPage(CLauncherTab::IDD,CMsg("IDS_LAUNCHER"))
{
	//{{AFX_DATA_INIT(CLauncherTab)
	//}}AFX_DATA_INIT
	m_iCurAccountSel = -1;
}

CLauncherTab::~CLauncherTab()
{
}

void CLauncherTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLauncherTab)
	DDX_Control(pDX, IDC_ACCOUNT_LIST, m_clbAccountList);
	DDX_Control(pDX, IDC_NAME, m_ceName);
	DDX_Control(pDX, IDC_ACCOUNT, m_ceAccount);
	DDX_Control(pDX, IDC_PASSWORD, m_cePassword);
	DDX_Control(pDX, IDC_IPADDRESS, m_ceIPAddress);
	DDX_Control(pDX, IDC_PORT, m_cePort);
	DDX_Control(pDX, IDC_ALTCLIENT, m_ceAltClient);
	DDX_Control(pDX, IDC_DONE, m_cbDone);
	DDX_Control(pDX, IDC_CANCEL, m_cbCancel);
	DDX_Control(pDX, IDC_CLIENTBROWSE, m_cbBrowse);
	DDX_Control(pDX, IDC_EDITACCT, m_cbEdit);
	DDX_Control(pDX, IDC_NEW, m_cbNew);
	DDX_Control(pDX, IDC_DELETE, m_cbDelete);
	DDX_Control(pDX, IDC_LAUNCH2D, m_cbLaunch2D);
	DDX_Control(pDX, IDC_LAUNCH3D, m_cbLaunch3D);
	DDX_Control(pDX, IDC_LAUNCHALT, m_cbLaunchAlt);
	DDX_Control(pDX, IDC_LAUNCHAUTO, m_cbLaunchAuto);
	DDX_Control(pDX, IDC_STATICLNAME, m_stName);
	DDX_Control(pDX, IDC_STATICLACCT, m_stAcct);
	DDX_Control(pDX, IDC_STATICLPASS, m_stPass);
	DDX_Control(pDX, IDC_STATICLIP, m_stIP);
	DDX_Control(pDX, IDC_STATICLPORT, m_stPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLauncherTab, CDockingPage)
	//{{AFX_MSG_MAP(CLauncherTab)
	ON_LBN_SELCHANGE(IDC_ACCOUNT_LIST, OnSelchangeAccountList)
	ON_BN_CLICKED(IDC_DONE, OnSave)
	ON_BN_CLICKED(IDC_CANCEL, OnCancelAccount)
	ON_BN_CLICKED(IDC_CLIENTBROWSE, OnClientBrowse)
	ON_BN_CLICKED(IDC_EDITACCT, OnEdit)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_LAUNCH2D, OnLaunch2D)
	ON_BN_CLICKED(IDC_LAUNCH3D, OnLaunch3D)
	ON_BN_CLICKED(IDC_LAUNCHALT, OnLaunchAlt)
	ON_BN_CLICKED(IDC_LAUNCHAUTO, OnLaunchAuto)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLauncherTab message handlers

BOOL CLauncherTab::OnInitDialog() 
{
	m_dcPropertyPage = Main->m_pcppLauncherTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CLauncherTab;
	Main->m_pcppLauncherTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();
	AxisMarkPrimary(this, IDC_LAUNCH2D);

	HKEY hKey;
	LONG lStatus;
	lStatus = RegOpenKeyEx(hRegLocation, REGKEY_ACCOUNT, 0, KEY_ALL_ACCESS, &hKey);
	int iIndex = 0;
	while (lStatus == ERROR_SUCCESS)
	{
		char szBuffer[MAX_PATH];
		DWORD dwSize = sizeof(szBuffer);
		lStatus = RegEnumKeyEx(hKey, iIndex, &szBuffer[0], &dwSize, 0, NULL, NULL, NULL);
		if (lStatus == ERROR_SUCCESS)
		{
			this->m_clbAccountList.AddString(szBuffer);
			m_clbAccountList.SelectString(-1,szBuffer);
			m_iCurAccountSel = m_clbAccountList.GetCurSel();
			OnSelchangeAccountList();
		}
		iIndex++;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLauncherTab::OnSelchangeAccountList()
{
	m_iCurAccountSel = m_clbAccountList.GetCurSel();

	m_cbNew.EnableWindow(true);

	if (m_iCurAccountSel == -1)
		return;

	m_cbDelete.EnableWindow(true);
	m_cbEdit.EnableWindow(true);
	m_cbLaunch2D.EnableWindow(true);
	m_cbLaunch3D.EnableWindow(true);
	m_cbLaunchAlt.EnableWindow(true);

	m_ceName.EnableWindow(false);
	m_ceAccount.EnableWindow(false);
	m_cePassword.EnableWindow(false);
	m_ceIPAddress.EnableWindow(false);
	m_cePort.EnableWindow(false);
	m_ceAltClient.EnableWindow(false);

	m_stName.EnableWindow(false);
	m_stAcct.EnableWindow(false);
	m_stPass.EnableWindow(false);
	m_stIP.EnableWindow(false);
	m_stPort.EnableWindow(false);

	m_cbCancel.EnableWindow(false);
	m_cbDone.EnableWindow(false);
	m_cbBrowse.EnableWindow(false);
	
	CString csAccount;
	m_clbAccountList.GetText(m_iCurAccountSel, csAccount);
	m_ceName.SetWindowText(csAccount);

	CString csKey;
	csKey.Format("%s\\%s",REGKEY_ACCOUNT, csAccount);
	CString csAcct = Main->GetRegistryString("Account", "", hRegLocation, csKey);
	CString csPass = Main->GetRegistryString("Password", "", hRegLocation, csKey);
	csPass = Encrypt(csPass);
	CString csIP = Main->GetRegistryString("IP", "", hRegLocation, csKey);
	CString csPort = Main->GetRegistryString("Port", "", hRegLocation, csKey);
	CString csAlt = Main->GetRegistryString("Alternate Client", "", hRegLocation, csKey);
	m_ceAccount.SetWindowText(csAcct);
	m_cePassword.SetWindowText(csPass);
	m_ceIPAddress.SetWindowText(csIP);
	m_cePort.SetWindowText(csPort);
	m_ceAltClient.SetWindowText(csAlt);
}

void CLauncherTab::OnSave()
{
	CString csKey, m_csName, m_csAccount, m_csPass, m_csIP, m_csPort, m_csAlt, csError;
	m_ceName.GetWindowText(m_csName);
	m_ceAccount.GetWindowText(m_csAccount);
	m_cePassword.GetWindowText(m_csPass);
	m_csPass = Encrypt(m_csPass);
	m_ceIPAddress.GetWindowText(m_csIP);
	m_cePort.GetWindowText(m_csPort);
	m_ceAltClient.GetWindowText(m_csAlt);
	if(m_csName == "")
	{
		AxisSetStatus(AXT("Bitte einen Namen eintragen."), 2);
		return;
	}
	if(m_csIP == "")
	{
		AxisSetStatus(AXT("Bitte die Server-Adresse eintragen."), 2);
		return;
	}
	if(m_csPort == "")
	{
		AxisSetStatus(AXT("Bitte den Port eintragen."), 2);
		return;
	}

	csKey.Format("%s\\%s",REGKEY_ACCOUNT, m_csName);
	HKEY hKey;

	if (m_ceName.IsWindowEnabled())
	{
		if( RegOpenKeyEx(hRegLocation, csKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
		{
		//This Account already exists, stop here.
			csError.Format(AXT("Den Zugang \"%s\" gibt es schon.") , m_csName);
			AfxMessageBox(csError, MB_ICONSTOP);
			RegCloseKey(hKey);
			return;
		}
		
		//Highlight new profile
		m_clbAccountList.AddString(m_csName);
		m_clbAccountList.SelectString(-1,m_csName);
		m_iCurAccountSel = m_clbAccountList.GetCurSel();
	}

	Main->PutRegistryString("Account", m_csAccount, hRegLocation, csKey);
	Main->PutRegistryString("Password", m_csPass, hRegLocation, csKey);
	Main->PutRegistryString("IP", m_csIP, hRegLocation, csKey);
	Main->PutRegistryString("Port", m_csPort, hRegLocation, csKey);
	Main->PutRegistryString("Alternate Client", m_csAlt, hRegLocation, csKey);

	//reset windows
	m_cbNew.EnableWindow(true);
	m_cbEdit.EnableWindow(true);
	m_cbDelete.EnableWindow(true);
	m_cbLaunch2D.EnableWindow(true);
	m_cbLaunch3D.EnableWindow(true);
	m_cbLaunchAlt.EnableWindow(true);

	m_ceName.EnableWindow(false);
	m_ceAccount.EnableWindow(false);
	m_cePassword.EnableWindow(false);
	m_ceIPAddress.EnableWindow(false);
	m_cePort.EnableWindow(false);
	m_ceAltClient.EnableWindow(false);

	m_stName.EnableWindow(false);
	m_stAcct.EnableWindow(false);
	m_stPass.EnableWindow(false);
	m_stIP.EnableWindow(false);
	m_stPort.EnableWindow(false);

	m_cbCancel.EnableWindow(false);
	m_cbDone.EnableWindow(false);
	m_cbBrowse.EnableWindow(false);
	this->SetFocus();

}

void CLauncherTab::OnCancelAccount()
{
	m_cbNew.EnableWindow(true);

	m_ceName.EnableWindow(false);
	m_ceAccount.EnableWindow(false);
	m_cePassword.EnableWindow(false);
	m_ceIPAddress.EnableWindow(false);
	m_cePort.EnableWindow(false);
	m_ceAltClient.EnableWindow(false);

	m_stName.EnableWindow(false);
	m_stAcct.EnableWindow(false);
	m_stPass.EnableWindow(false);
	m_stIP.EnableWindow(false);
	m_stPort.EnableWindow(false);

	m_cbCancel.EnableWindow(false);
	m_cbDone.EnableWindow(false);
	m_cbBrowse.EnableWindow(false);

	if (m_iCurAccountSel != -1 )
	{
	m_cbEdit.EnableWindow(true);
	m_cbDelete.EnableWindow(true);
	m_cbLaunch2D.EnableWindow(true);
	m_cbLaunch3D.EnableWindow(true);
	m_cbLaunchAlt.EnableWindow(true);
	}
	this->SetFocus();
}

void CLauncherTab::OnEdit()
{
	if ( m_iCurAccountSel == -1 )
		return;

	m_ceAccount.EnableWindow(true);
	m_cePassword.EnableWindow(true);
	m_ceIPAddress.EnableWindow(true);
	m_cePort.EnableWindow(true);
	m_ceAltClient.EnableWindow(true);

	m_stAcct.EnableWindow(true);
	m_stPass.EnableWindow(true);
	m_stIP.EnableWindow(true);
	m_stPort.EnableWindow(true);

	m_cbCancel.EnableWindow(true);
	m_cbDone.EnableWindow(true);
	m_cbBrowse.EnableWindow(true);

	m_stName.EnableWindow(false);
	m_ceName.EnableWindow(false);
	m_cbNew.EnableWindow(false);
	m_cbEdit.EnableWindow(false);
	m_cbDelete.EnableWindow(false);
	m_cbLaunch2D.EnableWindow(false);
	m_cbLaunch3D.EnableWindow(false);
	m_cbLaunchAlt.EnableWindow(false);
	this->SetFocus();
}

void CLauncherTab::OnNew()
{
	m_ceName.EnableWindow(true);
	m_ceAccount.EnableWindow(true);
	m_cePassword.EnableWindow(true);
	m_ceIPAddress.EnableWindow(true);
	m_cePort.EnableWindow(true);
	m_ceAltClient.EnableWindow(true);

	m_stName.EnableWindow(true);
	m_stAcct.EnableWindow(true);
	m_stPass.EnableWindow(true);
	m_stIP.EnableWindow(true);
	m_stPort.EnableWindow(true);

	m_cbCancel.EnableWindow(true);
	m_cbDone.EnableWindow(true);
	m_cbBrowse.EnableWindow(true);

	m_cbNew.EnableWindow(false);
	m_cbEdit.EnableWindow(false);
	m_cbDelete.EnableWindow(false);
	m_cbLaunch2D.EnableWindow(false);
	m_cbLaunch3D.EnableWindow(false);
	m_cbLaunchAlt.EnableWindow(false);

	m_ceName.SetWindowText("");
	m_ceAccount.SetWindowText("");
	m_cePassword.SetWindowText("");
	m_ceIPAddress.SetWindowText("");
	m_cePort.SetWindowText("");
	m_ceAltClient.SetWindowText("");
	m_clbAccountList.SetCurSel(-1);
	m_iCurAccountSel = -1;
	this->SetFocus();
}

void CLauncherTab::OnDelete()
{
	if ( m_iCurAccountSel == -1 )
		return;
	CString csAccount, csMessage, csKey;
	m_clbAccountList.GetText(m_iCurAccountSel, csAccount);

	//Delete Confirmation
	csMessage.Format(AXT("Zugang \"%s\" wirklich l\xF6schen?"), csAccount);
	if ( AfxMessageBox(csMessage, MB_YESNO | MB_ICONQUESTION) == IDNO )
		return;

	csKey.Format("%s\\%s",REGKEY_ACCOUNT, csAccount);
	Main->DeleteRegistryKey(csKey);


	m_clbAccountList.DeleteString( m_iCurAccountSel );

	m_ceName.SetWindowText("");
	m_ceAccount.SetWindowText("");
	m_cePassword.SetWindowText("");
	m_ceIPAddress.SetWindowText("");
	m_cePort.SetWindowText("");
	m_ceAltClient.SetWindowText("");
	m_clbAccountList.SetCurSel(-1);

	m_cbNew.EnableWindow(true);

	m_ceName.EnableWindow(false);
	m_ceAccount.EnableWindow(false);
	m_cePassword.EnableWindow(false);
	m_ceIPAddress.EnableWindow(false);
	m_cePort.EnableWindow(false);
	m_ceAltClient.EnableWindow(false);

	m_stName.EnableWindow(false);
	m_stAcct.EnableWindow(false);
	m_stPass.EnableWindow(false);
	m_stIP.EnableWindow(false);
	m_stPort.EnableWindow(false);

	m_cbCancel.EnableWindow(false);
	m_cbDone.EnableWindow(false);
	m_cbBrowse.EnableWindow(false);
	m_cbEdit.EnableWindow(false);
	m_cbDelete.EnableWindow(false);

	m_cbLaunch2D.EnableWindow(false);
	m_cbLaunch3D.EnableWindow(false);
	m_cbLaunchAlt.EnableWindow(false);
	this->SetFocus();
}

void CLauncherTab::OnClientBrowse()
{
	CFileDialog dlg(TRUE, "exe", "", 0, "Application files (*.exe)|*.exe|All files (*.*)|*.*||", NULL);
	if ( dlg.DoModal() == IDOK )
		m_ceAltClient.SetWindowText(dlg.GetPathName());
}

// ---- Client launch: ClassicUO (and forks) and the OSI client ----

// Detects ClassicUO by file name or a settings.json next to the executable.
static bool IsClassicUOClient(const CString & csExe)
{
	CString csName = csExe.Mid(csExe.ReverseFind('\\') + 1);
	csName.MakeLower();
	if (csName.Find("classicuo") != -1)
		return true;
	CString csSettings = csExe.Left(csExe.ReverseFind('\\') + 1) + "settings.json";
	return GetFileAttributes(csSettings) != INVALID_FILE_ATTRIBUTES;
}

static CString QuoteArg(CString cs)
{
	cs.Replace("\"", "");
	return "\"" + cs + "\"";
}

// Writes the server to login.cfg and the account to uo.cfg (skipped if uo.cfg is missing).
bool CLauncherTab::WriteOsiConfig(const CString & csApp, const CString & csAccountName, const CString & csAccount,
	const CString & csPassword, const CString & csIP, const CString & csPort)
{
	CString csLoginFile;
	csLoginFile.Format("%slogin.cfg", (LPCTSTR) csApp);
	CStdioFile csfLoginFile;
	if (!csfLoginFile.Open(csLoginFile, CFile::modeWrite | CFile::modeCreate))
	{
		AfxMessageBox(AXT("login.cfg des Clients l\xE4sst sich nicht schreiben."), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	CString csOutput;
	csOutput.Format("; login.cfg file for the %s server\n", (LPCTSTR) csAccountName);
	csfLoginFile.WriteString(csOutput);
	csOutput.Format("LoginServer=%s,%s\n", (LPCTSTR) csIP, (LPCTSTR) csPort);
	csfLoginFile.WriteString(csOutput);
	csfLoginFile.Close();

	CString csUOFile;
	csUOFile.Format("%suo.cfg", (LPCTSTR) csApp);
	CStdioFile csfUOFile;
	if (!csfUOFile.Open(csUOFile, CFile::modeRead | CFile::shareDenyNone))
	{
		AxisSetStatus(AXT("Keine uo.cfg beim Client - Server ist eingetragen, das Konto bitte im Client eingeben."), 2);
		return true;
	}
	CString csTmpFile;
	csTmpFile.Format("%suo.axis", (LPCTSTR) csApp);
	CStdioFile csfTmpFile;
	if (!csfTmpFile.Open(csTmpFile, CFile::modeWrite | CFile::modeCreate))
	{
		csfUOFile.Close();
		AfxMessageBox(AXT("Tempor\xE4re Datei l\xE4sst sich nicht schreiben."), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	csOutput.Format("; UO.CFG for the %s server\n", (LPCTSTR) csAccountName);
	csfTmpFile.WriteString(csOutput);
	CString csInput;
	while (csfUOFile.ReadString(csInput))
	{
		if ( (csInput.Find(";") == -1) &&
			(csInput.Find("SavePassword=") == -1) &&
			(csInput.Find("AcctID=") == -1) &&
			(csInput.Find("AcctPassword=") == -1) &&
			(csInput.Find("RememberAcctPW") == -1))
		{
			csfTmpFile.WriteString(csInput);
			csfTmpFile.WriteString("\n");
		}
	}
	csfTmpFile.WriteString("SavePassword=on\n");
	csOutput.Format("AcctID=%s\n", (LPCTSTR) csAccount);
	csfTmpFile.WriteString(csOutput);
	csOutput.Format("AcctPassword=%s\n", (LPCTSTR) EncryptUO(csPassword));
	csfTmpFile.WriteString(csOutput);
	csfTmpFile.WriteString("RememberAcctPW=on\n");
	csfTmpFile.Close();
	csfUOFile.Close();
	ReplaceFiles(csTmpFile, csUOFile);
	return true;
}

void CLauncherTab::LaunchClient(CString csExe)
{
	csExe.Trim();
	if (csExe.IsEmpty() || GetFileAttributes(csExe) == INVALID_FILE_ATTRIBUTES)
	{
		AxisSetStatus(AXT("Client nicht gefunden: ") + (csExe.IsEmpty() ? CString("kein Pfad eingetragen") : csExe), 3);
		return;
	}
	CString csAccountName, csAccount, csIP, csPort, csPassword;
	m_ceName.GetWindowText(csAccountName);
	m_ceAccount.GetWindowText(csAccount);
	m_cePort.GetWindowText(csPort);
	m_ceIPAddress.GetWindowText(csIP);
	m_cePassword.GetWindowText(csPassword);
	CString csApp = csExe.Left(csExe.ReverseFind('\\') + 1);

	CString csCmdLine = QuoteArg(csExe);
	if (IsClassicUOClient(csExe))
	{
		// ClassicUO takes the login data as command-line arguments.
		if (!csIP.IsEmpty())
			csCmdLine += " -ip " + QuoteArg(csIP);
		if (!csPort.IsEmpty())
			csCmdLine += " -port " + csPort;
		if (!csAccount.IsEmpty())
			csCmdLine += " -username " + QuoteArg(csAccount);
		if (!csPassword.IsEmpty())
			csCmdLine += " -password " + QuoteArg(csPassword);
	}
	else if (!WriteOsiConfig(csApp, csAccountName, csAccount, csPassword, csIP, csPort))
		return;

	STARTUPINFO start;
	memset( &start, 0x00, sizeof(start) );
	start.cb = sizeof(start);
	PROCESS_INFORMATION procInfo;
	memset( &procInfo, 0x00, sizeof(procInfo) );
	CString csCmdBuf = csCmdLine;
	if ( !CreateProcess( csExe, csCmdBuf.GetBuffer(), NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, csApp, &start, &procInfo ) )
	{
		csCmdBuf.ReleaseBuffer();
		DWORD dwError = GetLastError();
		CString csMessage;
		csMessage.Format(AXT("Programm konnte nicht gestartet werden (Fehler %d):\n%s"), dwError, (LPCTSTR) csExe);
		Main->m_log.Add(1, csMessage);
		AfxMessageBox(csMessage);
		return;
	}
	csCmdBuf.ReleaseBuffer();
	CloseHandle(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
	AxisSetStatus(AXT("Client gestartet: ") + csExe.Mid(csExe.ReverseFind('\\') + 1), 1);
}

void CLauncherTab::OnLaunch2D()
{
	// Default client from the settings; a folder path falls back to client.exe inside it.
	CString csExe = csUOPath;
	if (csExe.Right(4).CompareNoCase(".exe") != 0)
		csExe = csExe.Left(csExe.ReverseFind('\\') + 1) + "client.exe";
	if (csUOPath.IsEmpty())
	{
		AxisSetStatus(AXT("Kein Standard-Client eingetragen - Einstellungen > Dateipfade oder \"Anderen Client starten\" nutzen."), 2);
		return;
	}
	LaunchClient(csExe);
}

void CLauncherTab::OnLaunch3D()
{
	// The 3D client is not supported; the button is hidden.
}

void CLauncherTab::OnLaunchAlt()
{
	CString csExe;
	m_ceAltClient.GetWindowText(csExe);
	LaunchClient(csExe);
}

void CLauncherTab::OnLaunchAuto()
{
	CString csCmd, csApp;
	csApp = csUOPath;
	csApp = csApp.Left(csApp.ReverseFind('\\')+1);
	csCmd.Format("%suopatch.exe",csApp);

	STARTUPINFO start;
	memset( &start, 0x00, sizeof(start) );
	start.cb = sizeof(start);
	PROCESS_INFORMATION procInfo;
	memset( &procInfo, 0x00, sizeof(procInfo) );
	if ( ! CreateProcess( csCmd, NULL, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, csApp, &start, &procInfo ) )
	{
		DWORD dwError = GetLastError();
		CString csMessage;
		csMessage.Format(AXT("Programm konnte nicht gestartet werden (Fehler %d):\n%s"),dwError, csCmd);
		Main->m_log.Add(1,csMessage);
		AfxMessageBox(csMessage);
	}
}