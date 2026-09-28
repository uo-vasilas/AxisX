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

#pragma once

// Posted to the notify window when a check finishes.
// wParam: 0 = up to date, 1 = update available, 2 = check failed. lParam: 1 = started by the user.
#define WM_AXIS_UPDATE_CHECKED	(WM_APP + 94)
// Posted while the installer downloads. wParam: percent 0-100, 101 = done, 102 = failed.
#define WM_AXIS_UPDATE_DOWNLOAD	(WM_APP + 93)

#define AXIS_RELEASES_PAGE	_T("https://github.com/uo-vasilas/AxisX/releases")

// Result of the last check/download. Written by the worker thread before it
// posts its message, read by the UI thread afterwards.
struct AxisUpdateInfo
{
	CString csVersion;		// newest release, e.g. "1.1"
	CString csSetupUrl;		// installer asset
	CString csSha256;		// asset digest from GitHub (lowercase hex)
	CString csPageUrl;		// release page
	CString csSetupFile;	// downloaded installer
	CString csError;
	bool bAvailable;		// newer than the running version
};
extern AxisUpdateInfo g_axisUpdate;

CString AxisCurrentVersion();
bool AxisUpdateCheckDue();
void AxisStartUpdateCheck(HWND hNotify, bool bUser);
void AxisStartUpdateDownload(HWND hNotify);
bool AxisRunUpdateInstaller();
