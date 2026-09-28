// ToolBar.cpp : floating mini bar shown while the main window is minimized

#include "stdafx.h"
#include "AxisX.h"
#include "AxisXDlg.h"
#include "ToolBar.h"

// Defined in AxisXDlg.cpp; shared with the sidebar.
void DrawNavIcon(HDC hdc, int iconType, const RECT& rc, COLORREF color);
int AxisGetNavPages(int* aIcon, CPropertyPage** aPage, int nMax);

static COLORREF MiniBarBkColor()     { return AxisClr(AXC_SIDEBAR); }
static COLORREF MiniBarBorderColor() { return AxisClr(AXC_BORDER); }
static COLORREF MiniBarHoverColor()  { return AxisClr(AXC_BUTTON_HOVER); }

IMPLEMENT_DYNAMIC(CAxisXLBar, CDialog)

CAxisXLBar::CAxisXLBar(CWnd* pParent /*=NULL*/)
	: CDialog(CAxisXLBar::IDD, pParent)
{
	m_nItems = 0;
	m_iHover = -1;
	m_bTracking = false;
}

CAxisXLBar::~CAxisXLBar()
{
}

BEGIN_MESSAGE_MAP(CAxisXLBar, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
END_MESSAGE_MAP()

BOOL CAxisXLBar::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nItems = AxisGetNavPages(m_aIcon, m_aPage, MAX_ITEMS - 1);
	m_aIcon[m_nItems] = 18;		// Restore button
	m_aPage[m_nItems] = NULL;
	m_nItems++;

	// Size from the icon count; the border is drawn in OnPaint().
	CRect rcWnd(0, 0, GRIP + PAD + m_nItems * CELL + (PAD * 3), CELL + 2 * PAD);
	SetWindowPos(&wndTopMost, 0, 0, rcWnd.Width(), rcWnd.Height(), SWP_NOMOVE | SWP_NOACTIVATE);

	m_tip.Create(this);
	for (int i = 0; i < m_nItems; i++)
	{
		CString csTip;
		if (m_aPage[i] == NULL)
			csTip = AXT("Axis X wieder gro\xDF machen (oder Doppelklick auf die Leiste)");
		else
			csTip = (m_aPage[i]->m_psp.pszTitle) ? m_aPage[i]->m_psp.pszTitle : _T("");
		m_tip.AddTool(this, csTip, ItemRect(i), i + 1);
	}
	m_tip.AddTool(this, AXT("Ziehen zum Verschieben"), CRect(0, 0, GRIP + PAD, rcWnd.Height()), 100);
	m_tip.Activate(TRUE);
	return TRUE;
}

CRect CAxisXLBar::ItemRect(int i) const
{
	int x = GRIP + PAD + i * CELL;
	if (m_aPage[i] == NULL)
		x += PAD * 2;	// Gap before the restore button
	return CRect(x, PAD, x + CELL, PAD + CELL);
}

int CAxisXLBar::HitTest(CPoint pt) const
{
	for (int i = 0; i < m_nItems; i++)
		if (ItemRect(i).PtInRect(pt))
			return i;
	return -1;
}

void CAxisXLBar::ShowBar()
{
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	int x = (int) Main->GetRegistryDword("MiniBar X", 0x7fffffff);
	int y = (int) Main->GetRegistryDword("MiniBar Y", 0x7fffffff);
	CRect rcWork;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
	// Invalid or off-screen position: place it bottom centre above the taskbar.
	POINT ptCheck = { x + rcWnd.Width() / 2, y + rcWnd.Height() / 2 };
	if (x == 0x7fffffff || y == 0x7fffffff || MonitorFromPoint(ptCheck, MONITOR_DEFAULTTONULL) == NULL)
	{
		x = rcWork.left + (rcWork.Width() - rcWnd.Width()) / 2;
		y = rcWork.bottom - rcWnd.Height() - 8;
	}
	SetWindowPos(&wndTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

void CAxisXLBar::SavePosition()
{
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	Main->PutRegistryDword("MiniBar X", (DWORD) rcWnd.left);
	Main->PutRegistryDword("MiniBar Y", (DWORD) rcWnd.top);
}

void CAxisXLBar::Activate(int i)
{
	CAxisXDlg* pMain = (CAxisXDlg*) Main->m_pMainWnd;
	if (pMain)
		pMain->RestoreFromMiniBar((i >= 0 && i < m_nItems) ? m_aPage[i] : NULL);
}

void CAxisXLBar::OnCancel()
{
	// Esc / Alt+F4 restores the main window.
	Activate(-1);
}

BOOL CAxisXLBar::PreTranslateMessage(MSG* pMsg)
{
	if (m_tip.GetSafeHwnd())
		m_tip.RelayEvent(pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CAxisXLBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;	// Painted in OnPaint()
}

void CAxisXLBar::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* pOldBmp = dcMem.SelectObject(&bmp);

	dcMem.FillSolidRect(&rc, MiniBarBkColor());

	// Border
	CPen penBorder(PS_SOLID, 1, MiniBarBorderColor());
	CPen* pOldPen = dcMem.SelectObject(&penBorder);
	CGdiObject* pOldBrush = dcMem.SelectStockObject(NULL_BRUSH);
	dcMem.Rectangle(&rc);

	// Drag grip: two rows of dots
	for (int y = rc.top + 10; y < rc.bottom - 8; y += 4)
	{
		dcMem.FillSolidRect(PAD + 2, y, 2, 2, MiniBarBorderColor());
		dcMem.FillSolidRect(PAD + 6, y, 2, 2, MiniBarBorderColor());
	}

	// Separator before the restore button
	CRect rcRestore = ItemRect(m_nItems - 1);
	dcMem.FillSolidRect(rcRestore.left - PAD - 1, rc.top + 8, 1, rc.Height() - 16, MiniBarBorderColor());

	for (int i = 0; i < m_nItems; i++)
	{
		CRect rcItem = ItemRect(i);
		bool bHover = (i == m_iHover);
		bool bActive = false;
		CAxisXDlg* pMain = (CAxisXDlg*) Main->m_pMainWnd;
		if (m_aPage[i] && pMain && pMain->GetActivePage() == m_aPage[i])
			bActive = true;
		if (bHover || bActive)
			dcMem.FillSolidRect(&rcItem, MiniBarHoverColor());
		if (bActive)
			dcMem.FillSolidRect(rcItem.left + 6, rcItem.bottom - 2, rcItem.Width() - 12, 2, DarkAccentColor());
		CRect rcIcon(rcItem.CenterPoint().x - 7, rcItem.CenterPoint().y - 7, rcItem.CenterPoint().x + 7, rcItem.CenterPoint().y + 7);
		COLORREF cr = (bHover || m_aPage[i] == NULL) ? AxisAccentTextColor() : DarkTextColor();
		DrawNavIcon(dcMem.GetSafeHdc(), m_aIcon[i], rcIcon, cr);
	}

	dcMem.SelectObject(pOldBrush);
	dcMem.SelectObject(pOldPen);
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pOldBmp);
}

void CAxisXLBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd, 0 };
		m_bTracking = TrackMouseEvent(&tme) ? true : false;
	}
	int iHit = HitTest(point);
	if (iHit != m_iHover)
	{
		m_iHover = iHit;
		Invalidate(FALSE);
	}
	::SetCursor(::LoadCursor(NULL, iHit >= 0 ? IDC_HAND : IDC_SIZEALL));
	CDialog::OnMouseMove(nFlags, point);
}

LRESULT CAxisXLBar::OnMouseLeave(WPARAM, LPARAM)
{
	m_bTracking = false;
	if (m_iHover != -1)
	{
		m_iHover = -1;
		Invalidate(FALSE);
	}
	return 0;
}

void CAxisXLBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int iHit = HitTest(point);
	if (iHit >= 0)
	{
		Activate(iHit);
		return;
	}
	// Anywhere else: drag the bar.
	SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0);
}

void CAxisXLBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (HitTest(point) < 0)
		Activate(-1);
}

LRESULT CAxisXLBar::OnExitSizeMove(WPARAM, LPARAM)
{
	SavePosition();
	return 0;
}
