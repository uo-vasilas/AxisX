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

// MiscTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "MiscTab.h"
#include "vfw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMiscTab property page

IMPLEMENT_DYNCREATE(CMiscTab, CDockingPage)

int CMiscTab::iNameSort = 1;

CMiscTab::CMiscTab() : CDockingPage(CMiscTab::IDD,CMsg("IDS_MISC"))
{
	//{{AFX_DATA_INIT(CMiscTab)
	//}}AFX_DATA_INIT
}

CMiscTab::~CMiscTab()
{
}

void CMiscTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMiscTab)
	DDX_Control(pDX, IDC_SPELL_LIST, m_clcSpellList);
	DDX_Control(pDX, IDC_SPELL_ID, m_csSpellID);
	DDX_Control(pDX, IDC_SOUND_LIST, m_clcSoundList);
	DDX_Control(pDX, IDC_SFX_ID, m_csSoundID);
	DDX_Control(pDX, IDC_MUSIC_LIST, m_clcMusicList);
	DDX_Control(pDX, IDC_MUSIC_ID, m_csMusicID);
	DDX_Control(pDX, IDC_SPELLFILTER, m_ceSpellFilter);
	DDX_Control(pDX, IDC_MUSICFILTER, m_ceMusicFilter);
	DDX_Control(pDX, IDC_SOUNDFILTER, m_ceSoundFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMiscTab, CDockingPage)
	//{{AFX_MSG_MAP(CMiscTab)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SPELL_LIST, OnItemchangedSpell)
	ON_BN_CLICKED(IDC_CAST_SPELL, OnCastSpell)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SOUND_LIST, OnItemchangedSound)
	ON_BN_CLICKED(IDC_PLAY_SFX, OnPlaySound)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MUSIC_LIST, OnItemchangedMusic)
	ON_BN_CLICKED(IDC_PLAYMIDI, OnPlayMusic)
	ON_BN_CLICKED(IDC_SEND_SFX, OnSendSound)
	ON_BN_CLICKED(IDC_SENDMIDI, OnSendMusic)
	ON_BN_CLICKED(IDC_STOP_SFX, OnStopSound)
	ON_BN_CLICKED(IDC_STOPMIDI, OnStopMusic)
	ON_EN_CHANGE(IDC_SPELLFILTER, OnChangeSpellFilter)
	ON_EN_CHANGE(IDC_MUSICFILTER, OnChangeMusicFilter)
	ON_EN_CHANGE(IDC_SOUNDFILTER, OnChangeSoundFilter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiscTab message handlers

BOOL CMiscTab::OnInitDialog() 
{
	m_dcPropertyPage = Main->m_pcppMiscTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CMiscTab;
	Main->m_pcppMiscTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();

	m_clcSpellList.InsertColumn(0, AXT("Zauber"),LVCFMT_LEFT,130,-1);
	m_clcSpellList.InsertColumn(1, AXT("Defname"),LVCFMT_LEFT,140,-1);
	m_clcSoundList.InsertColumn(0, AXT("Name"),LVCFMT_LEFT,180,-1);
	m_clcSoundList.InsertColumn(1, AXT("Nr."),LVCFMT_CENTER,50,-1);
	m_clcMusicList.InsertColumn(0, AXT("Name"),LVCFMT_LEFT,180,-1);
	m_clcMusicList.InsertColumn(1, AXT("Nr."),LVCFMT_CENTER,50,-1);
	CListCtrl * aLists[] = { &m_clcSpellList, &m_clcSoundList, &m_clcMusicList };
	for (int i = 0; i < 3; i++)
	{
		aLists[i]->SetExtendedStyle(aLists[i]->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
		ApplyAxisListHeader(aLists[i]->GetSafeHwnd());
	}
	AxisSetCue(m_ceSpellFilter.GetSafeHwnd(), "Zauber suchen ...");
	AxisSetCue(m_ceMusicFilter.GetSafeHwnd(), "Musik suchen ...");
	AxisSetCue(m_ceSoundFilter.GetSafeHwnd(), "Klang suchen (Name oder Nummer) ...");
	FillSpells();
	FillSounds();
	FillMusic();
	AxisMarkPrimary(this, IDC_CAST_SPELL);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMiscTab::OnItemchangedSpell(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (pNMListView->uNewState & LVNI_SELECTED)
	{
		int iSelIndex = m_clcSpellList.GetNextItem(-1, LVNI_SELECTED);
		if (iSelIndex == -1)
		{
			m_csSpellID.SetWindowText("0");
			return;
		}
		m_clcSpellList.SetHotItem(iSelIndex);
		CSObject * pObject = (CSObject *) m_clcSpellList.GetItemData(iSelIndex);
		if ( !pObject )
		{
			m_csSpellID.SetWindowText("0");
			return;
		}

		m_csSpellID.SetWindowText(pObject->m_csValue);
		return;
	}

	*pResult = 0;
}

void CMiscTab::OnCastSpell() 
{
	CString csVal, csCmd;
	m_csSpellID.GetWindowText(csVal);
	csCmd.Format("%scast %s", Main->m_csCommandPrefix, csVal);
	SendToUO(csCmd);
}

void CMiscTab::OnItemchangedSound(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if(m_Sound !=NULL)
	{
		MCIWndDestroy(m_Sound);
		m_Sound = NULL;
	}
	
	if (pNMListView->uNewState & LVNI_SELECTED)
	{
		int iSelIndex = m_clcSoundList.GetNextItem(-1, LVNI_SELECTED);
		if (iSelIndex == -1)
		{
			m_csSoundID.SetWindowText("0");
			return;
		}
		m_clcSoundList.SetHotItem(iSelIndex);
		CSoundIDX * pSound = (CSoundIDX *) m_clcSoundList.GetItemData(iSelIndex);
		if ( !pSound )
		{
			m_csSoundID.SetWindowText("0");
			return;
		}
		
		CString csIndex;
		csIndex.Format("%d",pSound->wIndex - 1);
		m_csSoundID.SetWindowText(csIndex);
		return;
	}

	*pResult = 0;
}

void CMiscTab::OnPlaySound() 
{
	if(m_Music !=NULL)
	{
		MCIWndDestroy(m_Music);
		m_Music = NULL;
	}

	int iSelIndex = m_clcSoundList.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1)
		return;

	if(m_Sound == NULL)
	{
		CSoundIDX * pSound = (CSoundIDX *) m_clcSoundList.GetItemData(iSelIndex);
		if ( !pSound )
			return;

		CFile cfSoundmul;
		CString csSoundmulFile;
		csSoundmulFile = Main->GetMulPath(VERFILE_SOUND);

		if ( cfSoundmul.Open( csSoundmulFile, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone ) )
		{
			byte bHeadertop[] = { 0x52, 0x49, 0x46, 0x46 };
			unsigned long ulHeadersize = (pSound->dwLenght+4&0x000000FF) + (pSound->dwLenght+4&0x0000FF00) 
										+ (pSound->dwLenght+4&0x00FF0000) + (pSound->dwLenght+4&0xFF000000);
			byte bHeadermid[] = { 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 
									0x01, 0x00, 0x01, 0x00, 0x22, 0x56, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 
									0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61 };
			unsigned long ulDatasize = (pSound->dwLenght-32&0x000000FF) + (pSound->dwLenght-32&0x0000FF00)
										+ (pSound->dwLenght-32&0x00FF0000) + (pSound->dwLenght-32&0xFF000000);

			byte bData[1000000];
			cfSoundmul.Seek(pSound->dwStart + 32,CFile::begin);
			cfSoundmul.Read( &bData, pSound->dwLenght - 32 );
			CFile cfWave;
			if ( cfWave.Open( "temp.wav" ,CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyNone ) )
			{
				cfWave.Write(&bHeadertop,sizeof(bHeadertop));
				cfWave.Write(&ulHeadersize,4);
				cfWave.Write(&bHeadermid,sizeof(bHeadermid));
				cfWave.Write(&ulDatasize,4);
				cfWave.Write(&bData,pSound->dwLenght - 32);
				cfWave.Close();

				m_Sound = MCIWndCreateA(this->GetSafeHwnd(),AfxGetInstanceHandle(),WS_CHILD|MCIWNDF_NOMENU,"temp.wav");
			}
		}
		cfSoundmul.Close();
	}
	else
	{
		MCIWndHome(m_Sound);
	}
	MCIWndPlay(m_Sound);
}

void CMiscTab::OnSendSound() 
{
	CString csVal, csCmd;
	m_csSoundID.GetWindowText(csVal);
	csCmd.Format("%ssound %s", Main->m_csCommandPrefix, csVal);
	SendToUO(csCmd);
}

void CMiscTab::OnStopSound() 
{
	if(m_Sound !=NULL)
	{
		MCIWndDestroy(m_Sound);
		m_Sound = NULL;
	}
}

void CMiscTab::OnItemchangedMusic(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if(m_Music !=NULL)
	{
		MCIWndDestroy(m_Music);
		m_Music = NULL;
	}
	
	if (pNMListView->uNewState & LVNI_SELECTED)
	{
		int iSelIndex = m_clcMusicList.GetNextItem(-1, LVNI_SELECTED);
		if (iSelIndex == -1)
		{
			m_csMusicID.SetWindowText("0");
			return;
		}
		m_clcMusicList.SetHotItem(iSelIndex);
		CMusicList * pMusic = (CMusicList *) m_clcMusicList.GetItemData(iSelIndex);
		if ( !pMusic )
		{
			m_csMusicID.SetWindowText("0");
			return;
		}
		
		CString csIndex;
		csIndex.Format("%d",pMusic->wIndex);
		m_csMusicID.SetWindowText(csIndex);
		return;
	}

	*pResult = 0;
}

void CMiscTab::OnPlayMusic() 
{
	if(m_Sound !=NULL)
	{
		MCIWndDestroy(m_Sound);
		m_Sound = NULL;
	}

	int iSelIndex = m_clcMusicList.GetNextItem(-1, LVNI_SELECTED);
	if (iSelIndex == -1)
		return;

	if(m_Music == NULL)
	{
		CMusicList * pMusic = (CMusicList *) m_clcMusicList.GetItemData(iSelIndex);
		if ( !pMusic )
			return;

		m_Music = MCIWndCreateA(this->GetSafeHwnd(), AfxGetInstanceHandle(), WS_CHILD|MCIWNDF_NOMENU,pMusic->csPath);

	}
	else
	{
		MCIWndHome(m_Music);
	}
	MCIWndPlay(m_Music);
}

void CMiscTab::OnSendMusic() 
{
	CString csVal, csCmd;
	m_csMusicID.GetWindowText(csVal);
	csCmd.Format("%smidilist %s", Main->m_csCommandPrefix, csVal);
	SendToUO(csCmd);
}

void CMiscTab::OnStopMusic() 
{
	if(m_Music !=NULL)
	{
		MCIWndDestroy(m_Music);
		m_Music = NULL;
	}
}

int CALLBACK CMiscTab::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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
	}
	return 0;
}

// ---- List filters ----

static bool MiscFilterMatch(const CString & csFilter, const CString & csA, const CString & csB)
{
	if (csFilter.IsEmpty())
		return true;
	CString csHay = csA + " " + csB;
	csHay.MakeLower();
	return csHay.Find(csFilter) != -1;
}

static CString MiscFilterText(CEdit & ceFilter)
{
	CString cs;
	ceFilter.GetWindowText(cs);
	cs.Trim();
	cs.MakeLower();
	return cs;
}

void CMiscTab::FillSpells()
{
	CString csFilter = MiscFilterText(m_ceSpellFilter);
	m_clcSpellList.SetRedraw(FALSE);
	m_clcSpellList.DeleteAllItems();
	int iRow = 0;
	for ( int i = 0; i < Main->m_pScripts->m_aSpellList.GetSize(); i++ )
	{
		CSObject * pSpell = (CSObject *) Main->m_pScripts->m_aSpellList.GetAt(i);
		if ( !pSpell || !MiscFilterMatch(csFilter, pSpell->m_csDescription, pSpell->m_csID) )
			continue;
		m_clcSpellList.InsertItem(iRow, pSpell->m_csDescription);
		m_clcSpellList.SetItemText(iRow, 1, pSpell->m_csID);
		m_clcSpellList.SetItemData(iRow, (DWORD_PTR)pSpell);
		iRow++;
	}
	m_clcSpellList.SortItems(CompareFunc, 0);
	m_clcSpellList.SetRedraw(TRUE);
}

void CMiscTab::FillSounds()
{
	CString csFilter = MiscFilterText(m_ceSoundFilter);
	m_clcSoundList.SetRedraw(FALSE);
	m_clcSoundList.DeleteAllItems();
	CString csIndex;
	int iRow = 0;
	for ( int i = 0; i < Main->m_aSounds.GetSize(); i++ )
	{
		CSoundIDX * pSound = (CSoundIDX *) Main->m_aSounds.GetAt(i);
		if ( !pSound )
			continue;
		csIndex.Format("%d",pSound->wIndex - 1);
		if ( !MiscFilterMatch(csFilter, pSound->csName, csIndex) )
			continue;
		m_clcSoundList.InsertItem(iRow, pSound->csName);
		m_clcSoundList.SetItemText(iRow, 1, csIndex);
		m_clcSoundList.SetItemData(iRow, (DWORD_PTR)pSound);
		iRow++;
	}
	m_clcSoundList.SetRedraw(TRUE);
}

void CMiscTab::FillMusic()
{
	CString csFilter = MiscFilterText(m_ceMusicFilter);
	m_clcMusicList.SetRedraw(FALSE);
	m_clcMusicList.DeleteAllItems();
	CString csIndex;
	int iRow = 0;
	for ( int i = 0; i < Main->m_aMusic.GetSize(); i++ )
	{
		CMusicList * pMusic = (CMusicList *) Main->m_aMusic.GetAt(i);
		if ( !pMusic )
			continue;
		csIndex.Format("%d",pMusic->wIndex);
		if ( !MiscFilterMatch(csFilter, pMusic->csName, csIndex) )
			continue;
		m_clcMusicList.InsertItem(iRow, pMusic->csName);
		m_clcMusicList.SetItemText(iRow, 1, csIndex);
		m_clcMusicList.SetItemData(iRow, (DWORD_PTR)pMusic);
		iRow++;
	}
	m_clcMusicList.SetRedraw(TRUE);
}

void CMiscTab::OnChangeSpellFilter() { FillSpells(); }
void CMiscTab::OnChangeMusicFilter() { FillMusic(); }
void CMiscTab::OnChangeSoundFilter() { FillSounds(); }