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

// TravelTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "TravelTab.h"
#include "UOmap.h"
#include "Common.h"
#include "SearchCritDlg.h"
#include "AddDestination.h"
#include "WorldMap.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTravelTab property page

IMPLEMENT_DYNCREATE(CTravelTab, CDockingPage)

int CTravelTab::iLocSort = 1;
int CTravelTab::iCoordSort = 1;

CTravelTab::CTravelTab() : CDockingPage(CTravelTab::IDD,CMsg("IDS_TRAVEL"))
{
	//{{AFX_DATA_INIT(CTravelTab)
	m_iCatSeq = 0;
	m_bTrack = FALSE;
	m_bIgnoreSearchChange = false;
	//}}AFX_DATA_INIT
	m_bCachedML = false;
	m_sLastSavedX = m_sLastSavedY = m_sLastSavedPlane = -1;
	m_dwLastSavedZoom = (DWORD) -1;
}

CTravelTab::~CTravelTab()
{
}

void CTravelTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTravelTab)
	DDX_Control(pDX, IDC_LOCATIONS, m_clcLocations);
	DDX_Control(pDX, IDC_CATEGORYTREE, m_ctcCategoryTree);
	DDX_Control(pDX, IDC_TRAVELMAP, m_TravelMap);
	DDX_Control(pDX, IDC_MAPPLANE, m_ceMapPlane);
	DDX_Control(pDX, IDC_COORDS, m_csCoords);
	DDX_Control(pDX, IDC_XSCROLL, m_csbXScroll);
	DDX_Control(pDX, IDC_YSCROLL, m_csbYScroll);
	DDX_Control(pDX, IDC_EDITLOC, m_cbEditLoc);
	DDX_Control(pDX, IDC_REMLOC, m_cbRemoveLoc);
	DDX_Check(pDX, IDC_TRACK, m_bTrack);
	DDX_Control(pDX, IDC_WORLD, cb_world);
	DDX_Control(pDX, IDC_ZOOMIN, cb_zoomin);
	DDX_Control(pDX, IDC_ZOOMOUT, cb_zoomout);
	DDX_Control(pDX, IDC_FINDAREA, cb_findarea);
	DDX_Control(pDX, IDC_TRAVELSEARCH, m_ceSearch);
	DDX_Control(pDX, IDC_TRAVEL_RESULTINFO, m_csResultInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTravelTab, CDockingPage)
	//{{AFX_MSG_MAP(CTravelTab)
	ON_NOTIFY(TVN_SELCHANGED, IDC_CATEGORYTREE, OnSelchangedCategorytree)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LOCATIONS, OnColumnclickLocations)
	ON_NOTIFY(NM_DBLCLK, IDC_LOCATIONS, OnDblclkLocations)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LOCATIONS, OnItemchangedLocations)
	ON_NOTIFY(NM_RCLICK, IDC_LOCATIONS, OnRclickLocations)
	ON_EN_CHANGE(IDC_MAPPLANE, OnChangeMapplane)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomout)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomin)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_ADDLOC, OnAddLoc)
	ON_BN_CLICKED(IDC_REMLOC, OnRemoveLoc)
	ON_BN_CLICKED(IDC_EDITLOC, OnEditLoc)
	ON_BN_CLICKED(IDC_FINDAREA, OnFindarea)
	ON_BN_CLICKED(IDC_WORLD, OnWorldMap)
	ON_BN_CLICKED(IDC_LOCATE, OnLocate)
	ON_BN_CLICKED(IDC_TRACK, OnTrack)
	ON_BN_CLICKED(IDC_WHERE, OnWhere)
	ON_EN_CHANGE(IDC_TRAVELSEARCH, OnChangeSearch)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTravelTab message handlers

BOOL CTravelTab::OnInitDialog() 
{
	m_dcPropertyPage = Main->m_pcppTravelTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CTravelTab;
	Main->m_pcppTravelTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();

	// Set up the travel map window
	int X, Y, M, w, h, zoom;
	X = Main->GetRegistryDword("TravelTab Center X", 3077);
	Y = Main->GetRegistryDword("TravelTab Center Y", 2048);
	M = Main->GetRegistryDword("TravelTab MapPlane", 0);
	zoom = Main->GetRegistryDword("TravelTab Zoom", (DWORD)-2);
	CString csMap;
	csMap.Format("%d", M);
	m_TravelMap.SetZoomLevel((short) zoom);
	WORD wFlags = 0;
	if ( Main->m_dwDrawStatics )
		wFlags |= 0x01;
	if ( Main->m_dwDrawDifs )
		wFlags |= 0x02;
	m_TravelMap.SetDrawFlags(wFlags);
	m_ceMapPlane.SetWindowText(csMap);

	w = m_TravelMap.GetWidth();
	h = m_TravelMap.GetHeight();
	m_csbXScroll.SetScrollRange(0, w);
	m_csbXScroll.SetScrollPos(X);
	m_csbYScroll.SetScrollRange(0, h);
	m_csbYScroll.SetScrollPos(Y);
	xtempscroll = 0;
	ytempscroll = 0;

	Recenter((short) X, (short) Y);
	FillCategoryTree();
	this->m_clcLocations.InsertColumn(0, AXT("Name"), LVCFMT_LEFT, 170, -1);
	this->m_clcLocations.InsertColumn(1, AXT("Koordinaten"), LVCFMT_LEFT, 100, -1);
	this->m_clcLocations.InsertColumn(2, AXT("Karte"), LVCFMT_CENTER, 42, -1);
	this->m_clcLocations.InsertColumn(3, AXT("Kategorie"), LVCFMT_LEFT, 150, -1);
	m_clcLocations.SetExtendedStyle(m_clcLocations.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	ApplyAxisListHeader(m_clcLocations.GetSafeHwnd());
	m_iCurState = m_clcLocations.GetView();
	AxisSetCue(m_ceSearch.GetSafeHwnd(), "z.B.  britain   bank   1496,1628");
	m_csResultInfo.SetWindowText(AXT("Kategorie links w\xE4hlen oder oben suchen"));
	AxisMarkPrimary(this, IDC_GO);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//*********************
//Scroll functions
void CTravelTab::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar->m_hWnd == m_csbXScroll.m_hWnd)
	{
		int minpos;
		int maxpos;
		pScrollBar->GetScrollRange(&minpos, &maxpos); 
		int curpos = pScrollBar->GetScrollPos();
		switch (nSBCode)
		{
		case SB_TOP:      // Scroll to far left.
			curpos = minpos;
			break;
		case SB_BOTTOM:      // Scroll to far right.
			curpos = maxpos;
			break;
		case SB_ENDSCROLL:   // End scroll.
			break;
		case SB_LINEUP:      // Scroll left.
			if (curpos > minpos)
				curpos--;
			break;
		case SB_LINEDOWN:   // Scroll right.
			if (curpos < maxpos)
				curpos++;
			break;
		case SB_PAGEUP:    // Scroll one page left.
		{
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);
			if (curpos > minpos)
				curpos = max(minpos, curpos - (int) info.nPage);
		}
			break;
		case SB_PAGEDOWN:      // Scroll one page right.
		{
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);
			if (curpos < maxpos)
				curpos = min(maxpos, curpos + (int) info.nPage);
		}
			break;
		case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
			curpos = nPos;      // of the scroll box at the end of the drag operation.
			break;
		case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
			curpos = nPos;     // position that the scroll box has been dragged to.
			break;
		}

		pScrollBar->SetScrollPos(curpos);
		Recenter((short)curpos, m_TravelMap.GetYCenter());
	}
	CDockingPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTravelTab::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar->m_hWnd == m_csbYScroll.m_hWnd)
	{
		int minpos;
		int maxpos;
		pScrollBar->GetScrollRange(&minpos, &maxpos); 
		int curpos = pScrollBar->GetScrollPos();
		switch (nSBCode)
		{
		case SB_TOP:      // Scroll to far left.
			curpos = minpos;
			break;
		case SB_BOTTOM:      // Scroll to far right.
			curpos = maxpos;
			break;
		case SB_ENDSCROLL:   // End scroll.
			break;
		case SB_LINEUP:      // Scroll left.
			if (curpos > minpos)
				curpos--;
			break;
		case SB_LINEDOWN:   // Scroll right.
			if (curpos < maxpos)
				curpos++;
			break;
		case SB_PAGEUP:    // Scroll one page left.
		{
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);
			if (curpos > minpos)
				curpos = max(minpos, curpos - (int) info.nPage);
		}
			break;
		case SB_PAGEDOWN:      // Scroll one page right.
		{
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);
			if (curpos < maxpos)
				curpos = min(maxpos, curpos + (int) info.nPage);
		}
			break;
		case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
			curpos = nPos;      // of the scroll box at the end of the drag operation.
			break;
		case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
			curpos = nPos;     // position that the scroll box has been dragged to.
			break;
		}

		pScrollBar->SetScrollPos(curpos);
		m_TravelMap.SetYCenter((short)curpos);
		Recenter(m_TravelMap.GetXCenter(), (short)curpos);
	}
	CDockingPage::OnVScroll(nSBCode, nPos, pScrollBar);
}

//***************************
//Main Buttons
void CTravelTab::OnGo() 
{
	CString csVal, csPlane, csCmd;
	this->m_csCoords.GetWindowText(csVal);
	this->m_ceMapPlane.GetWindowText(csPlane);
	csCmd.Format("%sgo %s,%s", Main->m_csCommandPrefix, csVal, csPlane);
	SendToUO(csCmd);
}

void CTravelTab::OnSend() 
{
	CString csVal, csPlane, csCmd;
	this->m_csCoords.GetWindowText(csVal);
	this->m_ceMapPlane.GetWindowText(csPlane);
	csCmd.Format("%sXgo %s,%s", Main->m_csCommandPrefix, csVal, csPlane);
	SendToUO(csCmd);
}

void CTravelTab::OnWhere() 
{
	CString csCmd;
	csCmd.Format("%swhere", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CTravelTab::OnZoomout() 
{
	m_TravelMap.ZoomOut();
	Recenter();
}

void CTravelTab::OnZoomin() 
{
	m_TravelMap.ZoomIn();
	Recenter();
}


//***************************************************
//Tree Control
void CTravelTab::FillCategoryTree()
{
	HTREEITEM CategoryParent;
	HTREEITEM SubsectionParent;
	TV_INSERTSTRUCT InsertItem;

	m_ctcCategoryTree.SetRedraw(FALSE); // no repaint per insert
	m_clcLocations.DeleteAllItems();
	m_ctcCategoryTree.DeleteAllItems();

	if (!Main->m_pScripts->m_olAreas.IsEmpty())
	{
		POSITION pos = Main->m_pScripts->m_olAreas.GetHeadPosition();
		while (pos != NULL)
		{
			CCategory * pCategory = (CCategory *) Main->m_pScripts->m_olAreas.GetNext(pos);
			InsertItem.item.mask = TVIF_TEXT;
			InsertItem.item.pszText = (char *)LPCTSTR(pCategory->m_csName);
			InsertItem.item.cchTextMax = pCategory->m_csName.GetLength();
			InsertItem.hParent = NULL;
			InsertItem.hInsertAfter = TVI_SORT;
			CategoryParent = this->m_ctcCategoryTree.InsertItem(&InsertItem);
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
					SubsectionParent = this->m_ctcCategoryTree.InsertItem(&InsertItem);
				}
			}
		}
	}
	m_ctcCategoryTree.SetRedraw(TRUE);
	m_iCatSeq = Main->m_pScripts->m_iACatSeq;
}

void CTravelTab::OnSelchangedCategorytree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREFERENCED_PARAMETER(pResult);
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	this->m_clcLocations.SetHotItem(-1);

	HTREEITEM hSelectedItem = NULL;
	HTREEITEM hParentItem = NULL;
	this->m_clcLocations.DeleteAllItems();
	this->m_csCoords.SetWindowText("");
	this->m_cbRemoveLoc.EnableWindow(FALSE);
	this->m_cbEditLoc.EnableWindow(FALSE);

	if (pNMTreeView != NULL)
	{
		HTREEITEM hOldItem = pNMTreeView->itemOld.hItem;
		if (hOldItem != NULL)
			this->m_ctcCategoryTree.SetItemState(hOldItem, 0, TVIS_BOLD);
	}

	hSelectedItem = this->m_ctcCategoryTree.GetSelectedItem();
	this->m_ctcCategoryTree.SetItemState(hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	hParentItem = this->m_ctcCategoryTree.GetParentItem(hSelectedItem);

	if (m_ceSearch.GetWindowTextLength() > 0)
	{
		KillTimer(1);
		m_bIgnoreSearchChange = true;
		m_ceSearch.SetWindowText("");
		m_bIgnoreSearchChange = false;
	}
	CPtrArray aObjects;
	CString csInfo = AXT("Unterbereich w\xE4hlen, um die Orte zu sehen");
	if (hParentItem != NULL)
	{
		CString csParent = this->m_ctcCategoryTree.GetItemText(hParentItem);
		CString csSelected = this->m_ctcCategoryTree.GetItemText(hSelectedItem);
		CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olAreas, csParent);
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
		csInfo.Format(AXT("%s / %s: %d Orte"), (LPCTSTR) csParent, (LPCTSTR) csSelected, (int) aObjects.GetSize());
	}
	FillList(aObjects);
	m_csResultInfo.SetWindowText(csInfo);
	iLocSort = 1;
	iCoordSort = 1;
	this->m_clcLocations.SortItems(CompareFunc, 0);		// Sort by Description
	iLocSort = -1;
}

void CTravelTab::OnItemchangedLocations(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (pNMListView->uNewState & LVNI_SELECTED)
	{
		int iSelIndex = this->m_clcLocations.GetNextItem(-1, LVNI_SELECTED);
		if (iSelIndex == -1)
		{
			this->m_csCoords.SetWindowText("");
			Recenter(3077, 2048);
			return;
		}
		this->m_clcLocations.SetHotItem(iSelIndex);
		CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(iSelIndex);
		if ( !pObject )
		{
			this->m_cbRemoveLoc.EnableWindow(FALSE);
			this->m_cbEditLoc.EnableWindow(FALSE);
			return;
		}

		this->m_cbRemoveLoc.EnableWindow(pObject->m_bCustom ? TRUE : FALSE);
		this->m_cbEditLoc.EnableWindow(pObject->m_bCustom ? TRUE : FALSE);
		this->m_csCoords.SetWindowText(pObject->m_csID);
		this->m_ceMapPlane.SetWindowText(pObject->m_csDisplay);
		CString csX, csY, csZ;
		csX = pObject->m_csID.SpanExcluding(",");
		csY = pObject->m_csID.Mid(pObject->m_csID.Find(",") + 1);
		csZ = csY.Mid(csY.Find(",") + 1);
		csY = csY.SpanExcluding(",");
		Recenter((short)atoi(csX), (short)atoi(csY), (short)atoi(csZ), (short)atoi(pObject->m_csDisplay));
		return;
	}

	this->m_cbRemoveLoc.EnableWindow(FALSE);
	this->m_cbEditLoc.EnableWindow(FALSE);
	*pResult = 0;
}

void CTravelTab::OnColumnclickLocations(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	this->m_clcLocations.SortItems(CompareFunc, pNMListView->iSubItem);
	if (pNMListView->iSubItem == 0)
	{
		switch(iLocSort)
		{
		case -1:
			iLocSort = 1;
			break;
		case 0:
			iLocSort = 1;
			break;
		case 1:
			iLocSort = -1;
			break;
		}
	}
	if (pNMListView->iSubItem == 1)
	{
		switch(iCoordSort)
		{
		case -1:
			iCoordSort = 1;
			break;
		case 0:
			iCoordSort = 1;
			break;
		case 1:
			iCoordSort = -1;
			break;
		}
	}
	*pResult = 0;
}

void CTravelTab::OnDblclkLocations(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREFERENCED_PARAMETER(pNMHDR);
	int iSelIndex = this->m_clcLocations.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1)
	{
		this->m_csCoords.SetWindowText("");
		return;
	}
	CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(iSelIndex);
	if ( !pObject )
		return;

	this->OnGo();

	*pResult = 0;
}

//**************************
//Misc Functions
int CALLBACK CTravelTab::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSObject * pObject1 = (CSObject *) lParam1;
	CSObject * pObject2 = (CSObject *) lParam2;

	if ( pObject1 && pObject2 )
	{
		if (lParamSort == 0)
		{
			if (iLocSort == 0)
				return (lParam1 < lParam2 ? 1 : -1);
			if (iLocSort == 1)
			{
				if (pObject1->m_csDescription < pObject2->m_csDescription)
					return -1;
				else if (pObject1->m_csDescription > pObject2->m_csDescription)
					return 1;
				else
					return 0;
			}
			if (iLocSort == -1)
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
			if (iCoordSort == 0)
				return (lParam1 < lParam2 ? 1 : -1);
			if (iCoordSort == 1)
				return (pObject1->m_csID < pObject2->m_csID ? -1 : 1);
			if (iCoordSort == -1)
				return (pObject1->m_csID < pObject2->m_csID ? 1 : -1);
		}
	}
	return 0;
}

void CTravelTab::OnChangeMapplane() 
{
	if ( this->IsWindowVisible() )
	{
		Recenter();	
		if (Main->m_dlgWorldMap)
		{
			if (Main->m_dwShowMap)
			{
				short m = m_TravelMap.GetMapFile();
				short w = m_TravelMap.GetWidth();
				short h = m_TravelMap.GetHeight();
				Main->m_dlgWorldMap->m_WorldMap.LoadMap(m, w, h);

				WORD wFlags = 0;
				if ( Main->m_dwDrawStatics )
					wFlags |= 0x01;
				if ( Main->m_dwDrawDifs )
					wFlags |= 0x02;
				Main->m_dlgWorldMap->m_WorldMap.SetDrawFlags(wFlags);
				short zoom;
				switch ( m )
				{
				case 0:
				case 1:
					zoom = -3;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
					zoom = -2;
					break;
				default:
					zoom = -3;
					break;
				}
				Main->m_dlgWorldMap->m_WorldMap.SetZoomLevel(zoom);
				Main->m_dlgWorldMap->m_WorldMap.SetMapFile(m);
				CString csMap;
				m_ceMapPlane.GetWindowText(csMap);

				if (Main->m_dwShowSpawnpoints)
					Main->m_dlgWorldMap->UpdateSpawnDisplay(atoi(csMap));
				else
					Main->m_dlgWorldMap->m_WorldMap.RemoveDrawObjects();

				CString csTitle;
				csTitle.Format(AXT("Weltkarte - Karte %s (Klick w\xE4hlt die Stelle)"), csMap);
				Main->m_dlgWorldMap->SetWindowText(csTitle);

				Main->m_dlgWorldMap->UpdateSize();
				Main->m_dlgWorldMap->m_WorldMap.UpdateMap();
			}
		}
	}
}

void CTravelTab::Recenter()
{
	short x, y;
	m_TravelMap.GetCenter(&x, &y);
	Recenter(x, y);
}

void CTravelTab::RemoteRecenter(short x, short y, short z, short m)
{
	Recenter(x, y, z, m);
}

void CTravelTab::Recenter(short x, short y, short z, short m)
{
	if (Main->m_dwShowMap)
	{
		if ( m == 256 )
		{
			CString csPlane;
			m_ceMapPlane.GetWindowText(csPlane);
			m = (short)atoi(csPlane);
		}
		short w, h, sPlane = 0;
		sPlane = m;
		// Redo the ML map size detection only when the map0 path changes.
		bool bML = false;
		CString csMap0Path = Main->GetMulPath(VERFILE_MAP0);
		if ( csMap0Path != m_csCachedMap0Path )
		{
			CFile fData;
			CString csMapFileName = csMap0Path;
			if ( fData.Open(csMapFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone) )
			{
				if ( fData.GetLength() > 89915000 )
					m_bCachedML = true;
				else
					m_bCachedML = false;
				fData.Close();
			}else
			{
				m_bCachedML = false;
				csMapFileName = Main->GetMulPath(VERFILE_MAP0_UOP);
				if ( fData.Open(csMapFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone) )
				{
					m_bCachedML = true;
					fData.Close();
				}
			}
			m_csCachedMap0Path = csMap0Path;
		}
		bML = m_bCachedML;
		switch ( m )
		{
		case 0:
		case 1:
			if (bML)
			{
				w = 7168;
				h = 4096;
			}else{
				w = 6144;
				h = 4096;
			}
			break;
		case 2:
			w = 2304;
			h = 1600;
			break;
		case 3:
			w = 2560;
			h = 2048;
			break;
		case 4:
			w = 1448;
			h = 1448;
			break;
		case 5:
			w = 1280;
			h = 4096;
			break;
		default:
			w = 6144;
			h = 4096;
			break;
		}

		CString csMapSize;
		csMapSize.Format("MAP%d", m);
		int iMap = Main->m_pScripts->m_aDefList.Find(csMapSize);
		if ( iMap != -1 )
		{
			CSObject * pMap = (CSObject *) Main->m_pScripts->m_aDefList.GetAt(iMap);
			CString csW, csH, csMap;
			csW = pMap->m_csID.SpanExcluding(",");
			csH = pMap->m_csID.Mid(pMap->m_csID.Find(",") + 1);
			csMap = csH.Mid(csH.Find(",") + 1);
			csMap = csMap.Mid(csMap.Find(",") + 1);
			csMap = csMap.SpanExcluding(",");
			csH = csH.SpanExcluding(",");
			if((m == 1) && (csMap == "0"))
				csMap = "1";
			if(csMap != "-1")
				m = (short)atoi(csMap);
			w = (short)atoi(csW);
			h = (short)atoi(csH);
		}

		if (( m > 5 )||( m < 0 ))
		{
			m = m_TravelMap.GetMapFile();
			w = m_TravelMap.GetWidth();
			h = m_TravelMap.GetHeight();
		}

		m_TravelMap.LoadMap(m, w, h);
		m_TravelMap.SetCenter(x, y);
		m_csbXScroll.SetScrollRange(0, w);
		m_csbYScroll.SetScrollRange(0, h);
		m_csbXScroll.SetScrollPos(x);
		m_csbYScroll.SetScrollPos(y);
		if (x > w)
			m_csbXScroll.SetScrollPos(w/2);
		if (y > h)
			m_csbYScroll.SetScrollPos(h/2);

		if (Main->m_dwShowSpawnpoints)
			UpdateSpawnDisplay();
		else
			m_TravelMap.RemoveDrawObjects();

		m_TravelMap.RemoveDrawObjectAt(m_crosshairIndex);
		m_crosshairIndex = m_TravelMap.AddDrawObject((short)m_csbXScroll.GetScrollPos(), (short)m_csbYScroll.GetScrollPos(), 1, 3, 0x00FFFFFF);

		// Persist position and zoom only when they changed.
		DWORD dwZoom = (DWORD) m_TravelMap.GetZoomLevel();
		if ( x != m_sLastSavedX || y != m_sLastSavedY || dwZoom != m_dwLastSavedZoom || sPlane != m_sLastSavedPlane )
		{
			Main->PutRegistryDword("TravelTab Center X", (DWORD) x);
			Main->PutRegistryDword("TravelTab Center Y", (DWORD) y);
			Main->PutRegistryDword("TravelTab Zoom", dwZoom);
			Main->PutRegistryDword("TravelTab MapPlane", (DWORD) sPlane);
			m_sLastSavedX = x;
			m_sLastSavedY = y;
			m_dwLastSavedZoom = dwZoom;
			m_sLastSavedPlane = sPlane;
		}

		CString csCoords;
		if ( z > 255 )
			csCoords.Format("%d, %d, %d", x, y, m_TravelMap.GetMapHeight(x, y));
		else
			csCoords.Format("%d, %d, %d", x, y, z);
		m_csCoords.SetWindowText(csCoords);
		m_TravelMap.UpdateMap();
	}
	else
	{
		m_TravelMap.SetMapFile(-1);
		m_TravelMap.UpdateMap();
	}
}

//**************
//Map Control

void CTravelTab::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ClientToScreen(&point);
	CRect rect;
	m_TravelMap.GetWindowRect(&rect);
	if ( rect.PtInRect(point) )
	{
		xscroll = point.x-rect.left;
		yscroll = point.y-rect.top;
	}
	else
	{
		ScreenToClient(&point);
		CDockingPage::OnRButtonDown(nFlags, point);
	}
}

void CTravelTab::OnMouseMove(UINT nFlags, CPoint point) 
{
	ClientToScreen(&point);
	CRect rect;
	m_TravelMap.GetWindowRect(&rect);
	if ( (rect.PtInRect(point)) && (nFlags & MK_RBUTTON) )
	{
		short xs, ys, xc, yc, zc, zoom;
		xs = (short) point.x - (short) rect.left - (short) xscroll;
		ys = (short) point.y - (short) rect.top - (short) yscroll;
		xscroll = point.x-rect.left;
		yscroll = point.y-rect.top;
		zoom = m_TravelMap.GetZoomLevel();
		switch( zoom )
		{
		case -4:
			xs *= 16;
			ys *= 16;
			xtempscroll = 0;
			ytempscroll = 0;
			break;
		case -3:
			xs *= 8;
			ys *= 8;
			xtempscroll = 0;
			ytempscroll = 0;
			break;
		case -2:
			xs *= 4;
			ys *= 4;
			xtempscroll = 0;
			ytempscroll = 0;
			break;
		case -1:
			xs *= 2;
			ys *= 2;
			xtempscroll = 0;
			ytempscroll = 0;
			break;
		case 0:
			break;
			xtempscroll = 0;
			ytempscroll = 0;
		case 1:
			xtempscroll += xs*8;
			ytempscroll += ys*8;
			break;
		case 2:
			xtempscroll += xs*4;
			ytempscroll += ys*4;
			break;
		case 3:
			xtempscroll += xs*2;
			ytempscroll += ys*2;
			break;
		case 4:
			xtempscroll += xs;
			ytempscroll += ys;
			break;
		}

		if (xtempscroll)
		{
			if(xtempscroll >= 16)
			{
				m_TravelMap.Scroll(6, 1);
				xtempscroll -= 16;
			}
			else if(xtempscroll <= -16)
			{
				m_TravelMap.Scroll(6, -1);
				xtempscroll += 16;
			}
		}
		else
			m_TravelMap.Scroll(6, xs);

		if (ytempscroll)
		{
			if(ytempscroll >= 16)
			{
				m_TravelMap.Scroll(0, 1);
				ytempscroll -= 16;
			}
			else if(ytempscroll <= -16)
			{
				m_TravelMap.Scroll(0, -1);
				ytempscroll += 16;
			}
		}
		else
			m_TravelMap.Scroll(0, ys);

		xc = m_TravelMap.GetXCenter();
		yc = m_TravelMap.GetYCenter();
		zc = m_TravelMap.GetMapHeight(xc, yc);
		CString csMessage;
		csMessage.Format(AXT("%d,%d,%d"), xc, yc, zc);
		m_csCoords.SetWindowText(csMessage);
		Recenter(xc, yc);
	}
}


void CTravelTab::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UNREFERENCED_PARAMETER(nFlags);
	ClientToScreen(&point);
	CRect rect;
	m_TravelMap.GetWindowRect(&rect);
	if ( rect.PtInRect(point) )
	{
			short xc, yc, z;
			xc = (short) point.x - (short) rect.left;
			yc = (short) point.y - (short) rect.top;
			m_TravelMap.CtrlToMap(&xc, &yc);
			if(xc >= m_TravelMap.GetWidth())
				xc = m_TravelMap.GetWidth()-1;
			if(yc >= m_TravelMap.GetHeight())
				yc = m_TravelMap.GetHeight()-1;
			if(xc < 0)
				xc = 0;
			if(yc < 0)
				yc = 0;
			CString csCoords;
			z = m_TravelMap.GetMapHeight(xc, yc);
			csCoords.Format("%d,%d,%d", xc, yc, z);
			m_csCoords.SetWindowText(csCoords);
			Recenter(xc, yc);
	}
}

BOOL CTravelTab::OnSetActive() 
{
	UpdateData();
	if ( Main->m_pScripts->m_iACatSeq != m_iCatSeq )
		this->FillCategoryTree();
	WORD wFlags = 0;
	if ( Main->m_dwDrawStatics )
		wFlags |= 0x01;
	if ( Main->m_dwDrawDifs )
		wFlags |= 0x02;
	m_TravelMap.SetDrawFlags(wFlags);
	Recenter();

	return CDockingPage::OnSetActive();
}

void CTravelTab::UpdateSpawnDisplay()
{
	CString csPlane;
	m_ceMapPlane.GetWindowText(csPlane);
	int M = atoi(csPlane);
	m_TravelMap.RemoveDrawObjects();
	short sSize = (m_TravelMap.GetZoomLevel()+6)/2;
	for ( int i = 0; i < Main->m_pScripts->m_asaSPAWNchar.GetSize(); i++ )
	{
		CString csSpawn, csTemp, csX, csY, csZ, csM;
		csSpawn = Main->m_pScripts->m_asaSPAWNchar.GetAt(i);
		csX = csSpawn.SpanExcluding(",");
		csTemp = csSpawn.Mid(csSpawn.Find(",") + 1);
		csY = csTemp.SpanExcluding(",");
		if ( csTemp.Find(",") != -1 )
		{
			csTemp = csTemp.Mid(csTemp.Find(",") + 1);
			csZ = csTemp.SpanExcluding(",");
			if ( csTemp.Find(",") != -1 )
			{
				csTemp = csTemp.Mid(csTemp.Find(",") + 1);
				csM = csTemp.SpanExcluding(",");
			}
		}
		short x = (short) atoi(csX);
		short y = (short) atoi(csY);
		short m = (short) atoi(csM);
		if (m == M)
			m_TravelMap.AddDrawObject(x, y, 1, sSize, Main->m_dwNPCSpawnColor);
	}
	for ( int j = 0; j < Main->m_pScripts->m_asaSPAWNitem.GetSize(); j++ )
	{
		CString csSpawn, csTemp, csX, csY, csZ, csM;
		csSpawn = Main->m_pScripts->m_asaSPAWNitem.GetAt(j);
		csX = csSpawn.SpanExcluding(",");
		csTemp = csSpawn.Mid(csSpawn.Find(",") + 1);
		csY = csTemp.SpanExcluding(",");
		if ( csTemp.Find(",") != -1 )
		{
			csTemp = csTemp.Mid(csTemp.Find(",") + 1);
			csZ = csTemp.SpanExcluding(",");
			if ( csTemp.Find(",") != -1 )
			{
				csTemp = csTemp.Mid(csTemp.Find(",") + 1);
				csM = csTemp.SpanExcluding(",");
			}
		}
		short x = (short) atoi(csX);
		short y = (short) atoi(csY);
		short m = (short) atoi(csM);
		if (m == M)
			m_TravelMap.AddDrawObject(x, y, 1, sSize, Main->m_dwItemSpawnColor);
	}

}

// ---- Live search over all locations (see SearchScriptObjects() in Common.cpp) ----

void CTravelTab::FillList(CPtrArray & aObjects)
{
	m_clcLocations.SetRedraw(FALSE);
	m_clcLocations.SetHotItem(-1);
	m_clcLocations.DeleteAllItems();
	m_csCoords.SetWindowText("");
	m_cbRemoveLoc.EnableWindow(FALSE);
	m_cbEditLoc.EnableWindow(FALSE);
	for (int i = 0; i < aObjects.GetSize(); i++)
	{
		CSObject * pObject = (CSObject *) aObjects[i];
		m_clcLocations.InsertItem(i, pObject->m_csDescription, 0);
		m_clcLocations.SetItemText(i, 1, pObject->m_csID);
		m_clcLocations.SetItemText(i, 2, pObject->m_csDisplay);
		m_clcLocations.SetItemText(i, 3, ScriptObjectCategory(pObject));
		m_clcLocations.SetItemData(i, (DWORD_PTR) pObject);
	}
	m_clcLocations.SetRedraw(TRUE);
	m_clcLocations.Invalidate();
}

void CTravelTab::OnFindarea()
{
	// Clear the search and return focus to the search field.
	KillTimer(1);
	m_bIgnoreSearchChange = true;
	m_ceSearch.SetWindowText("");
	m_bIgnoreSearchChange = false;
	RunSearch();
	m_ceSearch.SetFocus();
}

void CTravelTab::OnChangeSearch()
{
	if (m_bIgnoreSearchChange)
		return;
	KillTimer(1);
	SetTimer(1, 220, NULL);
}

void CTravelTab::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		RunSearch();
		return;
	}
	CDockingPage::OnTimer(nIDEvent);
}

void CTravelTab::RunSearch()
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
	const int MAX_HITS = 1500;
	CPtrArray aObjects;
	int iTotal = SearchScriptObjects(&Main->m_pScripts->m_olAreas, csQuery, aObjects, MAX_HITS);
	FillList(aObjects);
	CString csInfo;
	if (iTotal == 0)
		csInfo.Format(AXT("Keine Treffer f\xFCr \"%s\""), (LPCTSTR) csQuery);
	else
		csInfo.Format(AXT("%d Treffer f\xFCr \"%s\" - Enter reist hin"), iTotal, (LPCTSTR) csQuery);
	m_csResultInfo.SetWindowText(csInfo);
	if (m_clcLocations.GetItemCount() > 0)
		m_clcLocations.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

BOOL CTravelTab::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->hwnd == m_ceSearch.GetSafeHwnd() )
		{
			if ( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DOWN )
			{
				KillTimer(1);
				RunSearch();
				if ( m_clcLocations.GetItemCount() > 0 )
				{
					m_clcLocations.SetFocus();
					m_clcLocations.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
				return TRUE;
			}
			if ( pMsg->wParam == VK_ESCAPE )
			{
				OnFindarea();
				return TRUE;
			}
		}
		else if ( pMsg->hwnd == m_clcLocations.GetSafeHwnd() && pMsg->wParam == VK_RETURN )
		{
			if ( m_clcLocations.GetNextItem(-1, LVNI_SELECTED) != -1 )
				OnGo();
			return TRUE;
		}
		if ( pMsg->wParam == 'F' && (GetKeyState(VK_CONTROL) & 0x8000) )
		{
			m_ceSearch.SetFocus();
			m_ceSearch.SetSel(0, -1);
			return TRUE;
		}
	}
	return CDockingPage::PreTranslateMessage(pMsg);
}

void CTravelTab::OnAddLoc() 
{
	CAddDestination dlgAdd;
	dlgAdd.m_pCategoryList = &Main->m_pScripts->m_olAreas;

	CString csCoords, csMap;
	m_csCoords.GetWindowText(csCoords);
	m_ceMapPlane.GetWindowText(csMap);
	dlgAdd.m_csXCoord = csCoords.SpanExcluding(",");
	dlgAdd.m_csYCoord = csCoords.Mid(csCoords.Find(",") + 1);
	dlgAdd.m_csZCoord = dlgAdd.m_csYCoord.Mid(dlgAdd.m_csYCoord.Find(",") + 1);
	dlgAdd.m_csYCoord = dlgAdd.m_csYCoord.SpanExcluding(",");
	dlgAdd.m_csPlane = csMap;

	if (dlgAdd.DoModal() == IDOK)
	{
		CString csCategory = dlgAdd.m_csCategory;
		CString csSubsection = dlgAdd.m_csSubsection;
		CString csDescription = dlgAdd.m_csDescription;
		CString csX = dlgAdd.m_csXCoord;
		CString csY = dlgAdd.m_csYCoord;
		CString csZ = dlgAdd.m_csZCoord;
		CString csM = dlgAdd.m_csPlane;
		CString csPoint;
		csPoint.Format("%s,%s,%s", csX, csY, csZ);

		// Make sure an entry with this name doesn't already exist
		CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olAreas, csCategory);
		CSObject * pObj = NULL;
		if (pCategory != NULL)
		{
			CSubsection * pSubsection = FindSubsection(pCategory, csSubsection);
			if (pSubsection != NULL)
			{
				POSITION pos = pSubsection->m_ItemList.GetHeadPosition();
				while ( pos )
				{
					CSObject * pObject = (CSObject *) pSubsection->m_ItemList.GetNext(pos);
					if ( pObject != NULL )
					{
						if ( pObject->m_csDescription == csDescription )
						{
							pObj = pObject;
							break;
						}
						continue;
					}
				}
			}
		}
		
		if (pObj != NULL)
		{
			AfxMessageBox(AXT("An entry with that name already exists."), MB_OK);
			return;
		}
		CSObject * pLoc = new CSObject;
		pLoc->m_bCustom = true;
		pLoc->m_csCategory = csCategory;
		pLoc->m_csSubsection = csSubsection;
		pLoc->m_csDescription = csDescription;
		pLoc->m_bType = TYPE_AREA;
		pLoc->m_csID = csPoint;
		pLoc->m_csDisplay = csM;

		Main->m_pScripts->m_aAreas.Add(pLoc);
		CString csKey;
		csKey.Format("%s\\%s\\%s\\%s",REGKEY_LOCATION, csCategory, csSubsection, csDescription);
		Main->PutRegistryString("Point", csPoint, hRegLocation, csKey);
		Main->PutRegistryString("Map", csM, hRegLocation, csKey);
		pCategory = FindCategory(&Main->m_pScripts->m_olAreas, pLoc->m_csCategory);
		pSubsection = FindSubsection(pCategory, pLoc->m_csSubsection);
		if ( pSubsection->m_ItemList.IsEmpty() )
			pSubsection->m_ItemList.AddHead( pLoc );
		else
			pSubsection->m_ItemList.AddTail( pLoc );

		FillCategoryTree();
		SelectTreeItem(csCategory, csSubsection);
	}
}

void CTravelTab::OnRemoveLoc() 
{
	CString csCategory;
	CString csSubsection;
	CString csDescription;

	HTREEITEM hCategory, hSubsection;
	hSubsection = this->m_ctcCategoryTree.GetSelectedItem();
	if (hSubsection != NULL)
	{
		hCategory = this->m_ctcCategoryTree.GetParentItem(hSubsection);
		if (hCategory == NULL)
			return;
	}

	csCategory = this->m_ctcCategoryTree.GetItemText(hCategory);
	csSubsection = this->m_ctcCategoryTree.GetItemText(hSubsection);
	int iItemSel = this->m_clcLocations.GetNextItem(-1, LVNI_SELECTED);
	if (iItemSel == -1)
		return;
	
	CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olAreas, csCategory, false);
	CSubsection * pSubsection = FindSubsection(pCategory, csSubsection, false);

	CSObject * pLoc = (CSObject *)this->m_clcLocations.GetItemData(iItemSel);

	csDescription = pLoc->m_csDescription;

	POSITION pos = pSubsection->m_ItemList.Find(pLoc);
	if ( pos )
		pSubsection->m_ItemList.RemoveAt(pos);

	for ( int i = 0; i < Main->m_pScripts->m_aAreas.GetSize(); i++ )
	{
		CSObject * pTest = (CSObject *) Main->m_pScripts->m_aAreas.GetAt(i);
		if ( pTest == pLoc )
		{
			Main->m_pScripts->m_aAreas.RemoveAt(i);
			delete pLoc;
			break;
		}
	}
	CString csKey;
	csKey.Format("%s\\%s\\%s\\%s", REGKEY_LOCATION, csCategory, csSubsection, csDescription);
	Main->DeleteRegistryKey(csKey);

	if ( pSubsection->m_ItemList.IsEmpty() )
	{
		csKey.Format("%s\\%s\\%s", REGKEY_LOCATION, csCategory, csSubsection);
		Main->DeleteRegistryKey(csKey);
		POSITION pos = pCategory->m_SubsectionList.Find(pSubsection);
		if ( pos )
			pCategory->m_SubsectionList.RemoveAt(pos);
	}

	if ( pCategory->m_SubsectionList.IsEmpty() )
	{
		csKey.Format("%s\\%s", REGKEY_LOCATION, csCategory);
		Main->DeleteRegistryKey(csKey);
		POSITION pos = Main->m_pScripts->m_olAreas.Find(pCategory);
		if ( pos )
			Main->m_pScripts->m_olAreas.RemoveAt(pos);
	}

	FillCategoryTree();
	SelectTreeItem(csCategory, csSubsection);
}

void CTravelTab::OnEditLoc() 
{
	int iSel = m_clcLocations.GetNextItem(-1, LVNI_SELECTED);
	if (iSel == -1)
		return;

	CSObject * pLoc = (CSObject *) m_clcLocations.GetItemData(iSel);
	if ( !pLoc )
		return;

	CAddDestination dlgAdd;
	dlgAdd.m_pCategoryList = &Main->m_pScripts->m_olAreas;

	dlgAdd.m_csDescription = pLoc->m_csDescription;
	dlgAdd.m_csCategory = pLoc->m_csCategory;
	dlgAdd.m_csSubsection = pLoc->m_csSubsection;

	CString csCoords, csMap;
	m_csCoords.GetWindowText(csCoords);
	m_ceMapPlane.GetWindowTextA(csMap);
	dlgAdd.m_csXCoord = csCoords.SpanExcluding(",");
	dlgAdd.m_csYCoord = csCoords.Mid(csCoords.Find(",") + 1);
	dlgAdd.m_csZCoord = dlgAdd.m_csYCoord.Mid(dlgAdd.m_csYCoord.Find(",") + 1);
	dlgAdd.m_csYCoord = dlgAdd.m_csYCoord.SpanExcluding(",");
	dlgAdd.m_csPlane = csMap;

	if (dlgAdd.DoModal() == IDOK)
	{
		OnRemoveLoc();
		CString csCategory = dlgAdd.m_csCategory;
		CString csSubsection = dlgAdd.m_csSubsection;
		CString csDescription = dlgAdd.m_csDescription;
		CString csX = dlgAdd.m_csXCoord;
		CString csY = dlgAdd.m_csYCoord;
		CString csZ = dlgAdd.m_csZCoord;
		CString csM = dlgAdd.m_csPlane;
		CString csPoint;
		csPoint.Format("%s,%s,%s", csX, csY, csZ);

		// Make sure an entry with this name doesn't already exist
		CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olAreas, csCategory);
		CSObject * pObj = NULL;
		if (pCategory != NULL)
		{
			CSubsection * pSubsection = FindSubsection(pCategory, csSubsection);
			if (pSubsection != NULL)
			{
				POSITION pos = pSubsection->m_ItemList.GetHeadPosition();
				while ( pos )
				{
					CSObject * pObject = (CSObject *) pSubsection->m_ItemList.GetNext(pos);
					if ( pObject != NULL )
					{
						if ( pObject->m_csDescription == csDescription )
						{
							pObj = pObject;
							break;
						}
						continue;
					}
				}
			}
		}
		
		if (pObj != NULL)
		{
			AfxMessageBox(AXT("An entry with that name already exists."), MB_OK);
			return;
		}
		CSObject * pLoc = new CSObject;
		pLoc->m_bCustom = true;
		pLoc->m_csCategory = csCategory;
		pLoc->m_csSubsection = csSubsection;
		pLoc->m_csDescription = csDescription;
		pLoc->m_bType = TYPE_AREA;
		pLoc->m_csID = csPoint;
		pLoc->m_csDisplay = csM;

		Main->m_pScripts->m_aAreas.Add(pLoc);
		CString csKey;
		csKey.Format("%s\\%s\\%s\\%s",REGKEY_LOCATION, csCategory, csSubsection, csDescription);
		Main->PutRegistryString("Point", csPoint, hRegLocation, csKey);
		Main->PutRegistryString("Map", csM, hRegLocation, csKey);
		pCategory = FindCategory(&Main->m_pScripts->m_olAreas, pLoc->m_csCategory);
		pSubsection = FindSubsection(pCategory, pLoc->m_csSubsection);
		if ( pSubsection->m_ItemList.IsEmpty() )
			pSubsection->m_ItemList.AddHead( pLoc );
		else
			pSubsection->m_ItemList.AddTail( pLoc );

		FillCategoryTree();
		SelectTreeItem(csCategory, csSubsection);
	}
}

void CTravelTab::SelectTreeItem(CString csParent, CString csChild)
{
	HTREEITEM hParent = m_ctcCategoryTree.GetFirstVisibleItem();
	HTREEITEM hChild;
	
	while (hParent != NULL)
	{
		CString csPText = m_ctcCategoryTree.GetItemText(hParent);
		if (csPText == csParent)
		{
			hChild = m_ctcCategoryTree.GetChildItem(hParent);
			while (hChild != NULL)
			{
				CString csCText = m_ctcCategoryTree.GetItemText(hChild);
				if (csCText == csChild)
				{
					m_ctcCategoryTree.SelectItem(hChild);
					return;
				}
				hChild = m_ctcCategoryTree.GetNextSiblingItem(hChild);
			}
			// Couldn't find the child...
			m_ctcCategoryTree.SelectItem(hParent);
			return;
		}
		hParent = m_ctcCategoryTree.GetNextSiblingItem(hParent);
	}
}

void CTravelTab::OnWorldMap() 
{
	if (Main->m_dwShowMap)
	{
		if (Main->m_dlgWorldMap)
			delete Main->m_dlgWorldMap;
		Main->m_dlgWorldMap = new CWorldMap;
		Main->m_dlgWorldMap->Create(IDD_WORLDMAP);
		short m = m_TravelMap.GetMapFile();
		short w = m_TravelMap.GetWidth();
		short h = m_TravelMap.GetHeight();
		Main->m_dlgWorldMap->m_WorldMap.LoadMap(m, w, h);

		WORD wFlags = 0;
		if ( Main->m_dwDrawStatics )
			wFlags |= 0x01;
		if ( Main->m_dwDrawDifs )
			wFlags |= 0x02;
		Main->m_dlgWorldMap->m_WorldMap.SetDrawFlags(wFlags);
		short zoom;
		switch ( m )
		{
		case 0:
		case 1:
			zoom = -3;
			break;
		case 2:
		case 3:
		case 4:
			zoom = -2;
			break;
		default:
			zoom = -3;
			break;
		}
		Main->m_dlgWorldMap->m_WorldMap.SetZoomLevel(zoom);
		Main->m_dlgWorldMap->m_WorldMap.SetMapFile(m);
		CString csMap;
		m_ceMapPlane.GetWindowText(csMap);

		if (Main->m_dwShowSpawnpoints)
			Main->m_dlgWorldMap->UpdateSpawnDisplay(atoi(csMap));
		else
			Main->m_dlgWorldMap->m_WorldMap.RemoveDrawObjects();

		CString csTitle;
		csTitle.Format(AXT("Weltkarte - Karte %s (Klick w\xE4hlt die Stelle)"), csMap);
		Main->m_dlgWorldMap->SetWindowText(csTitle);

		Main->m_dlgWorldMap->UpdateSize();
		Main->m_dlgWorldMap->m_WorldMap.UpdateMap();
	}
}

void CTravelTab::OnLocate()
{	
	Main->bMemoryAccess = AdjustPrivileges();
	DWORD dwProcessID;
	HANDLE m_hProcess;
	LPVOID lpAddress;
	CString csCoords;
	byte * pointer = new byte[Main->sizeX];
	byte * pointer1 = new byte[Main->sizeY];
	byte * pointer2 = new byte[Main->sizeZ];
	byte * pointer3 = new byte[Main->sizeM];

	if (!Main->calibrated)
		Calibrate();

	if (Main->calibrated)
	{
		EnumWindows(EnumWindowsProc, 1);
		if ((Main->bMemoryAccess) && (hwndUOClient))
		{

			GetWindowThreadProcessId(hwndUOClient, &dwProcessID);
			m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);

			lpAddress = (LPVOID)((DWORD_PTR)Main->LocX);
			ReadProcessMemory(m_hProcess, lpAddress, pointer, Main->sizeX, NULL);
			int x = (short)(pointer[0] | (pointer[1] << 8));

			lpAddress = (LPVOID)((DWORD_PTR)Main->LocY);
			ReadProcessMemory(m_hProcess, lpAddress, pointer1, Main->sizeY, NULL);
			int y = (short)(pointer1[0] | (pointer1[1] << 8));

			lpAddress = (LPVOID)((DWORD_PTR)Main->LocZ);
			ReadProcessMemory(m_hProcess, lpAddress, pointer2, Main->sizeZ, NULL);
			int z = (short)(pointer2[0] | (pointer2[1] << 8));

			lpAddress = (LPVOID)((DWORD_PTR)Main->LocM);
			ReadProcessMemory(m_hProcess, lpAddress, pointer3, Main->sizeM, NULL);
			int m = (byte)pointer3[0];

			csCoords.Format("%d",m);
			m_ceMapPlane.SetWindowText(csCoords);

			Recenter((short)x,(short)y,(short)z);
			CloseHandle( m_hProcess );
		}
		else
			Main->calibrated = false;
	}
}

void CTravelTab::OnTrack()
{	
	Main->bMemoryAccess = AdjustPrivileges();
	UpdateData();
	if (!m_bTrack)
	{
		Main->calibrated = false;
		return;
	}

	if (!Main->calibrated)
		Calibrate();

	if (Main->calibrated)
		AfxBeginThread(TrackingThread,NULL);
}

void CTravelTab::OnRclickLocations(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMITEMACTIVATE * pInfo = (NMITEMACTIVATE *)pNMHDR;
	CRect rListCtrl;
	m_clcLocations.GetWindowRect(&rListCtrl);

	HMENU hMenu = ::CreatePopupMenu();
	if (NULL != hMenu)
	{
		if (m_iCurState == 1)
			::AppendMenu(hMenu, MF_STRING|MF_USECHECKBITMAPS|MF_UNCHECKED, 1, AXT("Show Quicklist"));
		else
			::AppendMenu(hMenu, MF_STRING|MF_USECHECKBITMAPS|MF_CHECKED, 2, AXT("Show Quicklist"));

		if (pInfo->iItem != -1)
		{
			::AppendMenu(hMenu, MF_MENUBREAK, NULL, NULL);
			::AppendMenu(hMenu, MF_STRING, 5, AXT("View Area Info"));
			CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(pInfo->iItem);
			if ( !pObject->m_bCustom )
				::AppendMenu(hMenu, MF_STRING, 6, AXT("Edit Area Script"));
			::AppendMenu(hMenu, MF_MENUBREAK, NULL, NULL);
			if (m_iCurState != 2)
				::AppendMenu(hMenu, MF_STRING, 3, AXT("Add to Quicklist"));
			else
				::AppendMenu(hMenu, MF_STRING, 4, AXT("Remove from Quicklist"));
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
				m_iCurState = 2;
				if (m_clcLocations.GetView() != 0)
				{
					m_clcLocations.SetView(2);
					m_clcLocations.SetColumnWidth(0,150);
				}
				OnShowQuicklist();
			break;
			}
		case 2:
			{
				m_iCurState = 1;
				if (m_clcLocations.GetView() != 0)
					m_clcLocations.SetView(1);
				OnExitQuicklist();
			break;
			}
		case 3:
			{
				CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				if (Main->m_pScripts->m_AreaQuickList.Find(pObject->m_csValue) != -1)
				{
					CString csMessage;
					csMessage.Format(AXT("%s is already in the Quicklist"),pObject->m_csDescription );
					AfxMessageBox(csMessage);
					return;
				}
				Main->m_pScripts->m_AreaQuickList.Insert(pObject);
				OnSaveQuicklist("Area"); 
			break;
			}
		case 4:
			{
				CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				Main->m_pScripts->m_AreaQuickList.RemoveAt(Main->m_pScripts->m_AreaQuickList.Find(pObject->m_csValue));
				this->m_clcLocations.DeleteItem(pInfo->iItem);
				iLocSort = 1;
				iCoordSort = 1;
				this->m_clcLocations.SortItems(CompareFunc, 0);
				iLocSort = -1;
				OnSaveQuicklist("Area");
			break;
			}
		case 5:
			{
				CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(pInfo->iItem);
				if ( !pObject )
					return;
				CScriptInfo * dlg = new CScriptInfo;
				dlg->pObject = pObject;
				dlg->Create(IDD_SCRIPT_INFO);
			break;
			}
		case 6:
			{
				CSObject * pObject = (CSObject *) this->m_clcLocations.GetItemData(pInfo->iItem);
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

void CTravelTab::OnSaveQuicklist(CString csList) 
{
	CStringArray aQuicklist;
	for(int iCount = 0; iCount < Main->m_pScripts->m_AreaQuickList.GetCount(); iCount++)
	{
		CSObject * pObject = (CSObject *) Main->m_pScripts->m_AreaQuickList.GetAt(iCount);
		aQuicklist.Add(pObject->m_csValue);
	}
	// Also write an empty list so a removed last entry does not come back.
	CString csProfileKey;
	csProfileKey.Format("%s\\%s\\Quicklist",REGKEY_PROFILE, Main->m_csCurentProfile);
	Main->PutRegistryMultiSz(csList, &aQuicklist, hRegLocation, csProfileKey);
}

void CTravelTab::OnShowQuicklist() 
{
	m_clcLocations.SetHotItem(-1);
	m_clcLocations.DeleteAllItems();
	m_csCoords.SetWindowText("");

	for(int iCount = 0; iCount < Main->m_pScripts->m_AreaQuickList.GetCount(); iCount++)
	{
		CSObject * pObject = (CSObject *) Main->m_pScripts->m_AreaQuickList.GetAt(iCount);
		this->m_clcLocations.InsertItem(iCount, pObject->m_csDescription, iCount);
		this->m_clcLocations.SetItemData(iCount, (DWORD_PTR)pObject);
	}

	iLocSort = 1;
	iCoordSort = 1;
	this->m_clcLocations.SortItems(CompareFunc, 0);
	iLocSort = -1;
}

void CTravelTab::OnExitQuicklist() 
{
	HTREEITEM hSelectedItem = NULL;
	HTREEITEM hParentItem = NULL;

	m_clcLocations.SetHotItem(-1);
	m_clcLocations.DeleteAllItems();
	m_csCoords.SetWindowText("");

	hSelectedItem = this->m_ctcCategoryTree.GetSelectedItem();
	hParentItem = this->m_ctcCategoryTree.GetParentItem(hSelectedItem);

	if (hParentItem != NULL)
	{
		CString csParent = this->m_ctcCategoryTree.GetItemText(hParentItem);
		CString csSelected = this->m_ctcCategoryTree.GetItemText(hSelectedItem);
		CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olAreas, csParent);
		if (pCategory == NULL)
			return;
		CSubsection * pSubsection = FindSubsection(pCategory, csSelected);
		if (!pSubsection->m_ItemList.IsEmpty())
		{
			POSITION pos = pSubsection->m_ItemList.GetHeadPosition();
			int iCount = 0;
			while (pos != NULL)
			{
				CSObject * pObject = (CSObject *) pSubsection->m_ItemList.GetNext(pos);
				this->m_clcLocations.InsertItem(iCount, pObject->m_csDescription, iCount);
				CString cs_ItemID = pObject->m_csID;
				cs_ItemID.MakeLower();
				this->m_clcLocations.SetItemText(iCount, 1, cs_ItemID);
				this->m_clcLocations.SetItemData(iCount, (DWORD_PTR)pObject);
				iCount++;
			}
		}
	}

	iLocSort = 1;
	iCoordSort = 1;
	this->m_clcLocations.SortItems(CompareFunc, 0);
	iLocSort = -1;
}