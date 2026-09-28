/*
	CString csMessage;
	csMessage.Format(AXT("%d"), );
	AfxMessageBox(csMessage);
*/

#include "stdafx.h"
#include "AxisX.h"
#include "AxisXDlg.h"
#include <shlobj.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAxisXApp * Main;
/////////////////////////////////////////////////////////////////////////////
// CAxisXApp

BEGIN_MESSAGE_MAP(CAxisXApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAxisXApp construction

CAxisXApp::CAxisXApp()
{
	Main = this;
	EnableHtmlHelp();
	m_pcppAxisLogTab = NULL;
	m_pcppDashboardTab = NULL; // checked by AxisLog.cpp before the tab exists
	m_pRConsole = NULL;
	m_iReceiveTimeout = 60000;
}

// The one and only CAxisXApp object

CAxisXApp theApp;

// CAxisXApp initialization

BOOL CAxisXApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	BootstrapDarkMode();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	AfxInitRichEdit();

	//*******************************
	// Set Application Root Path
	TCHAR str[MAX_PATH];
	GetModuleFileName(NULL, str, MAX_PATH);
	CString strDir(str);
	m_csRootDirectory = strDir.Left(strDir.ReverseFind(_T('\\'))+1);

	//Load Language File
	LoadLang();

	m_pcppAxisLogTab = new CLogTab;
	m_pcppAxisLogTab->m_dcCurrentPage = m_pcppAxisLogTab;
	m_pScripts = new CScriptObjects;
	m_log.Add(0,CFMsg(CMsg("IDS_START"), Main->GetVersionTitle(), Main->GetBuildTimestamp()));
	m_csCurentProfile = CMsg("IDS_NONE");

	//*******************************
	// Load the Default settings
	m_log.Add(0,CMsg("IDS_LOADDEFAULT"));
	LoadIni();
	if (!hRegLocation)
		hRegLocation = HKEY_LOCAL_MACHINE;
	m_dwInitiallize = 1;
	crAxisBkg = RGB(249,249,251);
	m_bkbrush.CreateSolidBrush(crAxisBkg);

	//*******************************
	// Load the Registry settings
	HKEY hKey;
	if ( RegOpenKeyEx(hRegLocation, REGKEY_AXIS, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
	{
		m_log.Add(0,CMsg("IDS_LOADREGISTRY"));
		m_dwAllowMultiple = GetRegistryDword("AllowMultipleInstances", m_dwAllowMultiple);
		m_dwAlwaysOnTop = GetRegistryDword("AlwaysOnTop", m_dwAlwaysOnTop);
		m_dwDisableToolbar = GetRegistryDword("DisableToolbar", m_dwDisableToolbar);
		m_dwDrawDifs = GetRegistryDword("DrawDifs", m_dwDrawDifs);
		m_dwDrawStatics = GetRegistryDword("DrawStatics", m_dwDrawStatics);
		m_dwLoadDefault = GetRegistryDword("LoadDefault", m_dwLoadDefault);
		m_csPosition = GetRegistryString("Position", m_csPosition);
		m_dwRoomView = GetRegistryDword("RoomView", m_dwRoomView);
		m_dwSameAsClient = GetRegistryDword("SameAsCLient", m_dwSameAsClient);
		m_dwStartTab = GetRegistryDword("StartTab", m_dwStartTab);
		m_dwSysClose = GetRegistryDword("SysClose", m_dwSysClose);
		m_dwInitiallize = GetRegistryDword("Initallize", 1);
		m_dwShowSpawnpoints = GetRegistryDword("ShowSpawnpoints", m_dwShowSpawnpoints);
		m_dwNPCSpawnColor = GetRegistryDword("NPCSpawnColor", m_dwNPCSpawnColor);
		m_dwItemSpawnColor = GetRegistryDword("ItemSpawnColor", m_dwItemSpawnColor);
		m_dwItemBGColor = GetRegistryDword("ItemBGColor", m_dwItemBGColor);
		m_dwSpawnBGColor = GetRegistryDword("SpawnBGColor", m_dwSpawnBGColor);
		// Default preview backgrounds follow the light/dark theme; custom values are kept.
		AxisMapPreviewBg(m_dwItemBGColor);
		AxisMapPreviewBg(m_dwSpawnBGColor);
		m_dwShowItems = GetRegistryDword("ShowItems", m_dwShowItems);
		m_dwShowNPCs = GetRegistryDword("ShowNPCs", m_dwShowNPCs);
		m_dwShowMap = GetRegistryDword("ShowMap", m_dwShowMap);
		m_csCommandPrefix = GetRegistryString("CommandPrefix", m_csCommandPrefix);
		m_csUOTitle = GetRegistryString("UOTitle", m_csUOTitle);
		m_csCustomLogo = GetRegistryString("Custom Logo", m_csCustomLogo);
		m_csCustomIcon = GetRegistryString("Custom Icon", m_csCustomIcon);
	}
	RegCloseKey(hKey);

	//*******************************
	// Find the path to the UO client
	m_log.Add(0,CMsg("IDS_CLIENTINST"));
	csUOPath = GetRegistryString("Default Client", "");

	if ( csUOPath == "" )
		csUOPath = GetRegistryString("ExePath", "", HKEY_LOCAL_MACHINE, "SOFTWARE\\Origin Worlds Online\\Ultima Online\\1.0");

	PutRegistryString("Default Client", csUOPath);

	if ( csUOPath == "" )
		m_log.Add(1,CMsg("IDS_ERRNOCLIENT"));
	else
	{
	//*******************************
	// Check Client info
	m_pClient = new CClientInfo;
	m_pClient->SetClientBuffer();
	m_log.Add(0,CFMsg(CMsg("IDS_CLIENTVER"), m_pClient->GetClientVersion()));
	m_pClient->DeleteClientBuffer();

	m_log.Add(0,CFMsg(CMsg("IDS_CLIENTPATH"), csUOPath));
	}

	//*******************************
	// Find the path to the UO Mul files

	csMulPath = GetRegistryString("Default MulPath", "");

	if ((( csMulPath == "" ) || (m_dwSameAsClient))&&( csUOPath != "" ))
	{
		csMulPath = csUOPath;
		csMulPath = csUOPath.Left(csUOPath.ReverseFind('\\')+1);
	}

	PutRegistryString("Default MulPath", csMulPath);

	if ( csMulPath == "" )
	{
		bLoadedMul = false;
		m_log.Add(1,CMsg("IDS_ERRNOMUL"));
	}
	else
	{
		bLoadedMul = true;
		m_log.Add(0,CFMsg(CMsg("IDS_MULPATH"), csMulPath));
	}

	//*******************************
	// Find Default Profile Path

	csProfilePath = GetRegistryString("Default ProfilePath", "");
	// Without a valid script folder, fall back to the first folder containing a server profile
	// (*.scp): %LOCALAPPDATA%\AxisX\Profile, then "Profile" next to AxisX.exe.
	bool bProfilePathValid = ( csProfilePath != "" ) && ( GetFileAttributes(csProfilePath) != INVALID_FILE_ATTRIBUTES )
		&& ( GetFileAttributes(csProfilePath) & FILE_ATTRIBUTE_DIRECTORY );
	CString csShippedProfile;
	CString aProfileDirs[2] = { AxisUserProfileDir(false), m_csRootDirectory + "Profile" };
	for ( int iDir = 0; iDir < 2 && csShippedProfile.IsEmpty(); iDir++ )
	{
		WIN32_FIND_DATA fdScp;
		HANDLE hScp = FindFirstFile(aProfileDirs[iDir] + "\\*.scp", &fdScp);
		if ( hScp != INVALID_HANDLE_VALUE )
		{
			FindClose(hScp);
			csShippedProfile = aProfileDirs[iDir];
		}
	}
	bool bShippedProfile = !csShippedProfile.IsEmpty();
	if ( !bProfilePathValid && bShippedProfile )
	{
		csProfilePath = csShippedProfile;
		PutRegistryString("Default ProfilePath", csProfilePath);
		PutRegistryString("Last Profile Loaded", "<Axis Profile>");
		PutRegistryString("Default Profile", "<Axis Profile>");
		m_log.Add(0, AXT("Server-Profil geladen aus: %s"), (LPCTSTR) csProfilePath);
	}
	if ( csProfilePath == "" )
	{
		CString pathSelected;
		CFolderDialog dlgfolder(&pathSelected, CMsg("IDS_DEFAULT_PROFILE_FOLDER"));
		if (dlgfolder.DoModal() == IDOK)
			csProfilePath = pathSelected;
		else
			csProfilePath = "C:/Sphere";
		PutRegistryString("Default ProfilePath", csProfilePath);
		PutRegistryString("Last Profile Loaded", "<Axis Profile>");	// Internal name, not translated
	}
	m_log.Add(0,CFMsg(CMsg("IDS_DEFAULT_PROFILE_PATH"), csProfilePath));

	//*******************************
	// Load Mul files

	InitializeMulPaths();
	LoadBodyDef();
	LoadBodyConvert();
	LoadHues();
	LoadSounds();
	LoadMusic();

	//*******************************
	// Check for multiple instances
	m_log.Add(0,CMsg("IDS_MULTIPLE_INSTANCES"));
	if (!m_dwAllowMultiple)
	{
		if (FindInstance())
		{
			return FALSE;
		}
	}

	CAxisXDlg dlg(CFMsg(CMsg("IDS_AXISTITLE"), Main->GetVersionTitle()));
	dlg.m_psh.dwFlags = PSH_HASHELP | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	dlg.EnableStackedTabs( false );

	//*******************************
	// Create Tabs
	m_log.Add(0,CMsg("IDS_CREATE_TABS"));

	m_pcppDashboardTab = new CDashboardTab;
	m_pcppGeneralTab = new CGeneralTab;
	m_pcppTravelTab = new CTravelTab;
	m_pcppItemTab = new CItemTab;
	m_pcppItemTweakTab = new CItemTweakTab;
	m_pcppLauncherTab = new CLauncherTab;
	m_pcppSpawnTab = new CSpawnTab;
	m_pcppPlayerTweakTab = new CPlayerTweakTab;
	m_pcppCommandsTab = new CCommandsTab;
	m_pcppAccountTab = new CAccountTab;
	m_pcppReminderTab = new CReminderTab;
	m_pcppMiscTab = new CMiscTab;

	//Settings Pages
	m_pcppSetGeneral = new CSettingsGeneral;
	m_pcppSetPaths = new CSettingsPaths;
	m_pcppSetItem = new CSettingsItem;
	m_pcppSetTravel = new CSettingsTravel;
	m_pcppSetSpawn = new CSettingsSpawn;
	m_pcppSetOverridePaths = new CSettingsOverridePaths;

	dlg.AddPage(m_pcppDashboardTab);
	dlg.AddPage(m_pcppGeneralTab);
	dlg.AddPage(m_pcppTravelTab);
	dlg.AddPage(m_pcppSpawnTab);
	dlg.AddPage(m_pcppPlayerTweakTab);
	dlg.AddPage(m_pcppItemTab);
	dlg.AddPage(m_pcppItemTweakTab);
	dlg.AddPage(m_pcppAccountTab);
	dlg.AddPage(m_pcppMiscTab);
	dlg.AddPage(m_pcppLauncherTab);
	dlg.AddPage(m_pcppCommandsTab);
	dlg.AddPage(m_pcppReminderTab);
	dlg.AddPage(m_pcppAxisLogTab);

	m_pMainWnd = &dlg;


	//*******************************
	//Check what tab we start with
	if (m_dwStartTab != -1)
	{
		switch(m_dwStartTab)
		{
		case 0:
			dlg.SetActivePage(m_pcppGeneralTab);
			break;
		case 1:
			dlg.SetActivePage(m_pcppTravelTab);
			break;
		case 2:
			dlg.SetActivePage(m_pcppSpawnTab);
			break;
		case 3:
			dlg.SetActivePage(m_pcppPlayerTweakTab);
			break;
		case 4:
			dlg.SetActivePage(m_pcppItemTab);
			break;
		case 5:
			dlg.SetActivePage(m_pcppItemTweakTab);
			break;
		case 6:
			dlg.SetActivePage(m_pcppAccountTab);
			break;
		case 7:
			dlg.SetActivePage(m_pcppMiscTab);
			break;
		case 8:
			dlg.SetActivePage(m_pcppLauncherTab);
			break;
		case 9:
			dlg.SetActivePage(m_pcppCommandsTab);
			break;
		case 10:
			dlg.SetActivePage(m_pcppReminderTab);
			break;
		case 11:
			dlg.SetActivePage(m_pcppAxisLogTab);
			break;
		case 12:
			dlg.SetActivePage(m_pcppDashboardTab);
			break;
		default:
			dlg.SetActivePage(m_pcppGeneralTab);
			break;
		}
	}

	//*******************************
	// Check to see if it the first time we run Axis
#ifndef _DEBUG
	if ( m_dwInitiallize == 1 )
	{
		dlg.m_bFirstRunSettings = true;
		PutRegistryDword("Initallize", 0);
		PutRegistryString("Default Profile", "<Axis Profile>");	// Internal name, not translated
	}
#endif


	//*******************************
	// Loading default profile
	if (m_dwLoadDefault)
		AfxBeginThread(LoadProfileThread,(LPVOID)1);


	//*******************************
	//Start Reminder Thread
	AfxBeginThread(Reminder,NULL);


	//*******************************
	//Finish Loading
	m_log.Add(0,CMsg("IDS_LOADFINISH"));
	m_pcppAxisLogTab->SetVisible(true);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	return TRUE;
}

CAxisXApp::~CAxisXApp()
{
	if ( m_pScripts )
		delete m_pScripts;

	if ( m_pClient )
		delete m_pClient;

	delete m_pcppDashboardTab;
	delete m_pcppGeneralTab;
	delete m_pcppTravelTab;
	delete m_pcppSpawnTab;
	delete m_pcppPlayerTweakTab;
	delete m_pcppItemTab;
	delete m_pcppItemTweakTab;
	delete m_pcppLauncherTab;
	delete m_pcppCommandsTab;
	delete m_pcppAccountTab;
	delete m_pcppAxisLogTab;
	delete m_pcppReminderTab;
	delete m_pcppMiscTab;

	if (m_dlgWorldMap)
		delete m_dlgWorldMap;
	if (m_dlgDoorWiz)
		delete m_dlgDoorWiz;
	if (m_dlgLightWiz)
		delete m_dlgLightWiz;
	if (m_dlgProfile)
		delete m_dlgProfile;
	if (m_dlgMultiView)
		delete m_dlgMultiView;
	if (m_dlgColorPal)
		delete m_dlgColorPal;
	if (m_dlgToolBar)
		delete m_dlgToolBar;

	delete m_pcppSetGeneral;
	delete m_pcppSetPaths;
	delete m_pcppSetItem;
	delete m_pcppSetTravel;
	delete m_pcppSetSpawn;
	delete m_pcppSetOverridePaths;


	m_pcppAxisLogTab = NULL;

	UnLoadLog();
	UnLoadBodyDef();
	UnLoadBodyConvert();
	UnLoadHues();
	UnLoadSounds();
	UnLoadMusic();
}

//////////////////////////////////////////////////////////////////////////

BOOL CAxisXApp::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == 256) && (pMsg->wParam == 27))
	{
		// this is the escape key
		return (TRUE);
	}
	return CWinApp::PreTranslateMessage(pMsg);
}

bool CAxisXApp::FindInstance()
{
	// try to determine if there is already an instance of this application running.
	EnumWindows(EnumInstanceProc, 0);
	if (hwndHoGInstance)
	{
		// Found the client window
		CWnd * pWnd = CWnd::FromHandle(hwndHoGInstance);
		pWnd->ShowWindow(SW_SHOWNORMAL);
		pWnd->SetForegroundWindow();
		return true;
	}
	else
		return false;

}

//////////////////////////////////////////////////////////////////////////

DWORD CAxisXApp::GetRegistryDword(CString csValue, DWORD dwDefault, HKEY hMainKey, CString csSubKey)
{
	HKEY hKey;
	if ( RegOpenKeyEx(hMainKey, csSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS )
		return dwDefault;
	DWORD dwType, dwSize, dwValue;
	dwSize = sizeof(DWORD);
	LONG lStatus = RegQueryValueEx(hKey, LPCTSTR(csValue), 0, &dwType, (BYTE *) &dwValue, &dwSize);
	if ( lStatus != ERROR_SUCCESS )
	{
		if ( lStatus != ERROR_FILE_NOT_FOUND )
		{
			char szBuffer[MAX_BUFFER];
			FormatWin32Error(GetLastError(), szBuffer, MAX_BUFFER);
			m_log.Add(1,CFMsg(CMsg("IDS_WARNING_READ_REG_VAL"), csValue, szBuffer));
		}
		RegCloseKey(hKey);
		return dwDefault;
	}
	if ( dwType != REG_DWORD )
	{
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_REG_TYPE"), csValue, dwType, REG_DWORD));
		RegCloseKey(hKey);
		return dwDefault;
	}
	RegCloseKey(hKey);
	return dwValue;
}

CString CAxisXApp::GetRegistryString(CString csValue, CString csDefault, HKEY hMainKey, CString csSubKey)
{
	HKEY hKey;
	if ( RegOpenKeyEx(hMainKey, csSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS )
		return csDefault;
	DWORD dwType, dwSize;
	char szBuffer[MAX_BUFFER];
	memset(&szBuffer[0], 0x00, sizeof(szBuffer));
	dwSize = sizeof(szBuffer);
	LONG lStatus = RegQueryValueEx(hKey, LPCTSTR(csValue), 0, &dwType, (BYTE *) &szBuffer[0], &dwSize);
	if ( lStatus != ERROR_SUCCESS )
	{
		if ( lStatus != ERROR_FILE_NOT_FOUND )
		{
			char szBuffer[MAX_BUFFER];
			FormatWin32Error(GetLastError(), szBuffer, MAX_BUFFER);
			m_log.Add(1,CFMsg(CMsg("IDS_WARNING_READ_REG_VAL"), csValue, szBuffer));
		}
		RegCloseKey(hKey);
		return csDefault;
	}
	if ( dwType != REG_SZ )
	{
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_REG_TYPE"), csValue, dwType, REG_SZ));
		RegCloseKey(hKey);
		return csDefault;
	}
	RegCloseKey(hKey);
	return _T(szBuffer);
}

void CAxisXApp::GetRegistryMultiSz(CString csValue, CStringArray * pStrings, HKEY hMainKey, CString csSubKey)
{
	// Make sure the input array is empty
	pStrings->RemoveAll();

	HKEY hKey;
	if ( RegOpenKeyEx(hMainKey, csSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS )
		return;
	DWORD dwType, dwSize;
	// Need to find out how big the buffer is
	dwSize = 0;
	LONG lStatus = RegQueryValueEx(hKey, LPCTSTR(csValue), 0, &dwType, NULL, &dwSize);

	if ( dwSize == 0 )
	{
		RegCloseKey(hKey);
		return;
	}

	char * pszBuffer = new char [dwSize];
	memset(pszBuffer, 0x00, dwSize);
	lStatus = RegQueryValueEx(hKey, LPCTSTR(csValue), 0, &dwType, (BYTE*) pszBuffer, &dwSize);
	char * pDataPtr = pszBuffer;

	if ( lStatus != ERROR_SUCCESS )
	{
		if ( lStatus != ERROR_FILE_NOT_FOUND )
		{
			char szBuffer[MAX_BUFFER];
			FormatWin32Error(GetLastError(), szBuffer, MAX_BUFFER);
			m_log.Add(1,CFMsg(CMsg("IDS_WARNING_READ_REG_VAL"), csValue, szBuffer));
		}
	}
	else
	{
		// parse the registry value
		DWORD dwOffset = 0;
		while ( dwOffset < dwSize )
		{
			CString csValue = _T(pDataPtr);
			pStrings->Add(csValue);
			pDataPtr += csValue.GetLength() + 1;
			dwOffset += csValue.GetLength() + 1;
		}
		
	}
	if ( dwType != REG_MULTI_SZ )
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_REG_TYPE"), csValue, dwType, REG_MULTI_SZ));
	RegCloseKey(hKey);
	delete [] pszBuffer;
}

//////////////////////////////////////////////////////////////////////////

void CAxisXApp::PutRegistryDword(CString csValue, DWORD dwValue, HKEY hMainKey, CString csSubKey)
{
	HKEY hKey;
	DWORD dwDisp;
	LONG lStatus = RegCreateKeyEx(hMainKey, csSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);

	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_OPEN_REG"), csSubKey, szBuffer));
		return;
	}
	lStatus = RegSetValueEx(hKey, csValue, 0, REG_DWORD, ((BYTE *) &dwValue), sizeof(DWORD) );
	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_WRITE_REG_VAL"), csValue, szBuffer));
	}
	RegCloseKey(hKey);
}

void CAxisXApp::PutRegistryString(CString csValue, CString csString, HKEY hMainKey, CString csSubKey)
{
	HKEY hKey;
	DWORD dwDisp;

	LONG lStatus = RegCreateKeyEx(hMainKey, csSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);

	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_OPEN_REG"), csSubKey, szBuffer));
		return;
	}
	lStatus = RegSetValueEx(hKey, csValue, 0, REG_SZ, ((BYTE *) csString.GetBuffer(csString.GetLength()) ), csString.GetLength() );
	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_WRITE_REG_VAL"), csValue, szBuffer));
	}
	RegCloseKey(hKey);
}

void CAxisXApp::PutRegistryMultiSz(CString csValue, CStringArray * pStrings, HKEY hMainKey, CString csSubKey)
{
	HKEY hKey;
	DWORD dwDisp;
	LONG lStatus = RegCreateKeyEx(hMainKey, csSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);

	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_OPEN_REG"), csSubKey, szBuffer));
		return;
	}

	int i = 0;
	int iLength = 0;
	for ( i = 0; i <= pStrings->GetUpperBound(); i++ )
		iLength += ( pStrings->GetAt(i).GetLength() + 1);

	char * pszStrings = new char [iLength + 1];

	memset(pszStrings, 0x00, iLength + 1);
	char * ptr = pszStrings;
	for ( i = 0; i <= pStrings->GetUpperBound(); i++ )
	{
		memcpy(ptr, LPCTSTR(pStrings->GetAt(i)), pStrings->GetAt(i).GetLength());
		ptr += pStrings->GetAt(i).GetLength() + 1;
	}
	lStatus = RegSetValueEx(hKey, csValue, 0, REG_MULTI_SZ, ((BYTE *) pszStrings), iLength );
	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_WRITE_REG_VAL"), csValue, szBuffer));
	}
	RegCloseKey(hKey);

	delete [] pszStrings;
}

//////////////////////////////////////////////////////////////////////////

void CAxisXApp::DeleteRegistryKey(CString csKeyName, HKEY hRootKey)
{
	HKEY hKey;
	if ( RegOpenKeyEx(hRootKey, csKeyName, 0, KEY_READ, &hKey) == ERROR_FILE_NOT_FOUND )
		return;
	RegCloseKey(hKey);
	LONG lStatus = RegDeleteKey(hRootKey, csKeyName);
	if ( lStatus != ERROR_SUCCESS )
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_DELETE_REG_KEY"), csKeyName, szBuffer));
	}
	return;
}

void CAxisXApp::DeleteRegistryValue(CString csValueName, CString csSubKey, HKEY hRootKey)
{
	HKEY hKey;
	LONG lStatus = RegOpenKey(hRootKey, csSubKey, &hKey);
	if ( lStatus == ERROR_SUCCESS )
	{
		lStatus = RegDeleteValue(hKey, csValueName);
		if ( lStatus == ERROR_FILE_NOT_FOUND)
			return;
		if ( lStatus != ERROR_SUCCESS )
		{
			char szBuffer[MAX_BUFFER];
			FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
			m_log.Add(1,CFMsg(CMsg("IDS_WARNING_DELETE_REG_VAL"), csValueName, szBuffer));
		}
		RegCloseKey(hKey);
	}
	else
	{
		char szBuffer[MAX_BUFFER];
		FormatWin32Error(lStatus, szBuffer, MAX_BUFFER);
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_OPEN_REG"), csSubKey, szBuffer));
	}
	return;
}

//////////////////////////////////////////////////////////////////////////

CString CAxisXApp::GetVersionTitle()
{
	CString csExe = AfxGetAppName();
	csExe += ".exe";
	char * pszExe = new char [csExe.GetLength() + 1];
	sprintf_s(pszExe,csExe.GetLength()+1, csExe);
	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize(pszExe, &dwHandle);
	char * pBuffer = new char [dwSize];
	GetFileVersionInfo(pszExe, dwHandle, dwSize, (LPVOID) pBuffer);
	CString csVersionInfo;
	unsigned int iDataSize = 80;
	LPVOID pData;
	VerQueryValue(pBuffer, _T("\\StringFileInfo\\040904b0\\ProductVersion"), &pData, &iDataSize);
	csVersionInfo.Format("%s", pData);

	delete [] pszExe;
	delete [] pBuffer;
	
	csVersionInfo.Replace(",", ".");
	csVersionInfo.Replace(" ", "");

	return CFMsg(CMsg("IDS_AXIS_VERSION"),csVersionInfo);
}


CString CAxisXApp::GetBuildTimestamp()
{
	return CFMsg("%1 %2", __DATE__, __TIME__);
}

// Resolves a branding setting (local path or http(s) URL) to a local file,
// downloading URLs into %LOCALAPPDATA%\AxisX\Branding.
CString CAxisXApp::ResolveBrandingFile(CString csValue, LPCTSTR pszCacheName)
{
	csValue.TrimLeft();
	csValue.TrimRight();
	if ( csValue.IsEmpty() )
		return _T("");

	CFileStatus fs;
	if ( csValue.Left(7).CompareNoCase(_T("http://")) == 0
		|| csValue.Left(8).CompareNoCase(_T("https://")) == 0 )
	{
		TCHAR szPath[MAX_PATH];
		if ( FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)) )
			return _T("");
		CString csDir;
		csDir.Format(_T("%s\\AxisX\\Branding"), szPath);
		SHCreateDirectoryEx(NULL, csDir, NULL);
		CString csLocal;
		csLocal.Format(_T("%s\\%s"), csDir, pszCacheName);
		if ( URLDownloadToFile(NULL, csValue, csLocal, 0, NULL) != S_OK )
			m_log.Add(1, AXT("WARNING: Unable to download branding file %s"), (LPCTSTR) csValue);
		return CFile::GetStatus(csLocal, fs) ? csLocal : CString(_T(""));
	}

	return CFile::GetStatus(csValue, fs) ? csValue : CString(_T(""));
}

void CAxisXApp::UpdateProfileMenu()
{
	CMenu* pSubMenu = pDefMenu.GetSubMenu(1);

	if (pSubMenu)
	{
		pSubMenu->ModifyMenu(2,MF_BYPOSITION | MF_STRING,ID_PROFILES_UNLOAD,CFMsg(CMsg("IDS_UNLOAD_PROFILE"),m_csCurentProfile));
		if (m_csCurentProfile == CMsg("IDS_NONE"))
			pSubMenu->EnableMenuItem(2, MF_BYPOSITION | MF_GRAYED);
		else
			pSubMenu->EnableMenuItem(2, MF_BYPOSITION | MF_ENABLED);
		pSubMenu->ModifyMenu(3,MF_BYPOSITION | MF_STRING,ID_PROFILES_LOADDEFAULT,CFMsg(CMsg("IDS_LOAD_DEFAULT_PROFILE"),GetRegistryString("Default Profile")));
		pSubMenu->ModifyMenu(4,MF_BYPOSITION | MF_STRING,ID_PROFILES_LOADLASTPROFILE,CFMsg(CMsg("IDS_LOAD_LAST_PROFILE"),GetRegistryString("Last Profile Loaded")));
	}
}

void CAxisXApp::UpdateProfileSettings()
{
	int iDef = m_pScripts->m_aDefList.Find("USEMAPDIFFS");
	if ( iDef != -1 )
	{
		CSObject * pDef = (CSObject *) m_pScripts->m_aDefList.GetAt(iDef);
		m_dwDrawDifs = atoi(pDef->m_csID);
	}
	else
		m_dwDrawDifs = GetRegistryDword("DrawDifs", m_dwDrawDifs);
}

//*************************************
//Load Muls

void CAxisXApp::InitializeMulPaths()
{
	saMulPaths.SetSize(VERFILE_QTY);

	CString csPath;

	for(int i = 0; i < VERFILE_QTY; i++)
	{
		if ( i == VERFILE_MUSICLIST)
			csPath.Format(_T("%sMusic\\Digital\\%s"), csMulPath, GetMulFileName(i));
		else
			csPath.Format(_T("%s%s"), csMulPath, GetMulFileName(i));
		SetMulPath((short)i, csPath);
	}
	LoadCustomPaths();
}

void CAxisXApp::LoadCustomPaths()
{

	for(int i = 0; i < VERFILE_QTY; i++)
	{
		CString csPath = Main->GetRegistryString(GetMulFileName(i), "", hRegLocation, REGKEY_OVERRIDEPATH);
		if (csPath != "")
			SetMulPath((short)i, csPath);
	}
}

BOOL CAxisXApp::SetMulPath(short sFileIndex, LPCTSTR pszFilePath)
{
	if ( sFileIndex < 0 || sFileIndex > saMulPaths.GetUpperBound() )
		return FALSE;	// Invalid index!
	if ( _tcslen(pszFilePath) == 0 )
		return FALSE;	// Invalid filename!
	saMulPaths.SetAt(sFileIndex, pszFilePath);
	return TRUE;
}

CString CAxisXApp::GetMulPath(short sFileIndex)
{
	CString strResult;
	if ( sFileIndex < 0 || sFileIndex > saMulPaths.GetUpperBound() )
	{
		// Do nothing...we never set these
	}
	else
		strResult = saMulPaths.GetAt(sFileIndex);
	return strResult;
}

void CAxisXApp::LoadBodyDef()
{
	CString csPath = GetMulPath(VERFILE_BODYDEF);
	CStdioFile csfInput;

		if ( !csfInput.Open(csPath, CFile::modeRead | CFile::shareDenyNone) )
			return;

	BOOL bStatus = TRUE;
	while ( bStatus )
	{
		CString csLine;
		bStatus = csfInput.ReadString(csLine);
		if ( !bStatus )
			break;
		csLine = csLine.SpanExcluding("#");
		csLine.Trim();
		if ( csLine != "" )
		{
			CDefObj * pDef = new CDefObj;
			CString csTemp;
			csTemp = csLine.SpanExcluding(" \t");
			pDef->wValue = (WORD)atoi(csTemp);

			csTemp = csLine.Mid(csLine.Find("{") + 1);
			csTemp = csTemp.SpanExcluding("}");
			pDef->wID = (WORD)atoi(csTemp);

			m_daBodydef.Insert(pDef);
		}
	}
				
}

void CAxisXApp::LoadBodyConvert()
{
	CString csPath = GetMulPath(VERFILE_BODYCONVDEF);
	CStdioFile csfInput;

		if ( !csfInput.Open(csPath, CFile::modeRead | CFile::shareDenyNone) )
			return;

	BOOL bStatus = TRUE;
	while ( bStatus )
	{
		CString csLine;
		bStatus = csfInput.ReadString(csLine);
		if ( !bStatus )
			break;
		csLine = csLine.SpanExcluding("#");
		csLine.Trim();
		if ( csLine != "" )
		{
			CDefObj * pDef = new CDefObj;
			CString csTemp;
			int iLength, iMul = 2;
			csTemp = csLine.SpanExcluding(" \t");
			iLength = csTemp.GetLength()+1;
			if (atoi(csTemp)==0)
			{
				delete pDef;
				continue;
			}
			pDef->wValue = (WORD)atoi(csTemp);
			
			while(1)
			{
				csTemp = csLine.Mid(iLength);
				csTemp = csTemp.SpanExcluding(" \t");
				iLength += csTemp.GetLength()+1;
				if (csTemp == "")
				{
					pDef->wID = pDef->wValue;
					pDef->iMul = 1;
					break;
				}
				if (csTemp != "-1")
				{
					pDef->wID = (WORD)atoi(csTemp);
					pDef->iMul = iMul;
					break;
				}
				iMul++;
			}
			m_daBodyConv.Insert(pDef);
		}
	}
				
}

void CAxisXApp::LoadHues()
{
	CFile cfHues;
	CString csHueFile;
	csHueFile = GetMulPath(VERFILE_HUES);
	if ( cfHues.Open( csHueFile, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone ) )
	{
		int iBytesRead = 1;
		while ( iBytesRead )
		{
			CHueGroup * pHueGroup = new (CHueGroup);
			iBytesRead = cfHues.Read( pHueGroup, sizeof(CHueGroup) );
			if ( iBytesRead )
			{
				m_aHueGroups.Add( pHueGroup );
			}
			else
				delete pHueGroup;

		}
		cfHues.Close();
	}
	else
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_NOOPEN"), csHueFile));
}

void CAxisXApp::LoadSounds()
{
	CFile cfSoundidx;
	CString csSoundidxFile;
	csSoundidxFile = GetMulPath(VERFILE_SOUNDIDX);
	if ( cfSoundidx.Open( csSoundidxFile, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone ) )
	{
		int iBytesRead = 1;
		while ( iBytesRead )
		{
			CSoundIDX * pSound = new (CSoundIDX);
			iBytesRead = cfSoundidx.Read( pSound, 12 );
			if ( iBytesRead )
			{
				if(pSound->dwLenght != 0xFFFFFFFF)
				{
					CFile cfSoundmul;
					CString csSoundmulFile;
					char cName[33];
					csSoundmulFile = Main->GetMulPath(VERFILE_SOUND);

					if ( cfSoundmul.Open( csSoundmulFile, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone ) )
					{
						cfSoundmul.Seek(pSound->dwStart,CFile::begin);
						cfSoundmul.Read( &cName, 32 );
						cName[32] = 0x00;
						CString csName = cName;
						csName = csName.SpanExcluding(".");
						pSound->csName = csName;

						cfSoundmul.Close();
					}

					m_aSounds.Add( pSound );
				}
			}
			else
				delete pSound;

		}
		cfSoundidx.Close();
	}
	else
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_NOOPEN"), csSoundidxFile));
}

void CAxisXApp::LoadMusic()
{
	CString csMusicFile;
	csMusicFile = GetMulPath(VERFILE_MUSICLIST);
	CStdioFile * pMusicList = new CStdioFile;
	if ( pMusicList->Open(csMusicFile, CFile::modeRead | CFile::shareDenyNone) )
	{
		BOOL bStatus = TRUE;
		while ( bStatus )
		{
			CString csLine;
			bStatus = pMusicList->ReadString(csLine);

			if ( !bStatus )
				break;

			CMusicList * pMusic = new (CMusicList);

			CString csTitle = csLine.Mid(csLine.Find(" ") + 1);
			csTitle = csTitle.SpanExcluding(".,");
			pMusic->csName = csTitle;

			CString csMusicPath = GetMulPath(VERFILE_MUSICLIST);
			csMusicPath = csMusicPath.Left(csMusicPath.ReverseFind('\\')+1);
			csTitle.Format(AXT("%s\\%s.mp3"),csMusicPath,csTitle);
			pMusic->csPath = csTitle;

			pMusic->wIndex = (WORD)atoi(csLine.SpanExcluding(" "))+1;

			m_aMusic.Add( pMusic );
		}
	}
	else
		m_log.Add(1,CFMsg(CMsg("IDS_WARNING_NOOPEN"), csMusicFile));
}

void CAxisXApp::UnLoadBodyDef()
{
	for ( int i = 0; i <= m_daBodydef.GetUpperBound(); i++ )
	{
		CDefObj * pDef = (CDefObj *) m_daBodydef.GetAt(i);
		delete pDef;
	}
	m_daBodydef.RemoveAll();
}

void CAxisXApp::UnLoadBodyConvert()
{
	for ( int i = 0; i <= m_daBodyConv.GetUpperBound(); i++ )
	{
		CDefObj * pDef = (CDefObj *) m_daBodyConv.GetAt(i);
		delete pDef;
	}
	m_daBodyConv.RemoveAll();
}

void CAxisXApp::UnLoadHues()
{
	for ( int i = 0; i < m_aHueGroups.GetSize(); i++ )
	{
		CHueGroup * pHueGroup = (CHueGroup *) m_aHueGroups.GetAt(i);
		if ( pHueGroup )
			delete pHueGroup;
	}
	m_aHueGroups.RemoveAll();
}

void CAxisXApp::UnLoadSounds()
{
	for ( int i = 0; i < m_aSounds.GetSize(); i++ )
	{
		CSoundIDX * pSound = (CSoundIDX *) m_aSounds.GetAt(i);
		if ( pSound )
			delete pSound;
	}
	m_aSounds.RemoveAll();
}

void CAxisXApp::UnLoadMusic()
{
	for ( int i = 0; i < m_aMusic.GetSize(); i++ )
	{
		CMusicList * pMusic = (CMusicList *) m_aMusic.GetAt(i);
		if ( pMusic )
			delete pMusic;
	}
	m_aMusic.RemoveAll();
}

void CAxisXApp::UnLoadLog()
{
	if (!m_olLog.IsEmpty())
	{
		POSITION Pos = m_olLog.GetHeadPosition();
		while (Pos != NULL)
		{
			CLogArray * pLog = (CLogArray *) m_olLog.GetNext(Pos);
			delete (pLog);
		}
	}
	m_olLog.RemoveAll();
}

//*************************************
//Load INI File

enum INI_TYPE
{
	INI_SETTINGS,
	INI_NPCTAG,
	INI_ITEMTAG,
	INI_QTY,			// Don't care
};

const char * pIniBlocks[INI_QTY] =
{
	"SETTINGS",
	"NPCTAG",
	"ITEMTAG",
};

bool CAxisXApp::LoadIni(int iLoadPart, CString csLoadString)
{
	try
	{
		//Load Axis.ini
		CString csFile;
		csFile.Format("%sAxisX.ini",m_csRootDirectory);
		CStdioFile pFile;
		if ( !pFile.Open(csFile, CFile::modeRead | CFile::shareDenyNone) )
		{
			Main->m_log.Add(1,CFMsg(CMsg("IDS_WARNING_NOOPEN"), csFile));
			return false;
		}

		BOOL bStatus = TRUE;
		while ( bStatus )
		{
			CString csLine;
			bStatus = pFile.ReadString(csLine);

			if ( !bStatus )
				break;

			if ( csLine.Find("[") == 0 )
			{
				while ( bStatus )
				{
					if ( !bStatus )
						break;
					if ( csLine.Find("[") == 0 )
					{
						CString csKey = csLine.Mid(1);
						csKey = csKey.SpanExcluding("]");
						CString csIndex;
						if ( csKey.Find(" ") != -1 )
							csIndex = csKey.Mid(csKey.Find(" ") + 1);
						CString csType = csKey.SpanExcluding(" ");
						int resource = FindTable(csType, pIniBlocks, INI_QTY);		

						if ((iLoadPart > -1) && (resource != iLoadPart))
							break;

						switch (resource)
						{
						case INI_SETTINGS:
							{
								while ( bStatus )
								{
									bStatus = pFile.ReadString(csLine);
									if ( !bStatus )
										break;
									if ( csLine.Find("[") == 0 )
									{
										break;
									}
									csLine = csLine.SpanExcluding("//");
									csLine.Trim(); 
									if ( csLine != "" )
									{
										CString csSetting, csValue;
										csSetting = csLine.SpanExcluding(" \t=");
										csSetting.Trim();
										csValue = csLine.Mid(csSetting.GetLength() + 1);
										csValue.Trim();
										if( csValue.Find(" \t="))
											csValue = csValue.Mid(csValue.Find(" \t=") + 1);
										csValue.Trim();

										if ((csLoadString != "") && (csSetting != csLoadString))
											continue;

										if (csSetting == "AllowMultiple")
											m_dwAllowMultiple = atoi(csValue);
										if (csSetting == "AlwaysOnTop")
											m_dwAlwaysOnTop = atoi(csValue);
										if (csSetting == "DisableToolbar")
											m_dwDisableToolbar = atoi(csValue);
										if (csSetting == "DrawDifs")
											m_dwDrawDifs = atoi(csValue);
										if (csSetting == "DrawStatics")
											m_dwDrawStatics = atoi(csValue);
										if (csSetting == "LoadDefault")
											m_dwLoadDefault = atoi(csValue);
										if (csSetting == "Position")
											m_csPosition = csValue;
										if (csSetting == "RoomView")
											m_dwRoomView = atoi(csValue);
										if (csSetting == "SameAsClient")
											m_dwSameAsClient = atoi(csValue);
										if (csSetting == "StartTab")
											m_dwStartTab = atoi(csValue);
										if (csSetting == "SysClose")
											m_dwSysClose = atoi(csValue);
										if (csSetting == "ShowSpawnpoints")
											m_dwShowSpawnpoints = atoi(csValue);
										if (csSetting == "NPCSpawnColor")
											m_dwNPCSpawnColor = atoi(csValue);
										if (csSetting == "ItemSpawnColor")
											m_dwItemSpawnColor = atoi(csValue);
										// Map the default preview background to the current theme.
										if (csSetting == "ItemBGColor")
										{
											m_dwItemBGColor = atoi(csValue);
											AxisMapPreviewBg(m_dwItemBGColor);
										}
										if (csSetting == "SpawnBGColor")
										{
											m_dwSpawnBGColor = atoi(csValue);
											AxisMapPreviewBg(m_dwSpawnBGColor);
										}
										if (csSetting == "ShowItems")
											m_dwShowItems = atoi(csValue);
										if (csSetting == "ShowNPCs")
											m_dwShowNPCs = atoi(csValue);
										if (csSetting == "ShowMap")
											m_dwShowMap = atoi(csValue);
										if (csSetting == "CommandPrefix")
											m_csCommandPrefix = csValue;
										if (csSetting == "UOTitle")
											m_csUOTitle = csValue;
										if (csSetting == "RegInstallation")
										{
											if ( csValue == "Machine")
												hRegLocation = HKEY_LOCAL_MACHINE;
											else if ( csValue == "User")
												hRegLocation = HKEY_CURRENT_USER;
										}
									}
								}
							}
							break;
						case INI_NPCTAG:
							{
								CStringArray * csaList;
								if (csIndex == "Stats")
									csaList = &Main->m_pScripts->m_asaNPCStats;
								else if (csIndex == "Resistences")
									csaList = &Main->m_pScripts->m_asaNPCResistences;
								else if (csIndex == "Misc")
									csaList = &Main->m_pScripts->m_asaNPCMisc;
								else if (csIndex == "Tags")
									csaList = &Main->m_pScripts->m_asaNPCTags;
								while ( bStatus )
								{
									bStatus = pFile.ReadString(csLine);
									if ( !bStatus )
										break;
									if ( csLine.Find("[") == 0 )
									{
										break;
									}
									csLine = csLine.SpanExcluding("//");
									csLine.Trim();
									if ( csLine != "" )
									{
										csaList->Add(csLine);
									}
								}
							}
							break;
						case INI_ITEMTAG:
							{
								CStringArray * csaList;
								if (csIndex == "Props")
									csaList = &Main->m_pScripts->m_asaITEMProps;
								else if (csIndex == "Tags")
									csaList = &Main->m_pScripts->m_asaITEMTags;
								while ( bStatus )
								{
									bStatus = pFile.ReadString(csLine);
									if ( !bStatus )
										break;
									if ( csLine.Find("[") == 0 )
									{
										break;
									}
									csLine = csLine.SpanExcluding("//");
									csLine.Trim();
									if ( csLine != "" )
									{
										csaList->Add(csLine);
									}
								}

							}
							break;
						default:
							Main->m_log.Add(1,CFMsg(CMsg("IDS_WARNING_UNKNOWN_SECTION"), csLine, csFile));
							bStatus = pFile.ReadString(csLine);
							break;
						}
					}
					else
					{
						bStatus = pFile.ReadString(csLine);
					}
				}
			}
		}
	}
	catch (CFileException *e)
	{
		Main->m_log.Add(1,CFMsg(CMsg("IDS_WARNING_EXCEPTION_FILE"), e->m_strFileName, e->m_cause));
		e->Delete();
	}
	return true;
}

bool CAxisXApp::LoadLang()
{
	CString csFile;
	// Language "deu" or "eng" (default). Runs before LoadIni, so hRegLocation may be unset:
	// read the user hive first, then the machine hive.
	CString csLanguage = GetRegistryString("Language", "", hRegLocation ? hRegLocation : HKEY_CURRENT_USER);
	if (csLanguage.IsEmpty() && !hRegLocation)
		csLanguage = GetRegistryString("Language", "", HKEY_LOCAL_MACHINE);
	if (csLanguage != "deu" && csLanguage != "eng")
		csLanguage = "eng";
	csFile.Format("%sLanguage\\%s.lng",m_csRootDirectory,csLanguage);
	CIni lng(csFile);
	lng.GetKeyArray(_T("LANGUAGE"), &pLng);
	AxisLoadTranslations(csLanguage);
	return true;
}