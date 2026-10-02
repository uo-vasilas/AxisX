/*

 **********************************************************************
 *
 * Axis X - update check against the GitHub releases.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 **********************************************************************

*/

#include "stdafx.h"
#include "AxisX.h"
#include "Updater.h"
#include <winhttp.h>
#include <bcrypt.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "bcrypt.lib")

#define AXIS_RELEASES_API	L"https://api.github.com/repos/uo-vasilas/AxisX/releases/latest"

AxisUpdateInfo g_axisUpdate;
static volatile LONG s_lBusy = 0;	// one check or download at a time

// Splits "v1.2.3" into up to four numbers.
static void ParseVersion(const CString& csVersion, int aParts[4])
{
	for (int i = 0; i < 4; i++)
		aParts[i] = 0;
	int iPart = 0;
	bool bInNumber = false;
	for (int i = 0; i < csVersion.GetLength() && iPart < 4; i++)
	{
		TCHAR c = csVersion[i];
		if (c >= '0' && c <= '9')
		{
			aParts[iPart] = aParts[iPart] * 10 + (c - '0');
			bInNumber = true;
		}
		else if (bInNumber)
		{
			iPart++;
			bInNumber = false;
		}
	}
}

static bool IsNewer(const CString& csRemote, const CString& csLocal)
{
	int aRemote[4], aLocal[4];
	ParseVersion(csRemote, aRemote);
	ParseVersion(csLocal, aLocal);
	for (int i = 0; i < 4; i++)
		if (aRemote[i] != aLocal[i])
			return aRemote[i] > aLocal[i];
	return false;
}

// Version of the running AxisX.exe, e.g. "1.0" or "1.0.2".
CString AxisCurrentVersion()
{
	TCHAR szExe[MAX_PATH];
	GetModuleFileName(NULL, szExe, MAX_PATH);
	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize(szExe, &dwHandle);
	CString csVersion = _T("0.0");
	if (dwSize == 0)
		return csVersion;
	CByteArray aBuffer;
	aBuffer.SetSize(dwSize);
	VS_FIXEDFILEINFO* pInfo = NULL;
	UINT uLen = 0;
	if (GetFileVersionInfo(szExe, dwHandle, dwSize, aBuffer.GetData())
		&& VerQueryValue(aBuffer.GetData(), _T("\\"), (LPVOID*) &pInfo, &uLen) && pInfo != NULL)
	{
		int a = HIWORD(pInfo->dwFileVersionMS), b = LOWORD(pInfo->dwFileVersionMS);
		int c = HIWORD(pInfo->dwFileVersionLS), d = LOWORD(pInfo->dwFileVersionLS);
		if (d != 0)
			csVersion.Format(_T("%d.%d.%d.%d"), a, b, c, d);
		else if (c != 0)
			csVersion.Format(_T("%d.%d.%d"), a, b, c);
		else
			csVersion.Format(_T("%d.%d"), a, b);
	}
	return csVersion;
}

// True if the check is enabled; it runs on every start.
bool AxisUpdateCheckDue()
{
	return Main->GetRegistryDword(_T("CheckUpdates"), 1) != 0;
}

// Reads the string value of "key" between iFrom and iTo (-1 = end).
// Returns the position after the value, or -1.
static int JsonString(const CStringA& json, LPCSTR pszKey, int iFrom, int iTo, CStringA& csOut)
{
	CStringA csKey;
	csKey.Format("\"%s\"", pszKey);
	int iPos = json.Find(csKey, iFrom);
	if (iPos < 0 || (iTo >= 0 && iPos >= iTo))
		return -1;
	iPos += csKey.GetLength();
	while (iPos < json.GetLength() && isspace((unsigned char) json[iPos]))
		iPos++;
	if (iPos >= json.GetLength() || json[iPos] != ':')
		return -1;
	iPos++;
	while (iPos < json.GetLength() && isspace((unsigned char) json[iPos]))
		iPos++;
	if (iPos >= json.GetLength() || json[iPos] != '"')
		return -1;
	iPos++;
	csOut.Empty();
	while (iPos < json.GetLength() && json[iPos] != '"')
	{
		if (json[iPos] == '\\' && iPos + 1 < json.GetLength())
			iPos++;
		csOut += json[iPos];
		iPos++;
	}
	return iPos + 1;
}

// GET over HTTPS. Stores the body in pBody, or streams it into hFile while
// hashing it (SHA-256) and posting progress to hNotify.
static bool HttpGet(LPCWSTR pszUrl, CStringA* pBody, HANDLE hFile, BCRYPT_HASH_HANDLE hHash, HWND hNotify, CString& csError)
{
	// URL_COMPONENTSW explicitly: stdafx pulls in wininet.h, whose ANSI
	// URL_COMPONENTS shadows the WinHTTP one.
	URL_COMPONENTSW uc;
	memset(&uc, 0, sizeof(uc));
	uc.dwStructSize = sizeof(uc);
	WCHAR szHost[256], szPath[2048];
	uc.lpszHostName = szHost;
	uc.dwHostNameLength = 256;
	uc.lpszUrlPath = szPath;
	uc.dwUrlPathLength = 2048;
	WCHAR szExtra[1024];
	uc.lpszExtraInfo = szExtra;
	uc.dwExtraInfoLength = 1024;
	if (_wcsnicmp(pszUrl, L"https://", 8) != 0 || !WinHttpCrackUrl(pszUrl, 0, 0, (LPURL_COMPONENTS) &uc))
	{
		csError = _T("invalid URL");
		return false;
	}
	CStringW csPath = CStringW(szPath) + szExtra;

	bool bOk = false;
	HINTERNET hSession = WinHttpOpen(L"AxisX-Updater", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	HINTERNET hConnect = hSession ? WinHttpConnect(hSession, szHost, uc.nPort, 0) : NULL;
	HINTERNET hRequest = hConnect ? WinHttpOpenRequest(hConnect, L"GET", csPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : NULL;
	if (hRequest)
	{
		WinHttpSetTimeouts(hRequest, 10000, 10000, 15000, 30000);
		LPCWSTR pszHeaders = pBody ? L"Accept: application/vnd.github+json\r\n" : WINHTTP_NO_ADDITIONAL_HEADERS;
		if (WinHttpSendRequest(hRequest, pszHeaders, (DWORD) -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
			&& WinHttpReceiveResponse(hRequest, NULL))
		{
			DWORD dwStatus = 0, dwLen = sizeof(dwStatus);
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatus, &dwLen, NULL);
			if (dwStatus == 200)
			{
				DWORD dwTotal = 0;
				dwLen = sizeof(dwTotal);
				WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwTotal, &dwLen, NULL);
				DWORD dwDone = 0;
				int iLastPercent = -1;
				char buffer[16384];
				bOk = true;
				for (;;)
				{
					DWORD dwRead = 0;
					if (!WinHttpReadData(hRequest, buffer, sizeof(buffer), &dwRead))
					{
						bOk = false;
						csError = _T("connection lost");
						break;
					}
					if (dwRead == 0)
						break;
					if (pBody)
						pBody->Append(buffer, (int) dwRead);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						DWORD dwWritten = 0;
						if (!WriteFile(hFile, buffer, dwRead, &dwWritten, NULL) || dwWritten != dwRead)
						{
							bOk = false;
							csError = _T("cannot write the file");
							break;
						}
					}
					if (hHash)
						BCryptHashData(hHash, (PUCHAR) buffer, dwRead, 0);
					dwDone += dwRead;
					if (hNotify && dwTotal > 0)
					{
						int iPercent = (int) ((ULONGLONG) dwDone * 100 / dwTotal);
						if (iPercent != iLastPercent && iPercent <= 100)
						{
							iLastPercent = iPercent;
							::PostMessage(hNotify, WM_AXIS_UPDATE_DOWNLOAD, (WPARAM) iPercent, 0);
						}
					}
				}
			}
			else
				csError.Format(_T("HTTP %lu"), dwStatus);
		}
		else
			csError.Format(_T("no connection (%lu)"), GetLastError());
	}
	else
		csError.Format(_T("no connection (%lu)"), GetLastError());

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return bOk;
}

struct UpdateThreadArgs { HWND hNotify; bool bUser; };

static DWORD WINAPI UpdateCheckThread(LPVOID pParam)
{
	UpdateThreadArgs* pArgs = (UpdateThreadArgs*) pParam;
	HWND hNotify = pArgs->hNotify;
	LPARAM lUser = pArgs->bUser ? 1 : 0;
	delete pArgs;

	CStringA json;
	CString csError;
	WPARAM wResult = 2;
	if (HttpGet(AXIS_RELEASES_API, &json, INVALID_HANDLE_VALUE, NULL, NULL, csError))
	{
		CStringA csTag, csPage;
		JsonString(json, "tag_name", 0, -1, csTag);
		JsonString(json, "html_url", 0, -1, csPage);

		// First installer asset (*Setup*.exe) and its digest.
		CStringA csUrl, csDigest;
		int iCursor = json.Find("\"assets\"");
		while (iCursor >= 0)
		{
			int iKey = json.Find("\"browser_download_url\"", iCursor);
			if (iKey < 0)
				break;
			CStringA csCandidate, csCandidateDigest;
			JsonString(json, "digest", iCursor, iKey, csCandidateDigest);
			int iNext = JsonString(json, "browser_download_url", iKey, -1, csCandidate);
			if (iNext < 0)
				break;
			CStringA csLower = csCandidate;
			csLower.MakeLower();
			if (csLower.Right(4) == ".exe" && csLower.Find("setup") >= 0)
			{
				csUrl = csCandidate;
				csDigest = csCandidateDigest;
				break;
			}
			iCursor = iNext;
		}

		if (csTag.IsEmpty())
			csError = _T("no release found");
		else
		{
			CString csVersion(csTag);
			csVersion.TrimLeft(_T("vV"));
			g_axisUpdate.csVersion = csVersion;
			g_axisUpdate.csPageUrl = csPage.IsEmpty() ? CString(AXIS_RELEASES_PAGE) : CString(csPage);
			g_axisUpdate.csSetupUrl = CString(csUrl);
			CString csSha(csDigest);
			csSha.MakeLower();
			g_axisUpdate.csSha256 = (csSha.Left(7) == _T("sha256:")) ? csSha.Mid(7) : CString();
			g_axisUpdate.bAvailable = IsNewer(csVersion, AxisCurrentVersion());
			wResult = g_axisUpdate.bAvailable ? 1 : 0;
		}
	}
	g_axisUpdate.csError = csError;
	InterlockedExchange(&s_lBusy, 0);
	::PostMessage(hNotify, WM_AXIS_UPDATE_CHECKED, wResult, lUser);
	return 0;
}

// Starts the release check in the background; the result is posted to hNotify.
void AxisStartUpdateCheck(HWND hNotify, bool bUser)
{
	if (InterlockedCompareExchange(&s_lBusy, 1, 0) != 0)
		return;
	Main->PutRegistryString(_T("LastUpdateCheck"), CTime::GetCurrentTime().Format(_T("%Y%m%d")));
	UpdateThreadArgs* pArgs = new UpdateThreadArgs;
	pArgs->hNotify = hNotify;
	pArgs->bUser = bUser;
	HANDLE hThread = CreateThread(NULL, 0, UpdateCheckThread, pArgs, 0, NULL);
	if (hThread)
		CloseHandle(hThread);
	else
	{
		delete pArgs;
		InterlockedExchange(&s_lBusy, 0);
	}
}

static DWORD WINAPI UpdateDownloadThread(LPVOID pParam)
{
	HWND hNotify = (HWND) pParam;
	CString csError;
	bool bOk = false;

	TCHAR szTemp[MAX_PATH];
	GetTempPath(MAX_PATH, szTemp);
	CString csFile;
	csFile.Format(_T("%sAxisX_Setup_%s.exe"), szTemp, (LPCTSTR) g_axisUpdate.csVersion);

	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_HASH_HANDLE hHash = NULL;
	BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
	if (hAlg)
		BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0);

	HANDLE hFile = CreateFile(csFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE || hHash == NULL)
		csError = _T("cannot create the file");
	else
	{
		bOk = HttpGet(CStringW(g_axisUpdate.csSetupUrl), NULL, hFile, hHash, hNotify, csError);
		CloseHandle(hFile);
		if (bOk)
		{
			// The installer only runs if it matches the digest GitHub reports for it.
			UCHAR digest[32];
			BCryptFinishHash(hHash, digest, sizeof(digest), 0);
			CString csHex;
			for (int i = 0; i < 32; i++)
				csHex.AppendFormat(_T("%02x"), digest[i]);
			if (g_axisUpdate.csSha256.IsEmpty() || csHex != g_axisUpdate.csSha256)
			{
				bOk = false;
				csError = _T("checksum mismatch");
			}
		}
		if (!bOk)
			DeleteFile(csFile);
	}
	if (hHash) BCryptDestroyHash(hHash);
	if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);

	g_axisUpdate.csSetupFile = bOk ? csFile : CString();
	g_axisUpdate.csError = csError;
	InterlockedExchange(&s_lBusy, 0);
	::PostMessage(hNotify, WM_AXIS_UPDATE_DOWNLOAD, bOk ? 101 : 102, 0);
	return 0;
}

// Downloads the installer of g_axisUpdate in the background.
void AxisStartUpdateDownload(HWND hNotify)
{
	if (g_axisUpdate.csSetupUrl.IsEmpty() || InterlockedCompareExchange(&s_lBusy, 1, 0) != 0)
		return;
	HANDLE hThread = CreateThread(NULL, 0, UpdateDownloadThread, (LPVOID) hNotify, 0, NULL);
	if (hThread)
		CloseHandle(hThread);
	else
		InterlockedExchange(&s_lBusy, 0);
}

// Starts the downloaded installer. False if it could not be started
// (e.g. the elevation prompt was declined).
bool AxisRunUpdateInstaller()
{
	if (g_axisUpdate.csSetupFile.IsEmpty())
		return false;
	HINSTANCE hResult = ShellExecute(NULL, _T("open"), g_axisUpdate.csSetupFile, NULL, NULL, SW_SHOWNORMAL);
	return (INT_PTR) hResult > 32;
}
