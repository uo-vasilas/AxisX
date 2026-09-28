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

// ItemTweakTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "ItemTweakTab.h"
#include "ColorSelectionDlg.h"
#include "UOart.h"
#include "DoorWizard.h"
#include "LightWizard.h"
//#include "Random.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemTab property page

IMPLEMENT_DYNCREATE(CItemTweakTab, CDockingPage)

CItemTweakTab::CItemTweakTab() : CDockingPage(CItemTweakTab::IDD,CMsg("IDS_ITEM_TWEAK"))
{
	//{{AFX_DATA_INIT(CItemTweakTab)
	m_iTypeCount = 0;
	m_bIdentified = FALSE;
	m_bDecay = FALSE;
	m_bNewbie = FALSE;
	m_bAlwaysMoveable = FALSE;
	m_bNeverMoveable = FALSE;
	m_bMagic = FALSE;
	m_bOwnedByTown = FALSE;
	m_bInvisible = FALSE;
	m_bCursed = FALSE;
	m_bDamned = FALSE;
	m_Blessed = FALSE;
	m_Sacred = FALSE;
	m_ForSale = FALSE;
	m_Stolen = FALSE;
	m_CanDecay = FALSE;
	m_Statics = FALSE;
	//}}AFX_DATA_INIT
}

CItemTweakTab::~CItemTweakTab()
{
}

// Only t_ names are item types; other TYPEDEFs are event handlers.
static bool IsItemTypeName(const CString & csName)
{
	return csName.GetLength() > 2 && csName.Left(2).CompareNoCase("t_") == 0;
}

void CItemTweakTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CItemTweakTab)
	DDX_Control(pDX, IDC_ITPALETTE, m_cbPalette);
	DDX_Control(pDX, IDC_ITSPECTRUM, m_csSpectrum);
	DDX_Control(pDX, IDC_QUICKPICK, m_csQuickPalette);
	DDX_Control(pDX, IDC_ATTRVALUE, m_csAttrValue);
	DDX_Check(pDX, IDC_IDENTIFIED, m_bIdentified);
	DDX_Check(pDX, IDC_DECAY, m_bDecay);
	DDX_Check(pDX, IDC_NEWBIE, m_bNewbie);
	DDX_Check(pDX, IDC_ALWAYSMOVEABLE, m_bAlwaysMoveable);
	DDX_Check(pDX, IDC_NEVERMOVEABLE, m_bNeverMoveable);
	DDX_Check(pDX, IDC_MAGIC, m_bMagic);
	DDX_Check(pDX, IDC_OWNEDBYTOWN, m_bOwnedByTown);
	DDX_Check(pDX, IDC_INVISIBLE, m_bInvisible);
	DDX_Check(pDX, IDC_CURSED, m_bCursed);
	DDX_Check(pDX, IDC_DAMNED, m_bDamned);
	DDX_Check(pDX, IDC_BLESSED, m_Blessed);
	DDX_Check(pDX, IDC_SACRED, m_Sacred);
	DDX_Check(pDX, IDC_FORSALE, m_ForSale);
	DDX_Check(pDX, IDC_STOLEN, m_Stolen);
	DDX_Check(pDX, IDC_CANDECAY, m_CanDecay);
	DDX_Check(pDX, IDC_STATICFREEZE, m_Statics);

	DDX_Control(pDX, IDC_TYPES, m_ccbTypes);
	DDX_Control(pDX, IDC_ITEMEVENTS, m_ccbEvents);
	DDX_Control(pDX, IDC_ITEM_HUEINFO, m_csHueInfo);
	DDX_Control(pDX, IDC_ITEMMISC, m_ccbMisc);
	DDX_Control(pDX, IDC_ITEMMISCVALUE, m_ceMiscValue);
	DDX_Control(pDX, IDC_ITEMTAG, m_ccbTags);
	DDX_Control(pDX, IDC_ITEMTAGVALUE, m_ceTagsValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CItemTweakTab, CDockingPage)
	//{{AFX_MSG_MAP(CItemTweakTab)
	ON_BN_CLICKED(IDC_ITEMCOLOR, OnItemcolor)
	ON_BN_CLICKED(IDC_ITPALETTE, OnItpalette)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_RESET_ATTR, OnResetAttr)
	ON_BN_CLICKED(IDC_SETATTR, OnSetattr)

	ON_BN_CLICKED(IDC_IDENTIFIED, OnIdentified)
	ON_BN_CLICKED(IDC_DECAY, OnDecay)
	ON_BN_CLICKED(IDC_NEWBIE, OnNewbie)
	ON_BN_CLICKED(IDC_ALWAYSMOVEABLE, OnAlwaysmoveable)
	ON_BN_CLICKED(IDC_NEVERMOVEABLE, OnNevermoveable)
	ON_BN_CLICKED(IDC_MAGIC, OnMagic)
	ON_BN_CLICKED(IDC_OWNEDBYTOWN, OnOwnedbytown)
	ON_BN_CLICKED(IDC_INVISIBLE, OnInvisible)
	ON_BN_CLICKED(IDC_CURSED, OnCursed)
	ON_BN_CLICKED(IDC_DAMNED, OnDamned)
	ON_BN_CLICKED(IDC_BLESSED, OnBlessed)
	ON_BN_CLICKED(IDC_SACRED, OnSacred)
	ON_BN_CLICKED(IDC_FORSALE, OnForSale)
	ON_BN_CLICKED(IDC_STOLEN, OnStolen)
	ON_BN_CLICKED(IDC_CANDECAY, OnCanDecay)
	ON_BN_CLICKED(IDC_STATICFREEZE, OnStatics)

	ON_BN_CLICKED(IDC_SETTYPES, OnSettypes)
	ON_BN_CLICKED(IDC_SETEVENTS, OnSetevents)
	ON_BN_CLICKED(IDC_DELEVENTS, OnDelevents)
	ON_BN_CLICKED(IDC_ITEMSETMISC, OnSetmisc)
	ON_BN_CLICKED(IDC_ITEMSETTAG, OnSettag)
	ON_BN_CLICKED(IDC_OPENDOORWIZ, OnOpenDoorWizard)
	ON_BN_CLICKED(IDC_OPENLIGHTWIZ, OnOpenLightWizard)

	//ON_BN_CLICKED(IDC_RANDOMOLG, OnRandomOLG)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemTweakTab message handlers

BOOL CItemTweakTab::OnInitDialog() 
{
	m_dcPropertyPage = Main->m_pcppItemTweakTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CItemTweakTab;
	Main->m_pcppItemTweakTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();

	// Only real type names, no numeric TYPEDEFs.
	for ( int i = 0; i <= Main->m_pScripts->m_asaITEMTypes.GetUpperBound(); i++ )
		if ( IsItemTypeName(Main->m_pScripts->m_asaITEMTypes[i]) )
			m_ccbTypes.AddString(Main->m_pScripts->m_asaITEMTypes[i]);
	m_iTypeCount = (int) Main->m_pScripts->m_asaITEMTypes.GetSize();
	FillEvents();
	for ( int i = 0; i <= Main->m_pScripts->m_asaITEMProps.GetUpperBound(); i++ )
		m_ccbMisc.AddString(Main->m_pScripts->m_asaITEMProps[i]);
	for ( int i = 0; i <= Main->m_pScripts->m_asaITEMTags.GetUpperBound(); i++ )
		m_ccbTags.AddString(Main->m_pScripts->m_asaITEMTags[i]);

	m_ccbTypes.SetCurSel(0);
	m_ccbMisc.SetCurSel(0);
	m_ccbTags.SetCurSel(0);
	AjustComboBox(&m_ccbTypes);
	AjustComboBox(&m_ccbEvents);
	AjustComboBox(&m_ccbMisc);
	AjustComboBox(&m_ccbTags);

	for (int Index = 0; Index < 64; Index++)
	{
		CString csIndex;
		csIndex.Format("Index %d", Index);
		m_wQuickColorIndex[Index] = (WORD) Main->GetRegistryDword(csIndex, (Index*6)+2, hRegLocation, REGKEY_PALETTE);
	}
	m_wColorIndex = (WORD) ahextoi(Main->GetRegistryString("ColorPreviewColorIndex", "1"));
	UpdateQuikPick();
	DrawSpectrum(m_wColorIndex);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CItemTweakTab::OnItemcolor() 
{
	if ( m_wColorIndex > 0 && m_wColorIndex < 3000 )
	{
		CString csCommand;
		csCommand.Format("%sset color %05x", Main->m_csCommandPrefix, m_wColorIndex);
		SendToUO(csCommand);
	}
}

void CItemTweakTab::OnItpalette() 
{
	if (Main->m_dlgColorPal)
		delete Main->m_dlgColorPal;
	Main->m_dlgColorPal = new CColorSelectionDlg;
	Main->m_dlgColorPal->Create(IDD_COLOR_DLG);
	Main->m_dlgColorPal->colorIndex = -1;
}

// Quick palette grid; each cell shows the hue's gradient.
static const int QP_COLS = 16;
static const int QP_ROWS = 4;

int CItemTweakTab::QuickPickIndexAt(CPoint ptScreen)
{
	CRect rc;
	m_csQuickPalette.GetWindowRect(&rc);
	if ( !rc.PtInRect(ptScreen) )
		return -1;
	int cw = max(1, rc.Width() / QP_COLS);
	int ch = max(1, rc.Height() / QP_ROWS);
	int X = ( ptScreen.x - rc.left ) / cw;
	int Y = ( ptScreen.y - rc.top ) / ch;
	if ( X >= QP_COLS || Y >= QP_ROWS )
		return -1;
	return Y * QP_COLS + X;
}

void CItemTweakTab::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);
	int colorIndex = QuickPickIndexAt(ptScreen);
	if ( colorIndex >= 0 )
	{
		CString csColorIndex;
		csColorIndex.Format("%05x", m_wQuickColorIndex[colorIndex]);
		Main->PutRegistryString("ColorPreviewColorIndex", csColorIndex);
		m_wColorIndex = m_wQuickColorIndex[colorIndex];
		DrawSpectrum(m_wQuickColorIndex[colorIndex]);
		return;
	}
	CDockingPage::OnLButtonDown(nFlags, point);
}

void CItemTweakTab::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);
	int colorIndex = QuickPickIndexAt(ptScreen);
	if ( colorIndex >= 0 )
	{
		CString csColorIndex;
		csColorIndex.Format("%05x", m_wQuickColorIndex[colorIndex]);
		Main->PutRegistryString("ColorPreviewColorIndex", csColorIndex);

		if (Main->m_dlgColorPal)
			delete Main->m_dlgColorPal;
		Main->m_dlgColorPal = new CColorSelectionDlg;
		Main->m_dlgColorPal->Create(IDD_COLOR_DLG);
		Main->m_dlgColorPal->colorIndex = colorIndex;
		return;
	}
	CDockingPage::OnRButtonDown(nFlags, point);
}

BOOL CItemTweakTab::OnSetActive() 
{
	UpdateData();
	if ( Main->m_pScripts->m_asaITEMTypes.GetSize() != m_iTypeCount )
	{
		m_ccbTypes.ResetContent();
		for ( int i = 0; i <= Main->m_pScripts->m_asaITEMTypes.GetUpperBound(); i++ )
			if ( IsItemTypeName(Main->m_pScripts->m_asaITEMTypes[i]) )
				m_ccbTypes.AddString(Main->m_pScripts->m_asaITEMTypes[i]);
		m_ccbTypes.SetCurSel(0);
		AjustComboBox(&m_ccbTypes);
		FillEvents();
		m_iTypeCount = (int) Main->m_pScripts->m_asaITEMTypes.GetSize();
	}
	if ( m_ccbMisc.GetCount() != Main->m_pScripts->m_asaITEMProps.GetUpperBound()+1)
	{
		m_ccbMisc.ResetContent();
		for ( int i = 0; i <= Main->m_pScripts->m_asaITEMProps.GetUpperBound(); i++ )
			m_ccbMisc.AddString(Main->m_pScripts->m_asaITEMProps[i]);
		m_ccbMisc.SetCurSel(0);
		AjustComboBox(&m_ccbMisc);
	}
	if ( m_ccbTags.GetCount() != Main->m_pScripts->m_asaITEMTags.GetUpperBound()+1)
	{
		m_ccbTags.ResetContent();
		for ( int i = 0; i <= Main->m_pScripts->m_asaITEMTags.GetUpperBound(); i++ )
			m_ccbTags.AddString(Main->m_pScripts->m_asaITEMTags[i]);
		m_ccbTags.SetCurSel(0);
		AjustComboBox(&m_ccbTags);
	}

	return CDockingPage::OnSetActive();
}

void CItemTweakTab::UpdateQuikPick()
{
	HBITMAP hOldBmp = m_csQuickPalette.GetBitmap();
	if ( hOldBmp )
		DeleteObject(hOldBmp);
	CRect rc;
	m_csQuickPalette.GetClientRect(&rc);
	if ( rc.Width() <= 0 || rc.Height() <= 0 )
		return;

	CDC * pDC = m_csQuickPalette.GetDC();
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
	CBitmap * pOld = dcMem.SelectObject(&bmp);
	dcMem.FillSolidRect(0, 0, rc.Width(), rc.Height(), DarkFieldBkColor());

	int cw = rc.Width() / QP_COLS;
	int ch = rc.Height() / QP_ROWS;
	const int GAP = 3;
	for ( int Index = 0; Index < QP_COLS * QP_ROWS && Index < 64; Index++ )
	{
		CRect rcCell((Index % QP_COLS) * cw, (Index / QP_COLS) * ch, 0, 0);
		rcCell.right = rcCell.left + cw - GAP;
		rcCell.bottom = rcCell.top + ch - GAP;
		WORD wColor = m_wQuickColorIndex[Index];
		CHueGroup * pGroup = NULL;
		if ( !Main->m_aHueGroups.IsEmpty() && wColor > 0 && wColor < 3000 )
		{
			INT_PTR groupIndex = (wColor - 1) / 8;
			if ( groupIndex < Main->m_aHueGroups.GetSize() )
				pGroup = (CHueGroup *) Main->m_aHueGroups.GetAt(groupIndex);
		}
		if ( pGroup )
		{
			// Dark-to-light gradient across the cell width
			CHueEntry & Hue = pGroup->Hues[(wColor - 1) % 8];
			for ( int x = rcCell.left; x < rcCell.right; x++ )
			{
				int shade = (x - rcCell.left) * 32 / max(1, (int) rcCell.Width());
				if ( shade > 31 )
					shade = 31;
				dcMem.FillSolidRect(x, rcCell.top, 1, rcCell.Height(), ScaleColor(Hue.wColorTable[shade]));
			}
		}
		else
			dcMem.FillSolidRect(&rcCell, AxisClr(AXC_SEPARATOR));	// Empty cell
		if ( wColor == m_wColorIndex && wColor != 0 )
		{
			// Frame the selected color
			CBrush brGold(DarkAccentColor());
			dcMem.FrameRect(&rcCell, &brGold);
			CRect rcInner(rcCell);
			rcInner.DeflateRect(1, 1);
			dcMem.FrameRect(&rcInner, &brGold);
		}
	}
	dcMem.SelectObject(pOld);
	m_csQuickPalette.SetBitmap((HBITMAP) bmp.Detach());
	m_csQuickPalette.ReleaseDC(pDC);
}

DWORD CItemTweakTab::ScaleColor(WORD wColor)
{
	DWORD dwNewColor;

	dwNewColor = ((((((wColor >> 10) & 0x01f) * 0x0ff / 0x01f))
		| (((((wColor >> 5) & 0x01f) * 0x0ff / 0x01f)) << 8)
		| ((((wColor & 0x01f) * 0x0ff / 0x01f)) << 16)));
	return (dwNewColor);
}

void CItemTweakTab::DrawSpectrum(WORD wColor)
{
	HBITMAP hOldBmp = m_csSpectrum.GetBitmap();
	if ( hOldBmp )
		DeleteObject(hOldBmp);
	CRect rc;
	m_csSpectrum.GetClientRect(&rc);
	if ( rc.Width() <= 0 || rc.Height() <= 0 )
		return;

	CDC * pDC = m_csSpectrum.GetDC();
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
	CBitmap * pOld = dcMem.SelectObject(&bmp);
	dcMem.FillSolidRect(0, 0, rc.Width(), rc.Height(), DarkFieldBkColor());

	CString csInfo = AXT("Keine Farbe gew\xE4hlt");
	if ( !Main->m_aHueGroups.IsEmpty() && wColor > 0 && wColor < 3000 )
	{
		INT_PTR groupIndex = (wColor - 1) / 8;
		if ( groupIndex < Main->m_aHueGroups.GetSize() )
		{
			CHueGroup * pGroup = (CHueGroup *) Main->m_aHueGroups.GetAt(groupIndex);
			if ( pGroup )
			{
				// All 32 shades of the hue across the full width
				CHueEntry & Hue = pGroup->Hues[(wColor - 1) % 8];
				for ( int i = 0; i < 32; i++ )
				{
					int x1 = rc.Width() * i / 32;
					int x2 = rc.Width() * (i + 1) / 32;
					dcMem.FillSolidRect(x1, 0, x2 - x1, rc.Height(), ScaleColor(Hue.wColorTable[i]));
				}
				CString csName(Hue.cName, (int) strnlen(Hue.cName, 20));
				csName.Trim();
				csInfo.Format(AXT("Gew\xE4hlt: 0x%04X (%u)%s%s"), wColor, wColor, csName.IsEmpty() ? "" : "  -  ", (LPCTSTR) csName);
			}
		}
	}
	dcMem.SelectObject(pOld);
	m_csSpectrum.SetBitmap((HBITMAP) bmp.Detach());
	m_csSpectrum.ReleaseDC(pDC);
	if ( m_csHueInfo.GetSafeHwnd() )
		m_csHueInfo.SetWindowText(csInfo);
	// Update the selection frame in the quick palette
	if ( m_csQuickPalette.GetSafeHwnd() && m_wColorIndex == wColor )
		UpdateQuikPick();
}

void CItemTweakTab::OnResetAttr() 
{
	m_bIdentified = FALSE;
	m_bDecay = FALSE;
	m_bNewbie = FALSE;
	m_bAlwaysMoveable = FALSE;
	m_bNeverMoveable = FALSE;
	m_bMagic = FALSE;
	m_bOwnedByTown = FALSE;
	m_bInvisible = FALSE;
	m_bCursed = FALSE;
	m_bDamned = FALSE;
	m_Blessed = FALSE;
	m_Sacred = FALSE;
	m_ForSale = FALSE;
	m_Stolen = FALSE;
	m_CanDecay = FALSE;
	m_Statics = FALSE;
	m_csAttrValue.SetWindowText("00000");
	UpdateData(false);
}

void CItemTweakTab::OnSetattr() 
{
	CString csVal;
	m_csAttrValue.GetWindowText(csVal);
	CString csCmd;
	csCmd.Format("%sset attr %s", Main->m_csCommandPrefix, csVal);
	SendToUO(csCmd);
}

void CItemTweakTab::OnIdentified() 
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_IDENTIFIED;
	else
		dwVal ^= ATTR_IDENTIFIED;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnDecay()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_DECAY;
	else
		dwVal ^= ATTR_DECAY;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnNewbie()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_NEWBIE;
	else
		dwVal ^= ATTR_NEWBIE;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnAlwaysmoveable()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_MOVE_ALWAYS;
	else
		dwVal ^= ATTR_MOVE_ALWAYS;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnNevermoveable()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_MOVE_NEVER;
	else
		dwVal ^= ATTR_MOVE_NEVER;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnMagic()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_MAGIC;
	else
		dwVal ^= ATTR_MAGIC;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnOwnedbytown()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_OWNED;
	else
		dwVal ^= ATTR_OWNED;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnInvisible()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_INVIS;
	else
		dwVal ^= ATTR_INVIS;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnCursed()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_CURSED;
	else
		dwVal ^= ATTR_CURSED;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnDamned()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_DAMNED;
	else
		dwVal ^= ATTR_DAMNED;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnBlessed()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_BLESSED;
	else
		dwVal ^= ATTR_BLESSED;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnSacred()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_SACRED;
	else
		dwVal ^= ATTR_SACRED;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnForSale()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_FORSALE;
	else
		dwVal ^= ATTR_FORSALE;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnStolen()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_STOLEN;
	else
		dwVal ^= ATTR_STOLEN;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnCanDecay()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_CAN_DECAY;
	else
		dwVal ^= ATTR_CAN_DECAY;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnStatics()
{
	UpdateData(true);
	CString csCurrentVal;
	m_csAttrValue.GetWindowText(csCurrentVal);
	DWORD dwVal = ahextoi(csCurrentVal);
	if (m_bIdentified)
		dwVal |= ATTR_STATIC;
	else
		dwVal ^= ATTR_STATIC;
	CString csNewVal;
	csNewVal.Format("%05x", dwVal);
	m_csAttrValue.SetWindowText(csNewVal);
	UpdateData(false);	
}

void CItemTweakTab::OnSettypes() 
{
	CString csType;
	m_ccbTypes.GetWindowText(csType);
	csType.Trim();
	if (csType == "")
	{
		AxisSetStatus(AXT("Erst einen Typ in der Liste w\xE4hlen."), 2);
		return;
	}
	CString csCmd;
	csCmd.Format("%sset type %s", Main->m_csCommandPrefix, csType);
	SendToUO(csCmd);
}

void CItemTweakTab::OnSetevents() 
{
	// "set" is required, otherwise the event is added to the GM.
	CString csEvent;
	m_ccbEvents.GetWindowText(csEvent);
	csEvent.Trim();
	if (csEvent == "")
	{
		AxisSetStatus(AXT("Erst ein Event w\xE4hlen oder eintippen."), 2);
		return;
	}
	CString csCmd;
	csCmd.Format("%sset events +%s", Main->m_csCommandPrefix, csEvent);
	SendToUO(csCmd);
}

void CItemTweakTab::OnDelevents()
{
	CString csEvent;
	m_ccbEvents.GetWindowText(csEvent);
	csEvent.Trim();
	if (csEvent == "")
	{
		AxisSetStatus(AXT("Erst ein Event w\xE4hlen oder eintippen."), 2);
		return;
	}
	CString csCmd;
	csCmd.Format("%sset events -%s", Main->m_csCommandPrefix, csEvent);
	SendToUO(csCmd);
}

void CItemTweakTab::OnSetmisc() 
{
	CString csMisc, csValue;
	m_ccbMisc.GetWindowText(csMisc);
	m_ceMiscValue.GetWindowText(csValue);
	csMisc = csMisc.SpanExcluding("(");
	if (csMisc != "" && csValue != "")
	{
		CString csCmd;
		csCmd.Format("%sset %s %s", Main->m_csCommandPrefix, csMisc, csValue);
		SendToUO(csCmd);
	}
}

void CItemTweakTab::OnSettag() 
{
	CString csTag, csValue;
	m_ccbTags.GetWindowText(csTag);
	m_ceTagsValue.GetWindowText(csValue);
	csTag = csTag.SpanExcluding("(");
	if (csTag != "" && csValue != "")
	{
		CString csCmd;
		csCmd.Format("%sset %s %s", Main->m_csCommandPrefix, csTag, csValue);
		SendToUO(csCmd);
	}
}

void CItemTweakTab::OnOpenDoorWizard()
{
	if (Main->m_dlgDoorWiz)
			delete Main->m_dlgDoorWiz;
	Main->m_dlgDoorWiz = new CDoorWizard;
	Main->m_dlgDoorWiz->Create(IDD_DOORWIZARD);
}

void CItemTweakTab::OnOpenLightWizard()
{
	if (Main->m_dlgLightWiz)
		delete Main->m_dlgLightWiz;
	Main->m_dlgLightWiz = new CLightWizard;
	Main->m_dlgLightWiz->Create(IDD_LIGHTWIZARD);
}

/*int CNewUIntArray::Insert(int inum, int pos)
{
	INT_PTR iLower = 0;
	INT_PTR iUpper = this->GetUpperBound();
	if ( iUpper == -1 )
	{
		this->InsertAt(0, inum);
		return 0;
	}
	INT_PTR iIndex = 0;
	INT_PTR iCompare = 0;

	if (pos > -1)
	{
		iIndex = Main->m_pcppItemTweakTab->Order.Insert(pos);
	}
	else
	{
		while ( iLower <= iUpper )
		{
			iIndex = (iUpper + iLower ) / 2;
			int itest = this->GetAt(iIndex);
			if ( itest > inum )
			{
				iCompare = 0;
				iUpper = iIndex - 1;
			}
			else
			{
				iCompare = 1;
				iLower = iIndex + 1;
			}
		}
		iIndex += iCompare;
	}
	this->InsertAt(iIndex, inum);
	return (int)iIndex;
}

void CItemTweakTab::OnRandomOLG()
{  
	int c = 10000;
	Order.RemoveAll();
	CNewUIntArray numb,result;
for (int i=0; i<c; i++)
{
	CRandomMT MTrand(rand());
	numb.Insert(MTrand.RandomRange(1,50));
}

int j=0;
for (int i=0; i<c-1; i++)
{
    if (numb.GetAt(i) == numb.GetAt(i+1))
	{
		j+=1;
	}
    else
    {
		result.Insert(numb.GetAt(i),j);
        j=0;
    }
}

int test = result.GetAt(result.GetUpperBound()-6);
int test2 = result.GetAt(result.GetUpperBound()-1);
int test3 = result.GetAt(result.GetUpperBound()-2);
int test4 = result.GetAt(result.GetUpperBound()-3);
int test5 = result.GetAt(result.GetUpperBound()-4);
int test6 = result.GetAt(result.GetUpperBound()-5);

Main->m_log.Add(1, AXT("%d %d %d %d %d %d"), test,test2,test3,test4,test5,test6);
}*/

// Fills the events combo with all [EVENTS] plus the type names (TYPEDEFs can be attached as events too).
void CItemTweakTab::FillEvents()
{
	CString csCurrent;
	m_ccbEvents.GetWindowText(csCurrent);
	m_ccbEvents.ResetContent();
	for ( int i = 0; i <= Main->m_pScripts->m_asaEvents.GetUpperBound(); i++ )
		if ( !IsNumber(Main->m_pScripts->m_asaEvents[i]) )
			m_ccbEvents.AddString(Main->m_pScripts->m_asaEvents[i]);
	for ( int i = 0; i <= Main->m_pScripts->m_asaITEMTypes.GetUpperBound(); i++ )
		if ( !IsNumber(Main->m_pScripts->m_asaITEMTypes[i]) && m_ccbEvents.FindStringExact(-1, Main->m_pScripts->m_asaITEMTypes[i]) == CB_ERR )
			m_ccbEvents.AddString(Main->m_pScripts->m_asaITEMTypes[i]);
	m_ccbEvents.SetWindowText(csCurrent);
}
