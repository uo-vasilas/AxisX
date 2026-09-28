// ScriptInfo.cpp : implementation file
//

#include "stdafx.h"
#include "AxisX.h"
#include "ScriptInfo.h"


// CScriptInfo dialog

IMPLEMENT_DYNAMIC(CScriptInfo, CDialog)


CScriptInfo::CScriptInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptInfo::IDD, pParent)
{
	UINT	uiSize			= sizeof(CHARFORMAT);
	DWORD	dwMask			= CFM_COLOR | CFM_FACE | CFM_SIZE;
	LONG	lHeight			= 160;	// 8 point => 160 * (1/20)

	// Initialise the Key CHARFORMAT
	m_cfKey.cbSize				= uiSize;
	m_cfKey.dwMask				= dwMask;
	m_cfKey.dwEffects			= 0;
	m_cfKey.yHeight			= lHeight;
	m_cfKey.crTextColor		= AxisClr(AXC_MUTED2);	// colour from the palette (light/dark)
	strcpy(m_cfKey.szFaceName,	_T("Consolas"));

		// Initialise the Value CHARFORMAT
	m_cfValue.cbSize				= uiSize;
	m_cfValue.dwMask				= dwMask;
	m_cfValue.dwEffects			= 0;
	m_cfValue.yHeight			= lHeight;
	m_cfValue.crTextColor		= AxisClr(AXC_SYN_HEAD);
	strcpy(m_cfValue.szFaceName,	_T("Consolas"));
}

CScriptInfo::~CScriptInfo()
{
}

void CScriptInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO_SCREEN, m_ceScreenInfo);
}


BEGIN_MESSAGE_MAP(CScriptInfo, CDialog)
END_MESSAGE_MAP()

// CScriptInfo message handlers

BOOL CScriptInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();

		CString csTitle;
		csTitle.Format(AXT("Skript-Info - %s"),pObject->m_csID);
		this->SetWindowText(csTitle);

		AddText(AXT("Skriptdatei:"), m_cfKey);
		AddText(pObject->m_csFilename,m_cfValue);
		AddText("",m_cfKey);

		AddText(AXT("Skript-ID:"), m_cfKey);
		AddText(pObject->m_csValue,m_cfValue);
		if (pObject->m_bType == TYPE_AREA)
		{
			AddText(AXT("Ort:"), m_cfKey);
			AddText(pObject->m_csID,m_cfValue);
			AddText(AXT("Karte:"), m_cfKey);
			AddText(pObject->m_csDisplay,m_cfValue);
		}
		else
		{
			AddText(AXT("Defname:"), m_cfKey);
			AddText(pObject->m_csID,m_cfValue);
			WORD wDisp = -1;
			if ( alltoi(pObject->m_csDisplay,pObject->m_csValue) != 0 )
				wDisp = (WORD) alltoi(pObject->m_csDisplay,pObject->m_csValue);
			CString cs_Line;
			cs_Line.Format("%s (0x%X)",pObject->m_csDisplay,wDisp);
			AddText(AXT("Grafik:"), m_cfKey);
			AddText(cs_Line,m_cfValue);
			AddText(AXT("Farbe:"), m_cfKey);
			AddText(pObject->m_csColor,m_cfValue);
		}
		AddText("",m_cfKey);

		AddText(AXT("Kategorie:"), m_cfKey);
		AddText(pObject->m_csCategory,m_cfValue);
		AddText(AXT("Unterbereich:"), m_cfKey);
		AddText(pObject->m_csSubsection,m_cfValue);
		AddText(AXT("Name:"), m_cfKey);
		AddText(pObject->m_csDescription,m_cfValue);

		m_ceScreenInfo.SetSel(0,0);
		m_ceScreenInfo.SetOptions(ECOOP_OR,ECO_SAVESEL);
	return TRUE;
}

void CScriptInfo::AddText(CString csText, CHARFORMAT& cf)
{
	csText += AXT("\n");

	if (m_ceScreenInfo)
	{
		int nOldLines = 0, nNewLines = 0, nScroll = 0;
		long nInsertionPoint = m_ceScreenInfo.GetWindowTextLength();
		nOldLines = m_ceScreenInfo.GetLineCount();
		m_ceScreenInfo.SetSel(nInsertionPoint, -1);
		m_ceScreenInfo.SetSelectionCharFormat(cf);
		m_ceScreenInfo.ReplaceSel(csText);
		nNewLines = m_ceScreenInfo.GetLineCount();
		nScroll = nNewLines - nOldLines;
		m_ceScreenInfo.LineScroll(nScroll);
		UpdateData(false);
	}
}
