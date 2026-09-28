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

// CSpawnTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "SpawnTab.h"
#include "UOart.h"
#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpawnTab property page

IMPLEMENT_DYNCREATE(CSpawnTab, CDockingPage)

int CSpawnTab::iIDSort = 1;
int CSpawnTab::iNameSort = 1;

CSpawnTab::CSpawnTab() : CDockingPage(CSpawnTab::IDD,CMsg("IDS_SPAWN"))
{
	//{{AFX_DATA_INIT(CSpawnTab)
	m_iCatSeq = 0;
	m_bIgnoreSearchChange = false;
	m_iItemMapSeq = -1;
	icX = 60;
	icY = 110;
	//}}AFX_DATA_INIT
}

CSpawnTab::~CSpawnTab()
{
}

void CSpawnTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpawnTab)
	DDX_Control(pDX, IDC_CREATURELIST, m_clcCreatures);
	DDX_Control(pDX, IDC_NPCTREE, m_ctcNPC);
	DDX_Control(pDX, IDC_HOMEDIST, m_ceHomedist);
	DDX_Control(pDX, IDC_NPC_ID, m_csNPCId);
	DDX_Control(pDX, IDC_NPC_IDDEC, m_csNPCIdDec);
	DDX_Control(pDX, IDC_MIN_TIME, m_ceMinTime);
	DDX_Control(pDX, IDC_MAX_TIME, m_ceMaxTime);
	DDX_Control(pDX, IDC_MAX_DIST, m_ceMaxDist);
	DDX_Control(pDX, IDC_AMOUNT, m_ceAmount);
	DDX_Control(pDX, IDC_NPCDISPLAY, m_Display);
	DDX_Control(pDX, IDC_FRAME_SELECT, m_FrameSelect);
	DDX_Control(pDX, IDC_FINDNPC, cb_findnpc);
	DDX_Control(pDX, IDC_NPCSEARCH, m_ceSearch);
	DDX_Control(pDX, IDC_NPC_DETAILS, m_csDetails);
	DDX_Control(pDX, IDC_NPC_RESULTINFO, m_csResultInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpawnTab, CDockingPage)
	//{{AFX_MSG_MAP(CSpawnTab)
	ON_NOTIFY(TVN_SELCHANGED, IDC_NPCTREE, OnSelchangedNpctree)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CREATURELIST, OnColumnclickCreatures)
	ON_NOTIFY(NM_DBLCLK, IDC_CREATURELIST, OnDblclkCreatures)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CREATURELIST, OnItemchangedCreatures)
	ON_NOTIFY(NM_RCLICK, IDC_CREATURELIST, OnRclickCreatures)
	ON_BN_CLICKED(IDC_SUMMON, OnSummon)
	ON_BN_CLICKED(IDC_PLACE_SPAWNPOINT, OnPlacespawn)
	ON_BN_CLICKED(IDC_INIT_SPAWNPOINT, OnInitspawn)
	ON_BN_CLICKED(IDC_SETHOME, OnSethome)
	ON_BN_CLICKED(IDC_SETHOMEDIST, OnSethomedist)
	ON_BN_CLICKED(IDC_SPREMOVE, OnSpremove)
	ON_BN_CLICKED(IDC_SPFREEZE, OnSpfreeze)
	ON_BN_CLICKED(IDC_SHRINK, OnShrink)
	ON_EN_CHANGE(IDC_HOMEDIST, OnChangeParams)
	ON_EN_CHANGE(IDC_AMOUNT, OnChangeParams)
	ON_EN_CHANGE(IDC_MIN_TIME, OnChangeParams)
	ON_EN_CHANGE(IDC_MAX_TIME, OnChangeParams)
	ON_EN_CHANGE(IDC_MAX_DIST, OnChangeParams)
	ON_EN_CHANGE(IDC_NPCSEARCH, OnChangeSearch)
	ON_BN_CLICKED(IDC_FINDNPC, OnFindnpc)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpawnTab message handlers

BOOL CSpawnTab::OnInitDialog()
{
	m_dcPropertyPage = Main->m_pcppSpawnTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CSpawnTab;
	Main->m_pcppSpawnTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();
	AxisMarkPrimary(this, IDC_SUMMON);	// Primary action

	CString csValue;
	csValue = Main->GetRegistryString("NPCSpawnAmount", "1");
	m_ceAmount.SetWindowText(csValue);
	csValue = Main->GetRegistryString("NPCHomedist", "5");
	m_ceHomedist.SetWindowText(csValue);
	csValue = Main->GetRegistryString("NPCSpawnMaxDist", "0");
	m_ceMaxDist.SetWindowText(csValue);
	csValue = Main->GetRegistryString("NPCSpawnMaxTime", "20");
	m_ceMaxTime.SetWindowText(csValue);
	csValue = Main->GetRegistryString("NPCSpawnMinTime", "5");
	m_ceMinTime.SetWindowText(csValue);

	this->m_clcCreatures.InsertColumn(0, AXT("Name"), LVCFMT_LEFT, 170, -1);
	this->m_clcCreatures.InsertColumn(1, AXT("Defname"), LVCFMT_LEFT, 150, -1);
	this->m_clcCreatures.InsertColumn(2, AXT("K\xF6rper"), LVCFMT_LEFT, 60, -1);
	this->m_clcCreatures.InsertColumn(3, AXT("Kategorie"), LVCFMT_LEFT, 190, -1);
	m_clcCreatures.SetExtendedStyle(m_clcCreatures.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	ApplyAxisListHeader(m_clcCreatures.GetSafeHwnd());	// Dark column header
	AxisSetCue(m_ceSearch.GetSafeHwnd(), "z.B.  ork   c_orc   0x11   wache elf");
	m_csResultInfo.SetWindowText(AXT("Kategorie links w\xE4hlen oder oben suchen"));
	FillCategoryTree();

	m_pImageList = new CImageList();
	m_pImageList->Create(icX, icY, ILC_COLORDDB, 0, 1);
	m_clcCreatures.SetImageList(m_pImageList, LVSIL_NORMAL);
	m_iCurState = m_clcCreatures.GetView();

	m_Display.SetArtType(3);
	m_Display.m_bLayeredNpc = true;	// Large preview with equipment
	m_Display.SetArtIndex(-1);
	m_Display.SetBkColor(Main->m_dwSpawnBGColor);

	m_FrameSelect.SetRange(-4, 3);
	m_FrameSelect.SetPos(-1);
	m_Display.m_wFrame = -1;
	UpdateDetails(NULL);

	m_tip.Create(this);
	m_tip.SetMaxTipWidth(360);
	m_tip.AddTool(GetDlgItem(IDC_SUMMON), AXT("add <defname> (bzw. addnpc <nummer>) - dann Ort im Spiel anklicken"));
	m_tip.AddTool(GetDlgItem(IDC_SPREMOVE), AXT("remove - NPC anklicken"));
	m_tip.AddTool(GetDlgItem(IDC_SHRINK), AXT("shrink - NPC wird zur Figur"));
	m_tip.AddTool(GetDlgItem(IDC_SPFREEZE), AXT("set stone - schaltet Versteinern an/aus: NPC steht still, ist nicht angreifbar und grau. Andere Flags bleiben."));
	m_tip.AddTool(GetDlgItem(IDC_SETHOME), AXT("set home - Zuhause des NPCs auf seinen Standort"));
	m_tip.AddTool(GetDlgItem(IDC_SETHOMEDIST), AXT("set homedist <felder>"));
	m_tip.AddTool(GetDlgItem(IDC_PLACE_SPAWNPOINT), AXT("add 01ea7 - Spawnstein setzen"));
	m_tip.AddTool(GetDlgItem(IDC_INIT_SPAWNPOINT), AXT("act.type/amount/more/morep/attr/timer auf den zuletzt gesetzten Spawnstein"));
	m_tip.AddTool(GetDlgItem(IDC_FRAME_SELECT), AXT("Blickrichtung der Vorschau"));
	m_tip.Activate(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpawnTab::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->m_hWnd == m_FrameSelect.m_hWnd)
	{
		m_Display.m_wFrame = m_FrameSelect.GetPos();
		m_Display.ReloadArt();
		return;
	}
	CDockingPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSpawnTab::OnSummon()
{
	CString csCmd, csID;
	m_csNPCId.GetWindowText(csID);

	int iSelIndex = this->m_clcCreatures.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1 || csID == "")
	{
		AxisSetStatus(AXT("Erst einen NPC in der Liste w\xE4hlen."), 2);
		return;
	}
	CSObject * pObject = (CSObject *) this->m_clcCreatures.GetItemData(iSelIndex);
	if ( pObject && pObject->m_bType == TYPE_SPAWN )
	{
		AxisSetStatus(AXT("Spawngruppen lassen sich nicht beschw\xF6ren - \xFC" "ber einen Spawnpunkt einrichten."), 2);
		return;
	}
	if (IsNumber(csID))
		csCmd.Format("%saddnpc %s", Main->m_csCommandPrefix, csID);
	else
		csCmd.Format("%sadd %s", Main->m_csCommandPrefix, csID);
	if (SendToUO(csCmd) && pObject)
		AxisRememberRecent(pObject->m_csDescription + " (NPC)", csCmd);	// For the dashboard's recent list
}


void CSpawnTab::OnSethome()
{
	CString csCmd;
	csCmd.Format("%sset home", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}


void CSpawnTab::OnSethomedist()
{
	CString csVal;
	m_ceHomedist.GetWindowText(csVal);
	CString csCmd;
	csCmd.Format("%sset homedist %s", Main->m_csCommandPrefix, csVal);
	SendToUO(csCmd);
}


void CSpawnTab::OnShrink()
{
	CString csCmd;
	csCmd.Format("%sshrink", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}


void CSpawnTab::OnSpremove()
{
	CString csCmd;
	csCmd.Format("%sremove", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}


void CSpawnTab::OnSpfreeze()
{
	CString csCmd;
	// "set stone" toggles only the stone flag and leaves other flags intact.
	csCmd.Format("%sset stone", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}


void CSpawnTab::OnPlacespawn()
{
	CString csCmd;
	csCmd.Format("%sadd 01ea7", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CSpawnTab::OnInitspawn()
{
	CWaitCursor hourglass;
	CString csAmount, csMinTime, csMaxTime, csMaxDist, csID;
	this->m_ceAmount.GetWindowText(csAmount);
	this->m_ceMaxDist.GetWindowText(csMaxDist);
	this->m_ceMinTime.GetWindowText(csMinTime);
	this->m_ceMaxTime.GetWindowText(csMaxTime);
	this->m_csNPCId.GetWindowText(csID);

	if (csID == "")
	{
		AxisSetStatus(AXT("Erst einen NPC in der Liste w\xE4hlen."), 2);
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
	csCmd.Format("%sact.type %ld", Main->m_csCommandPrefix, ITEM_SPAWN_CHAR);
	SendToUO(csCmd);

	csCmd.Format("%sact.amount %ld", Main->m_csCommandPrefix, iAmount);
	SendToUO(csCmd);

	csCmd.Format("%sact.more %s", Main->m_csCommandPrefix, csID);
	SendToUO(csCmd);

	csCmd.Format("%sact.morep %ld %ld %ld", Main->m_csCommandPrefix, iMinTime, iMaxTime, iMaxDist);
	SendToUO(csCmd);

	csCmd.Format("%sact.attr %04x", Main->m_csCommandPrefix, ATTR_INVIS | ATTR_MAGIC | ATTR_MOVE_NEVER);
	SendToUO(csCmd);

	csCmd.Format("%sact.timer 1", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}


void CSpawnTab::OnChangeParams()
{
	UpdateData();
	if ( !this->IsWindowVisible() )
		return;

	CString csAmount, csMinTime, csMaxTime, csHomedist, csDist;

	m_ceAmount.GetWindowText(csAmount);
	m_ceHomedist.GetWindowText(csHomedist);
	m_ceMaxDist.GetWindowText(csDist);
	m_ceMaxTime.GetWindowText(csMaxTime);
	m_ceMinTime.GetWindowText(csMinTime);

	Main->PutRegistryString("NPCSpawnAmount", csAmount);
	Main->PutRegistryString("NPCHomedist", csHomedist);
	Main->PutRegistryString("NPCSpawnMaxDist", csDist);
	Main->PutRegistryString("NPCSpawnMaxTime", csMaxTime);
	Main->PutRegistryString("NPCSpawnMinTime", csMinTime);
}


//***************************************************

void CSpawnTab::FillCategoryTree()
{
	HTREEITEM CategoryParent;
	TV_INSERTSTRUCT InsertItem;

	m_ctcNPC.SetRedraw(FALSE); // fill in one batch - no repaint per TVI_SORT insert
	m_clcCreatures.DeleteAllItems();
	m_ctcNPC.DeleteAllItems();
	if (!Main->m_pScripts->m_olNPCs.IsEmpty())
	{
		POSITION pos = Main->m_pScripts->m_olNPCs.GetHeadPosition();
		while (pos != NULL)
		{
			CCategory * pCategory = (CCategory *) Main->m_pScripts->m_olNPCs.GetNext(pos);
			InsertItem.item.mask = TVIF_TEXT;
			InsertItem.item.pszText = (char *)LPCTSTR(pCategory->m_csName);
			InsertItem.item.cchTextMax = pCategory->m_csName.GetLength();
			InsertItem.hParent = NULL;
			InsertItem.hInsertAfter = TVI_SORT;
			CategoryParent = this->m_ctcNPC.InsertItem(&InsertItem);
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
					this->m_ctcNPC.InsertItem(&InsertItem);
				}
			}
		}
	}
	m_ctcNPC.SetRedraw(TRUE);
	m_iCatSeq = Main->m_pScripts->m_iNCatSeq;
}

// Fills the NPC list; shared by category selection, search and quick list.
void CSpawnTab::FillList(CPtrArray & aObjects)
{
	m_clcCreatures.SetRedraw(FALSE);
	m_clcCreatures.SetHotItem(-1);
	m_clcCreatures.DeleteAllItems();
	m_pImageList->Remove(-1);
	m_pImageList->SetImageCount((UINT)aObjects.GetSize());
	UpdateDetails(NULL);

	for (int iCount = 0; iCount < aObjects.GetSize(); iCount++)
	{
		CSObject * pObject = (CSObject *) aObjects[iCount];
		this->m_clcCreatures.InsertItem(iCount, pObject->m_csDescription, iCount);
		CString cs_ID = pObject->m_csID;
		cs_ID.MakeLower();
		this->m_clcCreatures.SetItemText(iCount, 1, cs_ID);
		this->m_clcCreatures.SetItemData(iCount, (DWORD_PTR)pObject);

		DWORD dwDisplay = alltoi(pObject->m_csDisplay,pObject->m_csValue);
		WORD wID = ( dwDisplay != 0 ) ? (WORD) dwDisplay : (WORD) -1;
		DWORD dwColor = alltoi(pObject->m_csColor,pObject->m_csValue);
		WORD wColor = ( dwColor != 0 ) ? (WORD) dwColor : 0;
		if ( wID != (WORD) -1 )
		{
			CString csBody;
			csBody.Format("0x%04X", wID);
			this->m_clcCreatures.SetItemText(iCount, 2, csBody);
		}
		else if ( pObject->m_bType == TYPE_SPAWN )
			this->m_clcCreatures.SetItemText(iCount, 2, AXT("Gruppe"));
		this->m_clcCreatures.SetItemText(iCount, 3, ScriptObjectCategory(pObject));

		// Icons are expensive; skip them for very large result lists.
		if ( aObjects.GetSize() <= 600 )
		{
			CBitmap *pTempBitmap = m_Icon.OnCreateIcon(icX,icY,wID,wColor,3);
			m_pImageList->Replace(iCount,pTempBitmap,NULL);
			delete pTempBitmap;
		}
	}
	m_clcCreatures.SetRedraw(TRUE);
	m_clcCreatures.Invalidate();
}

void CSpawnTab::OnSelchangedNpctree(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pResult);
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hSelectedItem = this->m_ctcNPC.GetSelectedItem();
	this->m_ctcNPC.SetItemState(hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	HTREEITEM hParentItem = this->m_ctcNPC.GetParentItem(hSelectedItem);

	if (pNMTreeView != NULL)
	{
		HTREEITEM hOldItem = pNMTreeView->itemOld.hItem;
		if (hOldItem != NULL)
			this->m_ctcNPC.SetItemState(hOldItem, 0, TVIS_BOLD);
	}

	m_iCurState = 1;
	if (m_clcCreatures.GetView() != 0)
		m_clcCreatures.SetView(1);

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
		CString csParent = this->m_ctcNPC.GetItemText(hParentItem);
		CString csSelected = this->m_ctcNPC.GetItemText(hSelectedItem);
		CCategory * pCategory = FindCategory(&Main->m_pScripts->m_olNPCs, csParent);
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
		csInfo.Format(AXT("%s / %s: %d NPCs"), (LPCTSTR) csParent, (LPCTSTR) csSelected, (int) aObjects.GetSize());
	}
	else
		csInfo = AXT("Unterbereich w\xE4hlen, um die NPCs zu sehen");
	FillList(aObjects);
	m_csResultInfo.SetWindowText(csInfo);

	iNameSort = 1;
	iIDSort = 1;
	this->m_clcCreatures.SortItems(CompareFunc, 0);
	iNameSort = -1;
}

// Shows details and preview for the selected NPC.
void CSpawnTab::UpdateDetails(CSObject * pObject)
{
	if (pObject == NULL)
	{
		m_csNPCId.SetWindowText("");
		m_csNPCIdDec.SetWindowText("");
		m_csDetails.SetWindowText(AXT("Kein NPC gew\xE4hlt.\n\nEinen NPC in der Liste anklicken - dann beschw\xF6ren oder als Spawnpunkt einrichten."));
		m_Display.ClearEquipment();
		m_Display.SetArtIndex(-1);
		m_Display.SetArtColor(0);
		return;
	}
	CString cs_NPCID = pObject->m_csID;
	cs_NPCID.MakeLower();
	m_csNPCId.SetWindowText(cs_NPCID);

	DWORD dwDisplay = alltoi(pObject->m_csDisplay, pObject->m_csValue);
	WORD wID = ( dwDisplay != 0 ) ? (WORD) dwDisplay : (WORD) -1;
	DWORD dwColor = alltoi(pObject->m_csColor, pObject->m_csValue);
	WORD wColor = ( dwColor != 0 ) ? (WORD) dwColor : 0;

	CString csDetails, csLine;
	csDetails = pObject->m_csDescription;
	csDetails += AXT("\n\nDefname:  ") + cs_NPCID;
	if (wID != (WORD) -1)
	{
		csLine.Format(AXT("\nK\xF6rper:  0x%04X   Farbe:  0x%X"), wID, wColor);
		csDetails += csLine;
	}
	CString csCat = ScriptObjectCategory(pObject);
	if (!csCat.IsEmpty())
		csDetails += AXT("\nKategorie:  ") + csCat;
	if (pObject->m_bType == TYPE_SPAWN)
		csDetails += AXT("\nSpawngruppe - nur \xFC" "ber einen Spawnpunkt nutzbar");
	m_csDetails.SetWindowText(csDetails);

	if (Main->m_dwShowNPCs)
	{
		LoadEquipment(pObject);	// Hair, clothing and weapons
		m_Display.SetArtIndex(wID);
		m_Display.SetArtColor(wColor);
	}
}

void CSpawnTab::OnItemchangedCreatures(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;
	if (!(pNMListView->uNewState & LVNI_SELECTED))
		return;
	int iSelIndex = this->m_clcCreatures.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1)
	{
		UpdateDetails(NULL);
		return;
	}
	this->m_clcCreatures.SetHotItem(iSelIndex);
	CSObject * pObject = (CSObject *) this->m_clcCreatures.GetItemData(iSelIndex);
	if ( !pObject || pObject->m_csFilename == "" )
		return;
	UpdateDetails(pObject);
}

void CSpawnTab::OnColumnclickCreatures(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	this->m_clcCreatures.SortItems(CompareFunc, pNMListView->iSubItem);
	if (pNMListView->iSubItem == 0)
		iNameSort = (iNameSort == 1) ? -1 : 1;
	if (pNMListView->iSubItem == 1)
		iIDSort = (iIDSort == 1) ? -1 : 1;
	*pResult = 0;
}

void CSpawnTab::OnDblclkCreatures(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	if (this->m_clcCreatures.GetNextItem(-1, LVNI_SELECTED) != -1)
		this->OnSummon();
	*pResult = 0;
}


BOOL CSpawnTab::OnSetActive()
{
	UpdateData();
	if ( Main->m_pScripts->m_iNCatSeq != m_iCatSeq )
		this->FillCategoryTree();
	m_Display.SetDrawFlags(0);
	m_Display.SetBkColor(Main->m_dwSpawnBGColor);

	return CDockingPage::OnSetActive();
}

int CALLBACK CSpawnTab::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSObject * pObject1 = (CSObject *) lParam1;
	CSObject * pObject2 = (CSObject *) lParam2;
	if ( !pObject1 || !pObject2 )
		return 0;
	if (lParamSort == 0)
		return pObject1->m_csDescription.CompareNoCase(pObject2->m_csDescription) * (iNameSort == -1 ? -1 : 1);
	return pObject1->m_csID.CompareNoCase(pObject2->m_csID) * (iIDSort == -1 ? -1 : 1);
}

// ---- Live search (logic in SearchScriptObjects(), Common.cpp) ----

void CSpawnTab::OnFindnpc()
{
	// Clear the search and return focus to the search field.
	KillTimer(1);
	m_bIgnoreSearchChange = true;
	m_ceSearch.SetWindowText("");
	m_bIgnoreSearchChange = false;
	RunSearch();
	m_ceSearch.SetFocus();
}

void CSpawnTab::OnChangeSearch()
{
	if (m_bIgnoreSearchChange)
		return;
	KillTimer(1);
	SetTimer(1, 220, NULL);
}

void CSpawnTab::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		RunSearch();
		return;
	}
	CDockingPage::OnTimer(nIDEvent);
}

void CSpawnTab::RunSearch()
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
	int iTotal = SearchScriptObjects(&Main->m_pScripts->m_olNPCs, csQuery, aObjects, MAX_HITS);
	FillList(aObjects);

	CString csInfo;
	if (iTotal == 0)
		csInfo.Format(AXT("Keine Treffer f\xFCr \"%s\""), (LPCTSTR) csQuery);
	else if (iTotal > MAX_HITS)
		csInfo.Format(AXT("%d Treffer f\xFCr \"%s\" - die ersten %d angezeigt, Suche genauer fassen"), iTotal, (LPCTSTR) csQuery, MAX_HITS);
	else
		csInfo.Format(AXT("%d Treffer f\xFCr \"%s\" - Enter/Pfeil runter springt in die Liste"), iTotal, (LPCTSTR) csQuery);
	m_csResultInfo.SetWindowText(csInfo);
	if (m_clcCreatures.GetItemCount() > 0)
		m_clcCreatures.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

BOOL CSpawnTab::PreTranslateMessage(MSG* pMsg)
{
	if (m_tip.GetSafeHwnd())
		m_tip.RelayEvent(pMsg);
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->hwnd == m_ceSearch.GetSafeHwnd() )
		{
			if ( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DOWN )
			{
				KillTimer(1);
				RunSearch();
				if ( m_clcCreatures.GetItemCount() > 0 )
				{
					m_clcCreatures.SetFocus();
					m_clcCreatures.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_clcCreatures.EnsureVisible(0, FALSE);
				}
				return TRUE;
			}
			if ( pMsg->wParam == VK_ESCAPE )
			{
				OnFindnpc();
				return TRUE;
			}
		}
		else if ( pMsg->hwnd == m_clcCreatures.GetSafeHwnd() && pMsg->wParam == VK_RETURN )
		{
			if ( m_clcCreatures.GetNextItem(-1, LVNI_SELECTED) != -1 )
				OnSummon();
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

void CSpawnTab::OnRclickCreatures(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMITEMACTIVATE * pInfo = (NMITEMACTIVATE *)pNMHDR;
	CRect rListCtrl;
	m_clcCreatures.GetWindowRect(&rListCtrl);

	HMENU hMenu = ::CreatePopupMenu();
	if (NULL != hMenu)
	{
		if (m_clcCreatures.GetView() == 0)
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
			::AppendMenu(hMenu, MF_STRING, 7, AXT("NPC-Infos anzeigen"));
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
			m_iCurState = m_clcCreatures.GetView();
			m_clcCreatures.SetView(0);
			break;
		case 2:
			m_clcCreatures.SetView(m_iCurState);
			if (m_iCurState == 2)
				m_clcCreatures.SetColumnWidth(0,150);
			break;
		case 3:
			m_iCurState = 2;
			if (m_clcCreatures.GetView() != 0)
			{
				m_clcCreatures.SetView(2);
				m_clcCreatures.SetColumnWidth(0,150);
			}
			OnShowQuicklist();
			break;
		case 4:
			m_iCurState = 1;
			if (m_clcCreatures.GetView() != 0)
				m_clcCreatures.SetView(1);
			OnExitQuicklist();
			break;
		case 5:
			{
				CSObject * pObject = (CSObject *) this->m_clcCreatures.GetItemData(pInfo->iItem);
				if ( !pObject )
					break;
				if (Main->m_pScripts->m_SpawnQuickList.Find(pObject->m_csValue) != -1)
				{
					CString csMessage;
					csMessage.Format(AXT("%s ist schon in der Schnellliste"), (LPCTSTR) pObject->m_csDescription);
					AxisSetStatus(csMessage, 2);
					break;
				}
				Main->m_pScripts->m_SpawnQuickList.Insert(pObject);
				OnSaveQuicklist("Spawns");
			}
			break;
		case 6:
			{
				CSObject * pObject = (CSObject *) this->m_clcCreatures.GetItemData(pInfo->iItem);
				if ( !pObject )
					break;
				Main->m_pScripts->m_SpawnQuickList.RemoveAt(Main->m_pScripts->m_SpawnQuickList.Find(pObject->m_csValue));
				this->m_clcCreatures.DeleteItem(pInfo->iItem);
				iNameSort = 1;
				this->m_clcCreatures.SortItems(CompareFunc, 0);
				OnSaveQuicklist("Spawns");
			}
			break;
		case 7:
			{
				CSObject * pObject = (CSObject *) this->m_clcCreatures.GetItemData(pInfo->iItem);
				if ( !pObject )
					break;
				CScriptInfo * dlg = new CScriptInfo;
				dlg->pObject = pObject;
				dlg->Create(IDD_SCRIPT_INFO);
			}
			break;
		case 8:
			{
				CSObject * pObject = (CSObject *) this->m_clcCreatures.GetItemData(pInfo->iItem);
				if ( !pObject )
					break;
				CScriptEditor * dlg = new CScriptEditor;
				dlg->pObject = pObject;
				dlg->Create(IDD_SCRIPT_EDITOR);
			}
			break;
		}
		::DestroyMenu(hMenu);
	}
	*pResult = 0;
}

void CSpawnTab::OnSaveQuicklist(CString csList)
{
	CStringArray aQuicklist;
	for(int iCount = 0; iCount < Main->m_pScripts->m_SpawnQuickList.GetCount(); iCount++)
	{
		CSObject * pObject = (CSObject *) Main->m_pScripts->m_SpawnQuickList.GetAt(iCount);
		aQuicklist.Add(pObject->m_csValue);
	}
	// Write even an empty list so the last removed entry does not come back.
	CString csProfileKey;
	csProfileKey.Format("%s\\%s\\Quicklist",REGKEY_PROFILE, Main->m_csCurentProfile);
	Main->PutRegistryMultiSz(csList, &aQuicklist, hRegLocation, csProfileKey);
}

void CSpawnTab::OnShowQuicklist()
{
	CPtrArray aObjects;
	for(int iCount = 0; iCount < Main->m_pScripts->m_SpawnQuickList.GetCount(); iCount++)
		aObjects.Add(Main->m_pScripts->m_SpawnQuickList.GetAt(iCount));
	FillList(aObjects);
	CString csInfo;
	csInfo.Format(AXT("Schnellliste: %d NPCs (Rechtsklick > Zur Schnellliste hinzuf\xFCgen)"), (int) aObjects.GetSize());
	m_csResultInfo.SetWindowText(csInfo);

	iNameSort = 1;
	iIDSort = 1;
	this->m_clcCreatures.SortItems(CompareFunc, 0);
	iNameSort = -1;
}

void CSpawnTab::OnExitQuicklist()
{
	if (m_ceSearch.GetWindowTextLength() > 0)
		RunSearch();
	else
	{
		LRESULT lResult = 0;
		OnSelchangedNpctree(NULL, &lResult);
	}
}

// ---- NPC equipment for the preview ----
// Reads ITEM=/ITEMNEWBIE= (+ COLOR=) from the CHARDEF's ON=@Create block and
// resolves each item to its animation and layer; unresolvable entries are skipped.

static CString SphereFirstToken(CString csValue)
{
	int iComment = csValue.Find("//");
	if (iComment != -1)
		csValue = csValue.Left(iComment);
	csValue.Trim();
	csValue.TrimLeft("{");
	csValue.Trim();
	return csValue.SpanExcluding(" \t,}");
}

// Sphere number: 0x.. or leading 0 = hex, else decimal; anything else = 0.
static WORD SphereNumber(const CString & csValue)
{
	if (csValue.IsEmpty())
		return 0;
	CString cs = csValue;
	cs.MakeLower();
	if (cs.Left(2) == "0x")
		return (WORD) strtoul(cs.Mid(2), NULL, 16);
	if (cs.SpanIncluding("0123456789abcdef") != cs)
		return 0;
	if (cs[0] == '0')
		return (WORD) strtoul(cs, NULL, 16);
	if (cs.SpanIncluding("0123456789") != cs)
		return 0;
	return (WORD) strtoul(cs, NULL, 10);
}

void CSpawnTab::BuildItemMap()
{
	if (m_iItemMapSeq == Main->m_pScripts->m_iICatSeq && m_mapItemDefs.GetCount() > 0)
		return;
	m_mapItemDefs.RemoveAll();
	m_mapItemDefs.InitHashTable(65521);
	POSITION posCat = Main->m_pScripts->m_olItems.GetHeadPosition();
	while (posCat != NULL)
	{
		CCategory * pCat = (CCategory *) Main->m_pScripts->m_olItems.GetNext(posCat);
		POSITION posSub = pCat->m_SubsectionList.GetHeadPosition();
		while (posSub != NULL)
		{
			CSubsection * pSub = (CSubsection *) pCat->m_SubsectionList.GetNext(posSub);
			POSITION posItem = pSub->m_ItemList.GetHeadPosition();
			while (posItem != NULL)
			{
				CSObject * pObj = (CSObject *) pSub->m_ItemList.GetNext(posItem);
				CString csKey = pObj->m_csID;
				csKey.MakeLower();
				if (!csKey.IsEmpty())
					m_mapItemDefs.SetAt(csKey, pObj);
				csKey = pObj->m_csValue;
				csKey.MakeLower();
				if (!csKey.IsEmpty())
					m_mapItemDefs.SetAt(csKey, pObj);
			}
		}
	}
	m_iItemMapSeq = Main->m_pScripts->m_iICatSeq;
}

void CSpawnTab::LoadEquipment(CSObject * pObject)
{
	CArray<CUOArt::CAnimLayer, CUOArt::CAnimLayer&> aEquip;
	if (pObject == NULL || pObject->m_bType == TYPE_SPAWN || pObject->m_csFilename.IsEmpty())
	{
		m_Display.SetEquipment(aEquip);
		return;
	}

	CStringArray aItems;
	CWordArray aColors;
	CStdioFile fScript;
	if (fScript.Open(pObject->m_csFilename, CFile::modeRead | CFile::shareDenyNone | CFile::typeText))
	{
		CString csWanted;
		csWanted.Format("[CHARDEF %s]", (LPCTSTR) pObject->m_csValue);
		CString csLine;
		bool bInBlock = false, bInCreate = false;
		while (fScript.ReadString(csLine))
		{
			csLine.Trim();
			if (!bInBlock)
			{
				if (csLine.CompareNoCase(csWanted) == 0)
					bInBlock = true;
				continue;
			}
			if (csLine.Left(1) == "[")
				break;
			CString csUpper = csLine;
			csUpper.MakeUpper();
			if (csUpper.Left(4) == "ON=@")
			{
				bInCreate = (csUpper.Mid(4).SpanExcluding(" \t/") == "CREATE");
				continue;
			}
			if (!bInCreate || csLine.Find('=') == -1)
				continue;
			CString csTag = csUpper.SpanExcluding("=");
			csTag.Trim();
			CString csValue = csLine.Mid(csLine.Find('=') + 1);
			if (csTag == "ITEM" || csTag == "ITEMNEWBIE")
			{
				aItems.Add(SphereFirstToken(csValue));
				aColors.Add(0);
			}
			else if (csTag == "COLOR" && aItems.GetSize() > 0)
				aColors[aColors.GetSize() - 1] = SphereNumber(SphereFirstToken(csValue));
		}
		fScript.Close();
	}

	if (aItems.GetSize() > 0)
		BuildItemMap();
	for (int i = 0; i < aItems.GetSize(); i++)
	{
		CString csKey = aItems[i];
		csKey.MakeLower();
		WORD wGraphic = 0;
		void * pFound = NULL;
		if (m_mapItemDefs.Lookup(csKey, pFound) && pFound != NULL)
		{
			CSObject * pItem = (CSObject *) pFound;
			wGraphic = (WORD) alltoi(pItem->m_csDisplay, pItem->m_csValue);
			if (wGraphic == 0)
				wGraphic = SphereNumber(pItem->m_csValue);
		}
		else
			wGraphic = SphereNumber(csKey);	// e.g. ITEM=0203b
		if (wGraphic == 0)
			continue;
		CUOArt::CItemTileInfo info;
		if (!m_Icon.GetItemTileInfo(wGraphic, info) || info.wAnim == 0)
			continue;
		if (info.bLayer == 0 || info.bLayer > 24 || info.bLayer == 21)	// No backpack/mount
			continue;
		CUOArt::CAnimLayer layer;
		layer.wAnim = info.wAnim;
		layer.wColor = aColors[i];
		layer.bLayer = info.bLayer;
		aEquip.Add(layer);
	}
	m_Display.SetEquipment(aEquip);
}
