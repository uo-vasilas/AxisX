#pragma once


// CDockingPage dialog

class CDockingPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDockingPage)

public:
	CDockingPage(){ ASSERT(FALSE); }
	CDockingPage(UINT id, CString csCaption);
	virtual ~CDockingPage();
	bool m_bModifyDlgStylesAndPos;
	bool m_bAlwaysColor;
	CDockingPage * m_dcPropertyPage;
	CDockingPage * m_dcDialogPage;
	CDockingPage * m_dcCurrentPage;
	CString csTitle;
	UINT template_id;

	virtual BOOL OnSetActive();
	virtual void OnCancel();
	void FixTabs(UINT iTab) ;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PreSubclassWindow();

	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnCaptureLayout(WPARAM, LPARAM);

	// Proportional scaling of the page's controls.
	struct CScaleItem { HWND hWnd; CRect rc; bool bBaseFont; };
	CArray<CScaleItem, CScaleItem&> m_aScale;
public:
	CSize m_szBase;	// Template size of the page in pixels
	void EnsureLayoutCaptured();	// Captures the base layout immediately if not done yet
protected:
	bool m_bScaleReady;
	LOGFONT m_lfBase;
	CFont m_fontScaled;
	int m_iFontPct;
	void ApplyScale(int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};
