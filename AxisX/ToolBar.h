#pragma once
#include "afxwin.h"

// CAxisXLBar - floating mini bar shown while the main window is minimized.
// Clicking an icon restores the main window on that page; drag to move.

class CAxisXLBar : public CDialog
{
	DECLARE_DYNAMIC(CAxisXLBar)

public:
	CAxisXLBar(CWnd* pParent = NULL);
	virtual ~CAxisXLBar();

	enum { IDD = IDD_TOOLBAR };

	// Shows the bar at its saved position, or bottom centre by default.
	void ShowBar();

protected:
	enum { MAX_ITEMS = 16, CELL = 34, GRIP = 14, PAD = 4 };

	int m_nItems;
	int m_aIcon[MAX_ITEMS];
	CPropertyPage* m_aPage[MAX_ITEMS];	// NULL = restore button
	int m_iHover;
	bool m_bTracking;
	CToolTipCtrl m_tip;

	CRect ItemRect(int i) const;
	int HitTest(CPoint pt) const;
	void Activate(int i);
	void SavePosition();

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK() {}
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnExitSizeMove(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
