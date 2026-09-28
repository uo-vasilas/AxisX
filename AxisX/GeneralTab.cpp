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

// GeneralTab.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "GeneralTab.h"
#include "RemoteConsoleDlg.h"
#include "RemoteConsole.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemTab property page

IMPLEMENT_DYNCREATE(CGeneralTab, CDockingPage)

CGeneralTab::CGeneralTab() : CDockingPage(CGeneralTab::IDD,CMsg("IDS_GENERAL"))
{
	//{{AFX_DATA_INIT(CGeneralTab)
	this->m_bRemoteConsoleCreated = false;
	this->m_bRemoteConsoleVisible = false;
	//}}AFX_DATA_INIT
}

CGeneralTab::~CGeneralTab()
{
}

void CGeneralTab::DoDataExchange(CDataExchange* pDX)
{
	CDockingPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralTab)
	DDX_Control(pDX, IDC_SLIDERVALUE, m_csSliderValue);
	DDX_Control(pDX, IDC_SLIDER1, m_cscSlider);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_GP_INFO, cg_information);
	DDX_Control(pDX, IDC_ADMIN, cb_admin);
	DDX_Control(pDX, IDC_ITEMINFO, cb_info);
	DDX_Control(pDX, IDC_SETEDIT, cb_edit);
	DDX_Control(pDX, IDC_CLIENTS, cb_clients);
	DDX_Control(pDX, IDC_SERVERINFO, cb_serv_info);
	DDX_Control(pDX, IDC_VERSION, cb_version);
	//***
	DDX_Control(pDX, IDC_GP_MISC, cg_misc);
	DDX_Control(pDX, IDC_LINK, cb_link);
	DDX_Control(pDX, IDC_FLIP, cb_flip);
	DDX_Control(pDX, IDC_SHRINK, cb_shrink);
	DDX_Control(pDX, IDC_DUPE, cb_dupe);
	DDX_Control(pDX, IDC_REMOVE, cb_remove);
	DDX_Control(pDX, IDC_NUKE, cb_nuke);
	//***
	DDX_Control(pDX, IDC_GP_VENDORS, cg_vendors);
	DDX_Control(pDX, IDC_BUY, cb_buy);
	DDX_Control(pDX, IDC_SELL, cb_sell);
	DDX_Control(pDX, IDC_STOCK, cb_inventory);
	DDX_Control(pDX, IDC_PURCHASES, cb_purchases);
	DDX_Control(pDX, IDC_SAMPLES, cb_samples);
	DDX_Control(pDX, IDC_XRESTOCK, cb_restock);
	//***
	DDX_Control(pDX, IDC_GP_WEATHER, cg_weather);
	DDX_Control(pDX, IDC_SNOW, cb_snow);
	DDX_Control(pDX, IDC_RAIN, cb_rain);
	DDX_Control(pDX, IDC_DRY, cb_dry);
	DDX_Control(pDX, IDC_SETLIGHT, cb_light_lvl);
	//***
	DDX_Control(pDX, IDC_SELF, cg_self);
	DDX_Control(pDX, IDC_INVULNERABLE, cb_invul);
	DDX_Control(pDX, IDC_SETALLMOVE, cb_allmove);
	DDX_Control(pDX, IDC_INVISIBLE, cb_invis);
	DDX_Control(pDX, IDC_FIX, cb_fix);
	DDX_Control(pDX, IDC_TELE, cb_tele);
	DDX_Control(pDX, IDC_SETHEARALL, cb_hear_all);
	DDX_Control(pDX, IDC_GMTOGGLE, cb_gm_toggle);
	DDX_Control(pDX, IDC_SETDETAIL, cb_details);
	DDX_Control(pDX, IDC_NIGHTSIGHT, cb_nightsight);
	DDX_Control(pDX, IDC_SETDEBUG, cb_debug);
	//***
	DDX_Control(pDX, IDC_GP_DISCIPLINE, cg_discipline);
	DDX_Control(pDX, IDC_JAIL, cb_jail);
	DDX_Control(pDX, IDC_FORGIVE, cb_forgive);
	DDX_Control(pDX, IDC_KILL, cb_kill);
	DDX_Control(pDX, IDC_DISCONNECT, cb_disconnect);
	DDX_Control(pDX, IDC_RESURECT, cb_resurrect);
	//***
	DDX_Control(pDX, IDC_GP_GMPAGE, cg_gm_page);
	DDX_Control(pDX, IDC_PAGEON, cb_page_on);
	DDX_Control(pDX, IDC_PAGELIST, cb_page_list);
	DDX_Control(pDX, IDC_PAGEPLAYER, cb_page_goplayer);
	DDX_Control(pDX, IDC_PAGEDISCONNECT, cb_page_disconnect);
	DDX_Control(pDX, IDC_PAGEKICK, cb_page_ban);
	DDX_Control(pDX, IDC_PAGEOFF, cb_page_off);
	DDX_Control(pDX, IDC_PAGEQUEUE, cb_page_queue);
	DDX_Control(pDX, IDC_PAGEORIGIN, cb_page_goorigine);
	DDX_Control(pDX, IDC_PAGEJAIL, cb_page_jail);
	DDX_Control(pDX, IDC_PAGEDELETE, cb_page_delete);
	//***
	DDX_Control(pDX, IDC_WORLDSAVE, cb_save);
	DDX_Control(pDX, IDC_SAVESTATICS, cb_save_statics);
	DDX_Control(pDX, IDC_RESYNC, cb_resync);
	DDX_Control(pDX, IDC_RESTOCKALL, cb_restock_all);
	DDX_Control(pDX, IDC_REMCONSOLE, m_cbRemoteConsole);
}


BEGIN_MESSAGE_MAP(CGeneralTab, CDockingPage)
	//{{AFX_MSG_MAP(CGeneralTab)
	ON_BN_CLICKED(IDC_REMCONSOLE, OnRconsole)
	ON_BN_CLICKED(IDC_ADMIN, OnAdmin)
	ON_BN_CLICKED(IDC_ITEMINFO, OnIteminfo)
	ON_BN_CLICKED(IDC_SETEDIT, OnSetedit)
	ON_BN_CLICKED(IDC_CLIENTS, OnClients)
	ON_BN_CLICKED(IDC_SERVERINFO, OnServerinfo)
	ON_BN_CLICKED(IDC_VERSION, OnVersion)
	ON_BN_CLICKED(IDC_LINK, OnLink)
	ON_BN_CLICKED(IDC_FLIP, OnFlip)
	ON_BN_CLICKED(IDC_SHRINK, OnShrink)
	ON_BN_CLICKED(IDC_DUPE, OnDupe)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_NUKE, OnNuke)
	ON_BN_CLICKED(IDC_BUY, OnBuy)
	ON_BN_CLICKED(IDC_SELL, OnSell)
	ON_BN_CLICKED(IDC_STOCK, OnStock)
	ON_BN_CLICKED(IDC_PURCHASES, OnPurchases)
	ON_BN_CLICKED(IDC_SAMPLES, OnSamples)
	ON_BN_CLICKED(IDC_XRESTOCK, OnXrestock)
	ON_BN_CLICKED(IDC_SNOW, OnSnow)
	ON_BN_CLICKED(IDC_RAIN, OnRain)
	ON_BN_CLICKED(IDC_DRY, OnDry)
	ON_BN_CLICKED(IDC_SETLIGHT, OnSetlight)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_INVULNERABLE, OnInvulnerable)
	ON_BN_CLICKED(IDC_SETALLMOVE, OnSetallmove)
	ON_BN_CLICKED(IDC_INVISIBLE, OnInvisible)
	ON_BN_CLICKED(IDC_FIX, OnFix)
	ON_BN_CLICKED(IDC_TELE, OnTele)
	ON_BN_CLICKED(IDC_SETHEARALL, OnSethearall)
	ON_BN_CLICKED(IDC_GMTOGGLE, OnGmtoggle)
	ON_BN_CLICKED(IDC_SETDETAIL, OnSetdetail)
	ON_BN_CLICKED(IDC_NIGHTSIGHT, OnNightsight)
	ON_BN_CLICKED(IDC_SETDEBUG, OnSetdebug)
	ON_BN_CLICKED(IDC_JAIL, OnJail)
	ON_BN_CLICKED(IDC_FORGIVE, OnForgive)
	ON_BN_CLICKED(IDC_KILL, OnKill)
	ON_BN_CLICKED(IDC_DISCONNECT, OnDisconnect)
	ON_BN_CLICKED(IDC_RESURECT, OnResurect)
	ON_BN_CLICKED(IDC_PAGEON, OnPageon)
	ON_BN_CLICKED(IDC_PAGELIST, OnPagelist)
	ON_BN_CLICKED(IDC_PAGEPLAYER, OnPageplayer)
	ON_BN_CLICKED(IDC_PAGEDISCONNECT, OnPagedisconnect)
	ON_BN_CLICKED(IDC_PAGEKICK, OnPagekick)
	ON_BN_CLICKED(IDC_PAGEOFF, OnPageoff)
	ON_BN_CLICKED(IDC_PAGEQUEUE, OnPagequeue)
	ON_BN_CLICKED(IDC_PAGEORIGIN, OnPageorigin)
	ON_BN_CLICKED(IDC_PAGEJAIL, OnPagejail)
	ON_BN_CLICKED(IDC_PAGEDELETE, OnPagedelete)
	ON_BN_CLICKED(IDC_WORLDSAVE, OnWorldsave)
	ON_BN_CLICKED(IDC_SAVESTATICS, OnSavestatics)
	ON_BN_CLICKED(IDC_RESYNC, OnResync)
	ON_BN_CLICKED(IDC_RESTOCKALL, OnRestockall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneralTab message handlers

BOOL CGeneralTab::OnInitDialog() 
{
	m_dcPropertyPage = Main->m_pcppGeneralTab;
	if(m_bModifyDlgStylesAndPos == false)
		m_dcDialogPage = new CGeneralTab;
	Main->m_pcppGeneralTab->m_dcCurrentPage = this;

	CDockingPage::OnInitDialog();
	OnSetLanguage();
	m_cscSlider.SetRange(0, 30);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGeneralTab::OnSetLanguage()
{
	// Only the console button changes its caption at runtime.
	m_cbRemoteConsole.SetWindowText(CMsg("IDS_REMOTE_CONSOLE"));

	// Tooltips showing the Sphere command behind each button
	if (m_tip.GetSafeHwnd() == NULL)
	{
		m_tip.Create(this);
		m_tip.SetMaxTipWidth(360);
		static const struct { UINT uID; LPCTSTR pszTip; } aTips[] = {
			{ IDC_INVULNERABLE, "invulnerable" }, { IDC_INVISIBLE, "invisible" }, { IDC_SETALLMOVE, "allmove" },
			{ IDC_SETHEARALL, "hearall" }, { IDC_GMTOGGLE, "gm" }, { IDC_NIGHTSIGHT, "nightsight" },
			{ IDC_SETDETAIL, "detail" }, { IDC_SETDEBUG, "debug" }, { IDC_FIX, "fix - eigene H\xF6he korrigieren" },
			{ IDC_TELE, "tele - Zielort anklicken" }, { IDC_ITEMINFO, "info" }, { IDC_SETEDIT, "xedit" },
			{ IDC_LINK, "link" }, { IDC_FLIP, "xflip" }, { IDC_SHRINK, "shrink" }, { IDC_DUPE, "dupe" },
			{ IDC_ADMIN, "admin" }, { IDC_CLIENTS, "show serv.clients" }, { IDC_SERVERINFO, "information" },
			{ IDC_VERSION, "version" }, { IDC_JAIL, "jail" }, { IDC_FORGIVE, "forgive" },
			{ IDC_DISCONNECT, "xdisconnect" }, { IDC_RESURECT, "xresurrect" }, { IDC_PAGEON, "page on" },
			{ IDC_PAGEOFF, "page off" }, { IDC_PAGELIST, "page list" }, { IDC_PAGEQUEUE, "page queue" },
			{ IDC_PAGEPLAYER, "page player" }, { IDC_PAGEORIGIN, "page origin" }, { IDC_PAGEDISCONNECT, "page disconnect" },
			{ IDC_PAGEJAIL, "page jail" }, { IDC_PAGEKICK, "page ban" }, { IDC_PAGEDELETE, "page delete" },
			{ IDC_BUY, "sagt \"buy\" - H\xE4ndler anklicken" }, { IDC_SELL, "sagt \"sell\"" }, { IDC_STOCK, "bank 1a" },
			{ IDC_PURCHASES, "bank 1b" }, { IDC_SAMPLES, "bank 1c" }, { IDC_XRESTOCK, "xrestock" },
			{ IDC_SNOW, "sector.snow" }, { IDC_RAIN, "sector.rain" }, { IDC_DRY, "sector.dry" },
			{ IDC_SETLIGHT, "sector.light <wert> - 0 hell ... 30 dunkel" }, { IDC_WORLDSAVE, "serv.save" },
			{ IDC_SAVESTATICS, "serv.savestatics" }, { IDC_RESYNC, "serv.resync" }, { IDC_RESTOCKALL, "serv.restock" },
			{ IDC_KILL, "kill - Ziel anklicken" }, { IDC_REMOVE, "remove - Ziel anklicken" },
			{ IDC_NUKE, "nuke - zwei Ecken aufziehen, alle Items darin werden gel\xF6scht" },
		};
		for (int i = 0; i < (int)(sizeof(aTips) / sizeof(aTips[0])); i++)
			if (GetDlgItem(aTips[i].uID))
				m_tip.AddTool(GetDlgItem(aTips[i].uID), AxisTr(aTips[i].pszTip));
		m_tip.Activate(TRUE);
	}
}

BOOL CGeneralTab::PreTranslateMessage(MSG* pMsg)
{
	if (m_tip.GetSafeHwnd())
		m_tip.RelayEvent(pMsg);
	return CDockingPage::PreTranslateMessage(pMsg);
}
BOOL CGeneralTab::OnSetActive() 
{
	UpdateData();

	return CDockingPage::OnSetActive();
}

void CGeneralTab::OnRconsole() 
{
	if (!m_bRemoteConsoleCreated)
	{
		// Set before Create(): OnInitDialog (with its login dialog) must not re-enter here.
		m_bRemoteConsoleCreated = true;
		m_bRemoteConsoleVisible = true;
		if (!m_rcDlg.Create(IDD_REMOTECONSOLEDLG, this))
		{
			m_bRemoteConsoleCreated = false;
			m_bRemoteConsoleVisible = false;
			return;
		}
		m_rcDlg.ShowWindow(SW_SHOW);
		m_cbRemoteConsole.SetWindowText(CMsg("IDS_HIDE_CONSOLE"));
	}
	else
	{
		if (m_bRemoteConsoleVisible)
		{
			// Hide it
			m_rcDlg.ShowWindow(SW_HIDE);
			m_bRemoteConsoleVisible = false;
			m_cbRemoteConsole.SetWindowText(CMsg("IDS_SHOW_CONSOLE"));
		}
		else
		{
			// Show it
			m_rcDlg.ShowWindow(SW_SHOW);
			m_bRemoteConsoleVisible = true;
			this->m_cbRemoteConsole.SetWindowText(CMsg("IDS_HIDE_CONSOLE"));
		}
	}
}

void CGeneralTab::OnAdmin() 
{
	CString csCmd;
	csCmd.Format("%sadmin", Main->m_csCommandPrefix);
	SendToUO(csCmd);	
}

void CGeneralTab::OnIteminfo()
{
	CString csCmd;
	csCmd.Format("%sinfo", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSetedit()
{
	CString csCmd;
	csCmd.Format("%sxedit", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnClients()
{
	CString csCmd;
	csCmd.Format("%sshow serv.clients", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnServerinfo()
{
	CString csCmd;
	csCmd.Format("%sinformation", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnVersion()
{
	CString csCmd;
	csCmd.Format("%sversion", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnLink()
{
	CString csCmd;
	csCmd.Format("%slink", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnFlip()
{
	CString csCmd;
	csCmd.Format("%sxflip", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnShrink()
{
	CString csCmd;
	csCmd.Format("%sshrink", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnDupe()
{
	CString csCmd;
	csCmd.Format("%sdupe", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnRemove()
{
	CString csCmd;
	if (AfxMessageBox(AXT("Entfernen: das angeklickte Objekt wird endg\xFCltig gel\xF6scht. Fortfahren?"), MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
		return;
	csCmd.Format("%sremove", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnNuke()
{
	CString csCmd;
	if (AfxMessageBox(AXT("Nuke: alle Items in der aufgezogenen Fl\xE4" "che werden gel\xF6scht. Fortfahren?"), MB_OKCANCEL | MB_ICONWARNING) != IDOK)
		return;
	csCmd.Format("%snuke", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnBuy()
{
	SendToUO("buy");
}

void CGeneralTab::OnSell()
{
	SendToUO("sell");
}

void CGeneralTab::OnStock()
{
	CString csCmd;
	csCmd.Format("%sbank 1a", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPurchases()
{
	CString csCmd;
	csCmd.Format("%sbank 1b", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSamples()
{
	CString csCmd;
	csCmd.Format("%sbank 1c", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnXrestock()
{
	CString csCmd;
	csCmd.Format("%sxrestock", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSnow()
{
	CString csCmd;
	csCmd.Format("%ssector.snow", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnRain()
{
	CString csCmd;
	csCmd.Format("%ssector.rain", Main->m_csCommandPrefix);
	SendToUO(csCmd);	
}

void CGeneralTab::OnDry()
{
	CString csCmd;
	csCmd.Format("%ssector.dry", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSetlight()
{
	CString csCmd;
	CString csVal;
	m_csSliderValue.GetWindowText(csVal);
	csCmd.Format("%ssector.light %s", Main->m_csCommandPrefix, csVal);
	SendToUO(csCmd);
}

void CGeneralTab::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar->m_hWnd == m_cscSlider.m_hWnd)
	{
		CString csValue;
		csValue.Format("%02ld", m_cscSlider.GetPos());
		m_csSliderValue.SetWindowText(csValue);
		return;
	}
	CDockingPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGeneralTab::OnInvulnerable()
{
	CString csCmd;
	csCmd.Format("%sinvulnerable", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSetallmove()
{
	CString csCmd;
	csCmd.Format("%sallmove", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnInvisible()
{
	CString csCmd;
	csCmd.Format("%sinvisible", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnFix()
{
	CString csCmd;
	csCmd.Format("%sfix", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnTele()
{
	CString csCmd;
	csCmd.Format("%stele", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSethearall()
{
	CString csCmd;
	csCmd.Format("%shearall", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnGmtoggle()
{
	CString csCmd;
	csCmd.Format("%sgm", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSetdetail()
{
	CString csCmd;
	csCmd.Format("%sdetail", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnNightsight()
{
	CString csCmd;
	csCmd.Format("%snightsight", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSetdebug()
{
	CString csCmd;
	csCmd.Format("%sdebug", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnJail()
{
	CString csCmd;
	csCmd.Format("%sjail", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnForgive()
{
	CString csCmd;
	csCmd.Format("%sforgive", Main->m_csCommandPrefix);
	SendToUO(csCmd);	
}

void CGeneralTab::OnKill()
{
	CString csCmd;
	if (AfxMessageBox(AXT("T\xF6ten: der angeklickte Charakter stirbt. Fortfahren?"), MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
		return;
	csCmd.Format("%skill", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnDisconnect()
{
	CString csCmd;
	csCmd.Format("%sxdisconnect", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnResurect()
{
	CString csCmd;
	csCmd.Format("%sxresurrect", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPageon()
{
	CString csCmd;
	csCmd.Format("%spage on", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPagelist()
{
	CString csCmd;
	csCmd.Format("%spage list", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPageplayer()
{
	CString csCmd;
	csCmd.Format("%spage player", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPagedisconnect()
{
	if (AfxMessageBox(CMsg("IDS_WARNING_DISCONNECT"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		CString csCmd;
		csCmd.Format("%spage disconnect", Main->m_csCommandPrefix);
		SendToUO(csCmd);
	}
}

void CGeneralTab::OnPagekick()
{
	if (AfxMessageBox(CMsg("IDS_WARNING_BAN"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		CString csCmd;
		csCmd.Format("%spage ban", Main->m_csCommandPrefix);
		SendToUO(csCmd);
	}
}

void CGeneralTab::OnPageoff()
{
	CString csCmd;
	csCmd.Format("%spage off", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPagequeue()
{
	CString csCmd;
	csCmd.Format("%spage queue", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnPageorigin()
{
	CString csCmd;
	csCmd.Format("%spage origin", Main->m_csCommandPrefix);
	SendToUO(csCmd);	
}

void CGeneralTab::OnPagejail()
{
	if(AfxMessageBox(CMsg("IDS_WARNING_JAIL"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		CString csCmd;
		csCmd.Format("%spage jail", Main->m_csCommandPrefix);
		SendToUO(csCmd);
	}
}

void CGeneralTab::OnPagedelete()
{
	CString csCmd;
	csCmd.Format("%spage delete", Main->m_csCommandPrefix);
	SendToUO(csCmd);	
}

void CGeneralTab::OnWorldsave()
{
	CString csCmd;
	csCmd.Format("%sserv.save", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnSavestatics()
{
	CString csCmd;
	csCmd.Format("%sserv.savestatics", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnResync()
{
	CString csCmd;
	csCmd.Format("%sserv.resync", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}

void CGeneralTab::OnRestockall()
{
	CString csCmd;
	csCmd.Format("%sserv.restock", Main->m_csCommandPrefix);
	SendToUO(csCmd);
}