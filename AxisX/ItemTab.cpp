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

// ItemTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "ItemTab.h"
#include "UOart.h"
#include "Common.h"
#include "SearchCritDlg.h"
#include "MultiView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemTab property page

IMPLEMENT_DYNCREATE(CItemTab, CDockingPage)

int CItemTab::iIDSort = 1;
int CItemTab::iNameSort = 1;

CItemTab::CItemTab() : CDockingPage(CItemTab::IDD,CMsg("IDS_ITEMS"))
{
	//{{AFX_DATA_INIT(CItemTab)
	m_bLockDown = FALSE;
	m_iCatSeq = 0;
	m_bIgnoreSearchChange = false;
	icX = 60;
	icY = 110;
	//}}AFX_DATA_INIT
}

CItemTab::~CItemTab()
{
}

void CItemTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CItemTab)
	DDX_Control(pDX, IDC_NUDGEUP, m_cbNudgeUp);
	DDX_Control(pDX, IDC_NUDGEDOWN, m_cbNudgeDown);
	DDX_Control(pDX, IDC_ITEMS, m_clcItems);
	DDX_Control(pDX, IDC_CATEGORY_TREE, m_ctcCategories);
	DDX_Control(pDX, IDC_ITEMID, m_csItemID);
	DDX_Control(pDX, IDC_ITEMIDDEC, m_csItemIDDec);
	DDX_Control(pDX, IDC_LOCKITEM, m_cbLockDown);
	DDX_Control(pDX, IDC_DISPLAY, m_Display);
	DDX_Control(pDX, IDC_ZTILE, m_ceZTile);
	DDX_Control(pDX, IDC_NUKEARG, m_ceNukearg);
	DDX_Control(pDX, IDC_MINTIME, m_ceMinTime);
	DDX_Control(pDX, IDC_MAXTIME, m_ceMaxTime);
	DDX_Control(pDX, IDC_MAXDIST, m_ceMaxDist);
	DDX_Control(pDX, IDC_AMOUNT, m_ceAmount);
	DDX_Control(pDX, IDC_SPAWNRATE, m_ceSpawnRate);
	DDX_Control(pDX, IDC_NUDGEAMOUNT, m_ceNudge);
	DDX_Control(pDX, IDC_MOVE1, m_cbMove1);
	DDX_Control(pDX, IDC_MOVE2, m_cbMove2);
	DDX_Control(pDX, IDC_MOVE3, m_cbMove3);
	DDX_Control(pDX, IDC_MOVE4, m_cbMove4);
	DDX_Control(pDX, IDC_MOVE5, m_cbMove5);
	DDX_Control(pDX, IDC_MOVE6, m_cbMove6);
	DDX_Control(pDX, IDC_MOVE7, m_cbMove7);
	DDX_Control(pDX, IDC_MOVE8, m_cbMove8);
	DDX_Check(pDX, IDC_LOCKITEM, m_bLockDown);
	DDX_Control(pDX, IDC_FINDITEM, cb_finditem);
	DDX_Control(pDX, IDC_ITEMSEARCH, m_ceSearch);
	DDX_Control(pDX, IDC_ITEM_DETAILS, m_csDetails);
	DDX_Control(pDX, IDC_ITEM_RESULTINFO, m_csResultInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CItemTab, CDockingPage)
	//{{AFX_MSG_MAP(CItemTab)
	ON_NOTIFY(TVN_SELCHANGED, IDC_CATEGORY_TREE, OnSelchangedCategoryTree)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ITEMS, OnItemchangedItems)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ITEMS, OnColumnclickItems)
	ON_NOTIFY(NM_DBLCLK, IDC_ITEMS, OnDblclkItems)
	ON_NOTIFY(NM_RCLICK, IDC_ITEMS, OnRclickItems)
	ON_BN_CLICKED(IDC_CREATE, OnCreate)
	ON_BN_CLICKED(IDC_LOCKITEM, OnLockitem)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_TILE, OnTile)
	ON_BN_CLICKED(IDC_FLIP, OnFlip)
	ON_BN_CLICKED(IDC_PLACESPAWN, OnPlacespawn)
	ON_BN_CLICKED(IDC_INITSPAWN, OnInitspawn)
	ON_BN_CLICKED(IDC_NUKE, OnNuke)
	ON_BN_CLICKED(IDC_NUDGEUP, OnNudgeup)
	ON_BN_CLICKED(IDC_NUDGEDOWN, OnNudgedown)
	ON_BN_CLICKED(IDC_MOVE1, OnMove1)
	ON_BN_CLICKED(IDC_MOVE2, OnMove2)
	ON_BN_CLICKED(IDC_MOVE3, OnMove3)
	ON_BN_CLICKED(IDC_MOVE4, OnMove4)
	ON_BN_CLICKED(IDC_MOVE5, OnMove5)
	ON_BN_CLICKED(IDC_MOVE6, OnMove6)
	ON_BN_CLICKED(IDC_MOVE7, OnMove7)
	ON_BN_CLICKED(IDC_MOVE8, OnMove8)
	ON_EN_CHANGE(IDC_MAXTIME, OnChangeParams)
	ON_EN_CHANGE(IDC_MINTIME, OnChangeParams)
	ON_EN_CHANGE(IDC_SPAWNRATE, OnChangeParams)
	ON_EN_CHANGE(IDC_AMOUNT, OnChangeParams)
	ON_EN_CHANGE(IDC_NUDGEAMOUNT, OnChangeParams)
	ON_EN_CHANGE(IDC_ZTILE, OnChangeParams)
	ON_EN_CHANGE(IDC_MAXDIST, OnChangeParams)
	ON_BN_CLICKED(IDC_FINDITEM, OnFinditem)
	ON_BN_CLICKED(IDC_CREATESTATIC, OnCreateStatic)
	ON_EN_CHANGE(IDC_ITEMSEARCH, OnChangeSearch)
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemTab message handlers

BOOL CItemTab::OnInitDialog() 
{
	m_dcPropertyPage = Main->m_pcppItemTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CItemTab;
	Main->m_pcppItemTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();
	AxisMarkPrimary(this, IDC_CREATE);	// Primary action

	CString csValue;
	DWORD dwValue;
	dwValue = Main->GetRegistryDword("LockDownItems", 0);
	m_cbLockDown.SetCheck(dwValue);
	csValue = Main->GetRegistryString("ItemSpawnMaxAmount", "1");
	m_ceAmount.SetWindowText(csValue);
	csValue = Main->GetRegistryString("ItemSpawnMaxTime", "240");
	m_ceMaxTime.SetWindowText(csValue);
	csValue = Main->GetRegistryString("ItemSpawnMinTime", "30");
	m_ceMinTime.SetWindowText(csValue);
	csValue = Main->GetRegistryString("ItemSpawnRate", "0");
	m_ceSpawnRate.SetWindowText(csValue);
	csValue = Main->GetRegistryString("ItemSpawnMaxDist", "0");
	m_ceMaxDist.SetWindowText(csValue);
	csValue = Main->GetRegistryString("ItemNudgeAmount", "1");
	m_ceNudge.SetWindowText(csValue);
	csValue = Main->GetRegistryString("ItemTileZ", "0");
	m_ceZTile.SetWindowText(csValue);

	// Weight/flags columns come from tiledata.mul (see SetTiledataColumns()).
	this->m_clcItems.InsertColumn(0, AXT("Name"), LVCFMT_LEFT, 140, -1);
	this->m_clcItems.InsertColumn(1, AXT("Defname"), LVCFMT_LEFT, 120, -1);
	this->m_clcItems.InsertColumn(2, AXT("Grafik"), LVCFMT_LEFT, 56, -1);
	this->m_clcItems.InsertColumn(3, AXT("Kategorie"), LVCFMT_LEFT, 120, -1);
	this->m_clcItems.InsertColumn(4, AXT("Gewicht"), LVCFMT_RIGHT, 50, -1);
	this->m_clcItems.InsertColumn(5, AXT("Eigenschaften"), LVCFMT_LEFT, 110, -1);
	m_clcItems.SetExtendedStyle(m_clcItems.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	ApplyAxisListHeader(m_clcItems.GetSafeHwnd());	// Dark column header
	AxisSetCue(m_ceSearch.GetSafeHwnd(), "z.B.  schwert   0x13b9   i_katana   waffen axt");
	AxisSetCue(m_ceNukearg.GetSafeHwnd(), "optional: Befehl, z.B. color 0x21");
	m_csResultInfo.SetWindowText(AXT("Kategorie links w\xE4hlen oder oben suchen"));
	FillCategoryTree();

	m_pImageList = new CImageList();
	m_pImageList->Create(icX, icY, ILC_COLORDDB, 0, 1);
	m_clcItems.SetImageList(m_pImageList, LVSIL_NORMAL);
	m_iCurState = m_clcItems.GetView();

	WORD wFlags = 0;
	if ( Main->m_dwRoomView )
		wFlags |= F_ROOMVIEW;
	m_Display.SetDrawFlags(wFlags);
	m_Display.SetArtType(1);
	m_Display.SetArtIndex(-1);
	m_Display.SetBkColor(Main->m_dwItemBGColor);

	// Tooltips show the Sphere command behind each button.
	m_tipItems.Create(this);
	m_tipItems.SetMaxTipWidth(360);
	m_tipItems.AddTool(GetDlgItem(IDC_CREATE), AXT("add <defname> - Item erstellen, dann Ort im Spiel anklicken"));
	m_tipItems.AddTool(GetDlgItem(IDC_CREATESTATIC), AXT("static <defname> - als festes Welt-Static setzen"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE1), AXT("xmove 0 -n (Norden)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE2), AXT("xmove n -n (Nordosten)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE3), AXT("xmove n 0 (Osten)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE4), AXT("xmove n n (Suedosten)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE5), AXT("xmove 0 n (Sueden)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE6), AXT("xmove -n n (Suedwesten)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE7), AXT("xmove -n 0 (Westen)"));
	m_tipItems.AddTool(GetDlgItem(IDC_MOVE8), AXT("xmove -n -n (Nordwesten)"));
	m_tipItems.AddTool(GetDlgItem(IDC_NUDGEUP), AXT("nudgeup n"));
	m_tipItems.AddTool(GetDlgItem(IDC_NUDGEDOWN), AXT("nudgedown n"));
	m_tipItems.AddTool(GetDlgItem(IDC_FLIP), AXT("xflip"));
	m_tipItems.AddTool(GetDlgItem(IDC_TILE), AXT("tile <z> <defname> - Flaeche mit dem Item auslegen"));
	m_tipItems.AddTool(GetDlgItem(IDC_PLACESPAWN), AXT("add 01ea7 - Spawnstein setzen"));
	m_tipItems.AddTool(GetDlgItem(IDC_INITSPAWN), AXT("act.type/amount/more/more2/morep/attr/timer auf den zuletzt gesetzten Spawnstein"));
	m_tipItems.AddTool(GetDlgItem(IDC_REMOVE), AXT("remove - Ziel anklicken"));
	m_tipItems.AddTool(GetDlgItem(IDC_NUKE), AXT("nuke [befehl] - zwei Ecken im Spiel anklicken"));
	m_tipItems.Activate(TRUE);

	//m_Display.SetArtType(1);
	//m_Display.SetArtIndex(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CItemTab::OnCreate() 
{
	CString csValue;
	this->m_csItemID.GetWindowText(csValue);
	if ( csValue == "" )
	{
		AxisSetStatus(AXT("Erst ein Item in der Liste w\xE4hlen."), 2);
		return;
	}
	CString csCmd;
	csCmd.Format("%sadd %s", Main->m_csCommandPrefix, csValue);
	if (SendToUO(csCmd))
	{
		// For the dashboard's recent list
		int iSel = m_clcItems.GetNextItem(-1, LVNI_SELECTED);
		CSObject * pObject = (iSel != -1) ? (CSObject *) m_clcItems.GetItemData(iSel) : NULL;
		AxisRememberRecent(pObject ? pObject->m_csDescription + " (Item)" : csValue, csCmd);
	}
}

void CItemTab::OnCreateStatic()
{
	CString csValue;
	this->m_csItemID.GetWindowText(csValue);
	if ( csValue == "" )
	{
		AxisSetStatus(AXT("Erst ein Item in der Liste w\xE4hlen."), 2);
		return;
	}
	CString csCmd;
	csCmd.Format("%sstatic %s", Main->m_csCommandPrefix, csValue);
	SendToUO(csCmd);
}

void CItemTab::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ClientToScreen(&point);
	CRect rect;
	m_Display.GetWindowRect(&rect);
	if ( rect.PtInRect(point) )
	{
		if (m_Display.GetArtType() == 0)
		{
			HMENU hMenu = ::CreatePopupMenu();
			if (NULL != hMenu)
				::AppendMenu(hMenu, MF_STRING, 1, AXT("Open Multi Viewer"));

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
					if (Main->m_dlgMultiView)
						delete Main->m_dlgMultiView;
					Main->m_dlgMultiView = new CMultiView;
					Main->m_dlgMultiView->l_ArtIndex = m_Display.GetArtIndex();
					Main->m_dlgMultiView->Create(IDD_MULTIVIEW);
				}
			}
			::DestroyMenu(hMenu);
		}
	}
	else
	{
		ScreenToClient(&point);
		CDockingPage::OnRButtonDown(nFlags, point);
	}
}


void CItemTab::OnLockitem() 
{
	UpdateData();
	Main->PutRegistryDword("LockDownItems", m_bLockDown);
}

void CItemTab::OnRemove() 
{
	CString csCmd;
	csCmd.Format("%sremove", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CItemTab::OnTile() 
{
	CString csValue, csID;
	this->m_csItemID.GetWindowText(csID);
	if (csID == "")
	{
		AfxMessageBox(AXT("No item is selected."), MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	this->m_ceZTile.GetWindowText(csValue);
	if (csValue.SpanIncluding("0123456789-") != csValue)
	{
		AfxMessageBox(AXT("Invalid value in the z-level field."), MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	if (csValue == "")
		csValue = "0";
	CString csCmd;
	csCmd.Format("%stile %s %s", Main->m_csCommandPrefix, csValue, csID);
	SendToUO(csCmd);
}

void CItemTab::OnFlip() 
{
	CString csCmd;
	csCmd.Format("%sxflip", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CItemTab::OnPlacespawn() 
{
	CString csCmd;
	csCmd.Format("%sadd 01ea7", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CItemTab::OnNuke() 
{
	CString csNukeArg, csCmd;
	this->m_ceNukearg.GetWindowText(csNukeArg);
	csCmd.Format("%snuke %s", Main->m_csCommandPrefix, csNukeArg);
	SendToUO(csCmd);
}

void CItemTab::OnNudgeup() 
{
	CString csAmount, csCmd;
	this->m_ceNudge.GetWindowText(csAmount);
	csCmd.Format("%snudgeup %s", Main->m_csCommandPrefix, csAmount);
	SendToUO(csCmd);
}

void CItemTab::OnNudgedown() 
{
	CString csAmount, csCmd;
	this->m_ceNudge.GetWindowText(csAmount);
	csCmd.Format("%snudgedown %s", Main->m_csCommandPrefix, csAmount);
	SendToUO(csCmd);
}

void CItemTab::OnMove1()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove 0 -%s", Main->m_csCommandPrefix, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove2()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove %s -%s", Main->m_csCommandPrefix, csMove, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove3()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove %s 0", Main->m_csCommandPrefix, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove4()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove %s %s", Main->m_csCommandPrefix, csMove, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove5()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove 0 %s", Main->m_csCommandPrefix, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove6()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove -%s %s", Main->m_csCommandPrefix, csMove, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove7()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove -%s 0", Main->m_csCommandPrefix, csMove);
	SendToUO(csCmd);
}

void CItemTab::OnMove8()
{
	CString csMove, csCmd;
	this->m_ceNudge.GetWindowText(csMove);
	if (csMove.Trim().IsEmpty())
		csMove = "1";
	csCmd.Format("%sxmove -%s -%s", Main->m_csCommandPrefix, csMove, csMove);
	SendToUO(csCmd);
}


void CItemTab::OnChangeParams() 
{
	UpdateData();
	if ( !this->IsWindowVisible() )
		return;

	CString csAmount, csMinTime, csMaxTime, csRate, csDist, csZHeight, csNudge;

	m_ceAmount.GetWindowText(csAmount);
	m_ceMaxDist.GetWindowText(csDist);
	m_ceMaxTime.GetWindowText(csMaxTime);
	m_ceMinTime.GetWindowText(csMinTime);
	m_ceNudge.GetWindowText(csNudge);
	m_ceSpawnRate.GetWindowText(csRate);
	m_ceZTile.GetWindowText(csZHeight);


	Main->PutRegistryString("ItemSpawnMaxAmount", csAmount);
	Main->PutRegistryString("ItemSpawnMaxTime", csMaxTime);
	Main->PutRegistryString("ItemSpawnMinTime", csMinTime);
	Main->PutRegistryString("ItemSpawnMaxDist", csDist);
	Main->PutRegistryString("ItemSpawnRate", csRate);
	Main->PutRegistryString("ItemNudgeAmount", csNudge);
	Main->PutRegistryString("ItemTileZ", csZHeight);

}

void CItemTab::OnInitspawn() 
{
	CWaitCursor hourglass;
	CString csAmount, csMinTime, csMaxTime, csMaxDist, csID, csSpawnRate;
	this->m_ceAmount.GetWindowText(csAmount);
	this->m_ceMaxDist.GetWindowText(csMaxDist);
	this->m_ceMinTime.GetWindowText(csMinTime);
	this->m_ceMaxTime.GetWindowText(csMaxTime);
	this->m_csItemID.GetWindowText(csID);
	this->m_ceSpawnRate.GetWindowText(csSpawnRate);

	if (csID == "")
	{
		AfxMessageBox(AXT("No item is selected."), MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	int iAmount, iMinTime, iMaxTime, iMaxDist;
	iAmount = atoi(csAmount);
	if (iAmount == 0)
		iAmount = 1;
	iMaxDist = atoi(csMaxDist);
	iMinTime = atoi(csMinTime);
	iMaxTime = atoi(csMaxTime);
	if (iMaxTime <= iMinTime)
		iMaxTime = iMinTime + 1;

	CString csCmd;
	csCmd.Format("%sact.type %ld", Main->m_csCommandPrefix, ITEM_SPAWN_ITEM);
	SendToUO(csCmd);

	csCmd.Format("%sact.amount %ld", Main->m_csCommandPrefix, iAmount);
	SendToUO(csCmd);
	Sleep(SPAWN_MESSAGE_DELAY);

	csCmd.Format("%sact.more %s", Main->m_csCommandPrefix, csID);
	SendToUO(csCmd);

	csCmd.Format("%sact.more2 %s", Main->m_csCommandPrefix, csSpawnRate);
	SendToUO(csCmd);

	csCmd.Format("%sact.morep %ld %ld %ld", Main->m_csCommandPrefix, iMinTime, iMaxTime, iMaxDist);
	SendToUO(csCmd);

	csCmd.Format("%sact.attr %04x", Main->m_csCommandPrefix, ATTR_INVIS | ATTR_MAGIC | ATTR_MOVE_NEVER);
	SendToUO(csCmd);

	csCmd.Format("%sact.timer 1", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

//***************************************************

// Short, readable summary of the most useful tiledata flags.
static CString TiledataFlagText(DWORD dwFlags)
{
	static const struct { DWORD dwFlag; LPCTSTR pszName; } aFlags[] =
	{
		{ 0x00200000, _T("Beh\xE4lter") },
		{ 0x00400000, _T("tragbar") },
		{ 0x20000000, _T("T\xFCr") },
		{ 0x00800000, _T("Lichtquelle") },
		{ 0x01000000, _T("animiert") },
		{ 0x00000800, _T("stapelbar") },
		{ 0x00000040, _T("blockiert") },
		{ 0x00000200, _T("begehbar") },
		{ 0x00000002, _T("Waffe") },
		{ 0x00000080, _T("Wasser") },
	};
	CString csText;
	for ( int i = 0; i < (int)(sizeof(aFlags) / sizeof(aFlags[0])); i++ )
	{
		if ( dwFlags & aFlags[i].dwFlag )
		{
			if ( !csText.IsEmpty() )
				csText += _T(", ");
			csText += AxisTr(aFlags[i].pszName);
		}
	}
	return csText;
}

// Fills the graphic, weight and flags columns of one list row from tiledata.
static void SetTiledataColumns(CListCtrl & list, CUOArt & art, int iRow, CSObject * pObject, WORD wID)
{
	// Columns: 2 = graphic, 4 = weight, 5 = flags (3 = category is set by FillList)
	if ( wID != (WORD) -1 )
	{
		CString csGraphic;
		csGraphic.Format(_T("0x%04X"), wID);
		list.SetItemText(iRow, 2, csGraphic);
	}
	if ( pObject->m_bType == TYPE_MULTI || wID == (WORD) -1 ) // multi ids don't index item tiledata
		return;
	CUOArt::CItemTileInfo info;
	if ( !art.GetItemTileInfo(wID, info) )
		return;
	CString csValue;
	if ( info.bWeight == 255 ) // 255 = immovable
		list.SetItemText(iRow, 4, AXT("fest"));
	else
	{
		csValue.Format(_T("%u"), info.bWeight);
		list.SetItemText(iRow, 4, csValue);
	}
	list.SetItemText(iRow, 5, TiledataFlagText(info.dwFlags));
}

void CItemTab::FillCategoryTree()
{
	HTREEITEM CategoryParent;
	HTREEITEM SubsectionParent;
	TV_INSERTSTRUCT InsertItem;

	m_ctcCategories.SetRedraw(FALSE); // fill in one batch - no repaint per TVI_SORT insert
	m_clcItems.DeleteAllItems();
	m_ctcCategories.DeleteAllItems();
	if (!Main->m_pScripts->m_olItems.IsEmpty())
	{
		POSITION pos = Main->m_pScripts->m_olItems.GetHeadPosition();
		while (pos != NULL)
		{
			CCategory * pCategory = (CCategory *) Main->m_pScripts->m_olItems.GetNext(pos);
			InsertItem.item.mask = TVIF_TEXT;
			InsertItem.item.pszText = (char *)LPCTSTR(pCategory->m_csName);
			InsertItem.item.cchTextMax = pCategory->m_csName.GetLength();
			InsertItem.hParent = NULL;
			InsertItem.hInsertAfter = TVI_SORT;
			CategoryParent = this->m_ctcCategories.InsertItem(&InsertItem);
			if (!pCategory->m_SubsectionList.IsEmpty())
			{
				POSITION sPos = pCategory->m_SubsectionList.GetHeadPosition();
				while (sPos != NULL)
				{
					CSubsection * pSubsection = (CSubsection *) pCategory->m_SubsectionList.GetNext(sPos);
					InsertItem.item.mask = TVIF_TEXT;
					InsertItem.item.pszText = (char *)LPCTSTR(pSubsection->m_csName);
					InsertItem.item.cchTextMax = pSubsection->m_csName.GetLength();
					InsertItem.hParent = CategoryParent;
					InsertItem.hInsertAfter = TVI_SORT;
					SubsectionParent = this->m_ctcCategories.InsertItem(&InsertItem);
				}
			}
		}
	}
	m_ctcCategories.SetRedraw(TRUE);
	m_iCatSeq = Main->m_pScripts->m_iICatSeq;
}

// Fills the item list; shared by category selection and search.
void CItemTab::FillList(CPtrArray & aObjects)
{
	m_clcItems.SetRedraw(FALSE); // fill in one batch, no repaint per item
	m_clcItems.SetHotItem(-1);
	m_clcItems.DeleteAllItems();
	m_pImageList->Remove(-1); // -1 removes all icons at once
	m_pImageList->SetImageCount((UINT)aObjects.GetSize());
	UpdateDetails(NULL);

	for (int iCount = 0; iCount < aObjects.GetSize(); iCount++)
	{
		CSObject * pObject = (CSObject *) aObjects[iCount];
		this->m_clcItems.InsertItem(iCount, pObject->m_csDescription, iCount);
		CString cs_ItemID = pObject->m_csID;
		cs_ItemID.MakeLower();
		this->m_clcItems.SetItemText(iCount, 1, cs_ItemID);
		this->m_clcItems.SetItemText(iCount, 3, ScriptObjectCategory(pObject));
		this->m_clcItems.SetItemData(iCount, (DWORD_PTR)pObject);

		// alltoi is a recursive binary search - resolve each value once
		DWORD dwDisplay = alltoi(pObject->m_csDisplay,pObject->m_csValue);
		WORD wID = ( dwDisplay != 0 ) ? (WORD) dwDisplay : (WORD) -1;
		DWORD dwColor = alltoi(pObject->m_csColor,pObject->m_csValue);
		WORD wColor = ( dwColor != 0 ) ? (WORD) dwColor : 0;

		SetTiledataColumns(m_clcItems, m_Icon, iCount, pObject, wID);

		// Icons are expensive; skip them for very large result lists.
		if (pObject->m_bType != TYPE_MULTI && aObjects.GetSize() <= 600)
		{
			CBitmap *pTempBitmap = m_Icon.OnCreateIcon(icX,icY,wID,wColor);
			m_pImageList->Replace(iCount,pTempBitmap,NULL);
			delete pTempBitmap;
		}
	}
	m_clcItems.SetRedraw(TRUE);
	m_clcItems.Invalidate();
}

void CItemTab::OnSelchangedCategoryTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pResult);
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hSelectedItem = NULL;
	HTREEITEM hParentItem = NULL;

	hSelectedItem = this->m_ctcCategories.GetSelectedItem();
	this->m_ctcCategories.SetItemState(hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	hParentItem = this->m_ctcCategories.GetParentItem(hSelectedItem);

	// Do the bold/nobold thing
	if (pNMTreeView != NULL)
	{
		HTREEITEM hOldItem = pNMTreeView->itemOld.hItem;
		if (hOldItem != NULL)
			this->m_ctcCategories.SetItemState(hOldItem, 0, TVIS_BOLD);
	}

	m_iCurState = 1;
	if (m_clcItems.GetView() != 0)
		m_clcItems.SetView(1);

	// Clear the search field without triggering a new search.
	if (m_ceSearch.GetWindowTextLength() > 0)
	{
		KillTimer(1);
		m_bIgnoreSearchChange = true;
		m_ceSearch.SetWindowText("");
		m_bIgnoreSearchChange = false;
	}

	CPtrArray aObjects;
	CString csInfo;
	if (hParentItem != NULL)
	{
		CString csParent = this->m_ctcCategories.GetItemText(hParentItem);
		CString csSelected = this->m_ctcCategories.GetItemText(hSelectedItem);
		CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olItems, csParent);
		if (pCategory != NULL)
		{
			CSubsection * pSubsection = FindSubsection(pCategory, csSelected);
			if (pSubsection != NULL)
			{
				POSITION pos = pSubsection->m_ItemList.GetHeadPosition();
				while (pos != NULL)
					aObjects.Add(pSubsection->m_ItemList.GetNext(pos));
			}
		}
		csInfo.Format(AXT("%s / %s: %d Items"), (LPCTSTR) csParent, (LPCTSTR) csSelected, (int) aObjects.GetSize());
	}
	else
		csInfo = AXT("Unterbereich w\xE4hlen, um die Items zu sehen");
	FillList(aObjects);
	m_csResultInfo.SetWindowText(csInfo);

	iNameSort = 1;
	iIDSort = 1;
	this->m_clcItems.SortItems(CompareFunc, 0);		// Sort by Description
	iNameSort = -1;
}

// Shows details and preview for the selected item.
void CItemTab::UpdateDetails(CSObject * pObject)
{
	if (pObject == NULL)
	{
		m_csItemID.SetWindowText("");
		m_csItemIDDec.SetWindowText("");
		m_csDetails.SetWindowText(AXT("Kein Item gew\xE4hlt.\n\nEin Item in der Liste anklicken - dann hier erstellen, "
			"als Static setzen oder als Spawnpunkt einrichten."));
		m_Display.SetArtIndex(-1);
		m_Display.SetArtColor(0);
		return;
	}

	CString cs_ItemID = pObject->m_csID;
	cs_ItemID.MakeLower();
	m_csItemID.SetWindowText(cs_ItemID);

	DWORD dwDisplay = alltoi(pObject->m_csDisplay, pObject->m_csValue);
	WORD wID = ( dwDisplay != 0 ) ? (WORD) dwDisplay : (WORD) -1;
	DWORD dwColor = alltoi(pObject->m_csColor, pObject->m_csValue);
	WORD wColor = ( dwColor != 0 ) ? (WORD) dwColor : 0;

	CString csDetails, csLine;
	csDetails = pObject->m_csDescription;
	csDetails += AXT("\n\nDefname:  ") + cs_ItemID;
	if (wID != (WORD) -1)
	{
		csLine.Format(AXT("\nGrafik:  0x%04X   Farbe:  0x%X"), wID, wColor);
		csDetails += csLine;
	}
	CString csCat = ScriptObjectCategory(pObject);
	if (!csCat.IsEmpty())
		csDetails += AXT("\nKategorie:  ") + csCat;
	CUOArt::CItemTileInfo info;
	if (pObject->m_bType != TYPE_MULTI && wID != (WORD) -1 && m_Icon.GetItemTileInfo(wID, info))
	{
		if (info.bWeight == 255)
			csLine.Format(AXT("\nGewicht:  fest   H\xF6he:  %u"), info.bHeight);
		else
			csLine.Format(AXT("\nGewicht:  %u   H\xF6he:  %u"), info.bWeight, info.bHeight);
		csDetails += csLine;
		CString csFlags = TiledataFlagText(info.dwFlags);
		if (!csFlags.IsEmpty())
			csDetails += AXT("\n") + csFlags;
	}
	if (pObject->m_bType == TYPE_MULTI)
		csDetails += AXT("\nMulti (Haus/Schiff) - Rechtsklick auf die Vorschau zeigt es ganz");
	m_csDetails.SetWindowText(csDetails);

	if (Main->m_dwShowItems)
	{
		m_Display.SetArtType(pObject->m_bType == TYPE_MULTI ? 0 : 1);
		m_Display.SetArtIndex(wID);
		m_Display.SetArtColor(wColor);
	}
}

void CItemTab::OnItemchangedItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;
	if (!(pNMListView->uNewState & LVNI_SELECTED))
		return;
	int iSelIndex = this->m_clcItems.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1)
	{
		UpdateDetails(NULL);
		return;
	}
	this->m_clcItems.SetHotItem(iSelIndex);
	CSObject * pObject = (CSObject *) this->m_clcItems.GetItemData(iSelIndex);
	if ( !pObject || pObject->m_csFilename == "" )
		return;
	UpdateDetails(pObject);
}

// ---- Live search (logic in SearchScriptObjects(), Common.cpp) ----

void CItemTab::OnChangeSearch()
{
	if (m_bIgnoreSearchChange)
		return;
	KillTimer(1);
	SetTimer(1, 220, NULL);	// Debounce typing
}

void CItemTab::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		RunSearch();
		return;
	}
	CDockingPage::OnTimer(nIDEvent);
}

void CItemTab::RunSearch()
{
	CString csQuery;
	m_ceSearch.GetWindowText(csQuery);
	csQuery.Trim();
	if (csQuery.IsEmpty())
	{
		CPtrArray aNone;
		FillList(aNone);
		m_csResultInfo.SetWindowText(AXT("Kategorie links w\xE4hlen oder oben suchen"));
		return;
	}

	CWaitCursor wait;
	const int MAX_HITS = 1500;
	CPtrArray aObjects;
	int iTotal = SearchScriptObjects(&Main->m_pScripts->m_olItems, csQuery, aObjects, MAX_HITS);
	FillList(aObjects);

	CString csInfo;
	if (iTotal == 0)
		csInfo.Format(AXT("Keine Treffer f\xFCr \"%s\""), (LPCTSTR) csQuery);
	else if (iTotal > MAX_HITS)
		csInfo.Format(AXT("%d Treffer f\xFCr \"%s\" - die ersten %d angezeigt, Suche genauer fassen"), iTotal, (LPCTSTR) csQuery, MAX_HITS);
	else
		csInfo.Format(AXT("%d Treffer f\xFCr \"%s\" - Enter/Pfeil runter springt in die Liste"), iTotal, (LPCTSTR) csQuery);
	m_csResultInfo.SetWindowText(csInfo);
	if (m_clcItems.GetItemCount() > 0)
		m_clcItems.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}
void CItemTab::OnColumnclickItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	this->m_clcItems.SortItems(CompareFunc, pNMListView->iSubItem);
	if (pNMListView->iSubItem == 0)
	{
		switch(iNameSort)
		{
		case -1:
			iNameSort = 1;
			break;
		case 0:
			iNameSort = 1;
			break;
		case 1:
			iNameSort = -1;
			break;
		}
	}
	if (pNMListView->iSubItem == 1)
	{
		switch(iIDSort)
		{
		case -1:
			iIDSort = 1;
			break;
		case 0:
			iIDSort = 1;
			break;
		case 1:
			iIDSort = -1;
			break;
		}
	}
	*pResult = 0;
}

void CItemTab::OnDblclkItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREFERENCED_PARAMETER(pNMHDR);
	int iSelIndex = this->m_clcItems.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1)
	{
		this->m_csItemID.SetWindowText("");
		m_csItemIDDec.SetWindowText("");
		m_Display.SetArtIndex(-1);
		m_Display.SetArtColor(0);
		return;
	}
	CSObject * pObject = (CSObject *) this->m_clcItems.GetItemData(iSelIndex);
	if ( !pObject )
		return;

	this->OnCreate();

	*pResult = 0;
}


BOOL CItemTab::OnSetActive() 
{
	UpdateData();
	if ( Main->m_pScripts->m_iICatSeq != m_iCatSeq )
		this->FillCategoryTree();
	WORD wFlags = 0;
	if ( Main->m_dwRoomView )
		wFlags |= F_ROOMVIEW;
	//wFlags |= F_BG_GRASS;
	m_Display.SetDrawFlags(wFlags);
	m_Display.SetBkColor(Main->m_dwItemBGColor);

	return CDockingPage::OnSetActive();
}

int CALLBACK CItemTab::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSObject * pObject1 = (CSObject *) lParam1;
	CSObject * pObject2 = (CSObject *) lParam2;

	if ( pObject1 && pObject2 )
	{
		if (lParamSort == 0)
		{
			if (iNameSort == 0)
				return (lParam1 < lParam2 ? 1 : -1);
			if (iNameSort == 1)
			{
				if (pObject1->m_csDescription < pObject2->m_csDescription)
					return -1;
				else if (pObject1->m_csDescription > pObject2->m_csDescription)
					return 1;
				else
					return 0;
			}
			if (iNameSort == -1)
			{
				if (pObject1->m_csDescription < pObject2->m_csDescription)
					return 1;
				else if (pObject1->m_csDescription > pObject2->m_csDescription)
					return -1;
				else
					return 0;
			}
		}
		else
		{
			if (iIDSort == 0)
				return (lParam1 < lParam2 ? 1 : -1);
			if (iIDSort == 1)
				return (pObject1->m_csID < pObject2->m_csID ? -1 : 1);
			if (iIDSort == -1)
				return (pObject1->m_csID < pObject2->m_csID ? 1 : -1);
		}
	}
	return 0;
}

void CItemTab::OnFinditem()
{
	// Clear the search and return focus to the search field.
	KillTimer(1);
	m_bIgnoreSearchChange = true;
	m_ceSearch.SetWindowText("");
	m_bIgnoreSearchChange = false;
	RunSearch();
	m_ceSearch.SetFocus();
}

BOOL CItemTab::PreTranslateMessage(MSG* pMsg)
{
	if (m_tipItems.GetSafeHwnd())
		m_tipItems.RelayEvent(pMsg);
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->hwnd == m_ceSearch.GetSafeHwnd() )
		{
			if ( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DOWN )
			{
				// Search immediately and jump into the list.
				KillTimer(1);
				RunSearch();
				if ( m_clcItems.GetItemCount() > 0 )
				{
					m_clcItems.SetFocus();
					m_clcItems.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_clcItems.EnsureVisible(0, FALSE);
				}
				return TRUE;
			}
			if ( pMsg->wParam == VK_ESCAPE )
			{
				OnFinditem();
				return TRUE;
			}
		}
		else if ( pMsg->hwnd == m_clcItems.GetSafeHwnd() && pMsg->wParam == VK_RETURN )
		{
			if ( m_clcItems.GetNextItem(-1, LVNI_SELECTED) != -1 )
				OnCreate();
			return TRUE;
		}
		// Ctrl+F focuses the search field.
		if ( pMsg->wParam == 'F' && (GetKeyState(VK_CONTROL) & 0x8000) )
		{
			m_ceSearch.SetFocus();
			m_ceSearch.SetSel(0, -1);
			return TRUE;
		}
	}
	return CDockingPage::PreTranslateMessage(pMsg);
}
void CItemTab::OnRclickItems(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMITEMACTIVATE * pInfo = (NMITEMACTIVATE *)pNMHDR;
	CRect rListCtrl;
	m_clcItems.GetWindowRect(&rListCtrl);

	HMENU hMenu = ::CreatePopupMenu();
	if (NULL != hMenu)
	{
		if (m_clcItems.GetView() == 0)
			::AppendMenu(hMenu, MF_STRING|MF_USECHECKBITMAPS|MF_CHECKED, 2, AXT("Symbolansicht"));
		else
			::AppendMenu(hMenu, MF_STRING|MF_USECHECKBITMAPS|MF_UNCHECKED, 1, AXT("Symbolansicht"));

		if (m_iCurState == 1)
			::AppendMenu(hMenu, MF_STRING|MF_USECHECKBITMAPS|MF_UNCHECKED, 3, AXT("Schnellliste anzeigen"));
		else
			::AppendMenu(hMenu, MF_STRING|MF_USECHECKBITMAPS|MF_CHECKED, 4, AXT("Schnellliste anzeigen"));

		if (pInfo->iItem != -1)
		{
			::AppendMenu(hMenu, MF_MENUBREAK, NULL, NULL);
			::AppendMenu(hMenu, MF_STRING, 7, AXT("Item-Infos anzeigen"));
			::AppendMenu(hMenu, MF_STRING, 8, AXT("Skript bearbeiten"));
			::AppendMenu(hMenu, MF_MENUBREAK, NULL, NULL);
			if (m_iCurState != 2)
				::AppendMenu(hMenu, MF_STRING, 5, AXT("Zur Schnellliste hinzuf\xFCgen"));
			else
				::AppendMenu(hMenu, MF_STRING, 6, AXT("Aus der Schnellliste entfernen"));
		}
		int sel = ::TrackPopupMenuEx(hMenu, 
				TPM_CENTERALIGN | TPM_RETURNCMD,
				pInfo->ptAction.x + rListCtrl.left,
				pInfo->ptAction.y + rListCtrl.top,
				m_hWnd,
				NULL);
		switch (sel)
		{
		case 1:
			{
				m_iCurState = m_clcItems.GetView();
				m_clcItems.SetView(0);
			break;
			}
		case 2:
			{
				m_clcItems.SetView(m_iCurState);
				if (m_iCurState == 2)
					m_clcItems.SetColumnWidth(0,150);
			break;
			}
		case 3:
			{
				m_iCurState = 2;
				if (m_clcItems.GetView() != 0)
				{
					m_clcItems.SetView(2);
					m_clcItems.SetColumnWidth(0,150);
				}
				OnShowQuicklist();
			break;
			}
		case 4:
			{
				m_iCurState = 1;
				if (m_clcItems.GetView() != 0)
					m_clcItems.SetView(1);
				OnExitQuicklist();
			break;
			}
		case 5:
			{
				CSObject * pObject = (CSObject *) this->m_clcItems.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				if (Main->m_pScripts->m_ItemQuickList.Find(pObject->m_csValue) != -1)
				{
					CString csMessage;
					csMessage.Format(AXT("%s ist schon in der Schnellliste"),pObject->m_csDescription );
					AfxMessageBox(csMessage);
					return;
				}
				Main->m_pScripts->m_ItemQuickList.Insert(pObject);
				OnSaveQuicklist("Items"); 
			break;
			}
		case 6:
			{
				CSObject * pObject = (CSObject *) this->m_clcItems.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				Main->m_pScripts->m_ItemQuickList.RemoveAt(Main->m_pScripts->m_ItemQuickList.Find(pObject->m_csValue));
				this->m_clcItems.DeleteItem(pInfo->iItem);
				iNameSort = 1;
				iIDSort = 1;
				this->m_clcItems.SortItems(CompareFunc, 0);
				iNameSort = -1;
				OnSaveQuicklist("Items");
			break;
			}
		case 7:
			{
				CSObject * pObject = (CSObject *) this->m_clcItems.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				CScriptInfo * dlg = new CScriptInfo;
				dlg->pObject = pObject;
				dlg->Create(IDD_SCRIPT_INFO);
			break;
			}
		case 8:
			{
				CSObject * pObject = (CSObject *) this->m_clcItems.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				CScriptEditor * dlg = new CScriptEditor;
				dlg->pObject = pObject;
				dlg->Create(IDD_SCRIPT_EDITOR);
			break;
			}
		}
		::DestroyMenu(hMenu);
	}


	*pResult = 0;
}

void CItemTab::OnSaveQuicklist(CString csList) 
{
	CStringArray aQuicklist;
	for(int iCount = 0; iCount < Main->m_pScripts->m_ItemQuickList.GetCount(); iCount++)
	{
		CSObject * pObject = (CSObject *) Main->m_pScripts->m_ItemQuickList.GetAt(iCount);
		aQuicklist.Add(pObject->m_csValue);
	}
	// Write even an empty list so the last removed entry does not come back.
	CString csProfileKey;
	csProfileKey.Format("%s\\%s\\Quicklist",REGKEY_PROFILE, Main->m_csCurentProfile);
	Main->PutRegistryMultiSz(csList, &aQuicklist, hRegLocation, csProfileKey);
}

void CItemTab::OnShowQuicklist() 
{
	CPtrArray aObjects;
	for(int iCount = 0; iCount < Main->m_pScripts->m_ItemQuickList.GetCount(); iCount++)
		aObjects.Add(Main->m_pScripts->m_ItemQuickList.GetAt(iCount));
	FillList(aObjects);
	CString csInfo;
	csInfo.Format(AXT("Schnellliste: %d Items (Rechtsklick > Zur Schnellliste hinzuf\xFCgen)"), (int) aObjects.GetSize());
	m_csResultInfo.SetWindowText(csInfo);

	iNameSort = 1;
	iIDSort = 1;
	this->m_clcItems.SortItems(CompareFunc, 0);		// Sort by Description
	iNameSort = -1;
}

void CItemTab::OnExitQuicklist() 
{
	// Return to the search results or the selected category.
	if (m_ceSearch.GetWindowTextLength() > 0)
		RunSearch();
	else
	{
		LRESULT lResult = 0;
		OnSelchangedCategoryTree(NULL, &lResult);
	}
}