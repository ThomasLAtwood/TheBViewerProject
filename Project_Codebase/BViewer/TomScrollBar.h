#pragma once
// TomScrollBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////

class TomScrollBar : public CScrollBar
{
// Construction
public:
	TomScrollBar( char *pButtonText, int ScrollBarWidth, int ScrollBarHeight, int MinScrollPosition, int MaxScrollPosition,
				int PagingSize, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD ScrollBarStyle, UINT nID );

// Attributes
											// this button is a member.
public:
	char			*m_ButtonText;
	int				m_ScrollBarWidth;
	int				m_ScrollBarHeight;
	int				m_MinScrollPosition;
	int				m_MaxScrollPosition;
	int				m_PagingSize;
	int				m_FontHeight;
	int				m_FontWidth;
	int				m_FontWeight;
	COLORREF		m_TextColor;
	COLORREF		m_OriginalIdleBkgColor;
	COLORREF		m_ActivatedBkgdColor;
	COLORREF		m_VisitedBkgdColor;
	DWORD			m_ScrollBarStyle;
						#define BUTTON_TEXT_LEFT_JUSTIFIED				0x00000001
						#define BUTTON_TEXT_RIGHT_JUSTIFIED				0x00000002
						#define BUTTON_TEXT_HORIZONTALLY_CENTERED		0x00000004
						#define BUTTON_TEXT_TOP_JUSTIFIED				0x00000008
						#define BUTTON_TEXT_BOTTOM_JUSTIFIED			0x00000010
						#define BUTTON_TEXT_VERTICALLY_CENTERED			0x00000020
						#define BUTTON_VISIBLE							0x00000040
						#define BUTTON_INVISIBLE						0x00000080
						#define BUTTON_MULTILINE						0x00000100
						#define BUTTON_CLIP								0x00000200
						#define BUTTON_PUSHBUTTON						0x00010000
						#define BUTTON_CHECKBOX							0x00020000
						#define GROUP_STATIC							0x00040000		// Used to identify static groups.
						#define GROUP_EDIT								0x00080000		// Used to identify edit groups.
						#define BUTTON_FROZEN							0x00100000

						#define SCROLLBAR_HORIZONTAL					0x00200000
						#define SCROLLBAR_VERTICAL						0x00400000

	UINT			m_nObjectID;
	CBrush			m_BkgdBrush;

	COLORREF		m_IdleBkgColor;
	COLORREF		m_PressedBkgColor;
	COLORREF		m_SpecialBkgColor;
	COLORREF		m_DisabledBkgndColor;
	COLORREF		m_Light;
	COLORREF		m_Highlight;
	COLORREF		m_Shadow;
	COLORREF		m_DarkShadow;

public:
	bool			m_EngageSpecialState;
	CFont			m_TextFont;

// Operations
private:
	bool			CreateSpecifiedFont();
	void			DrawFrame( CDC *pDC, CRect rc );
	void			DrawFilledRect( CDC *pDC, CRect rc, COLORREF color );
	void			DrawLine( CDC *pDC, long sx, long sy, long ex, long ey, int nWidth, COLORREF color );
	void			DrawButtonText( CDC *pDC, CRect rc, char *pCaption, COLORREF textcolor );
	
public:
	void			ChangeStatus( DWORD ClearStatus, DWORD SetStatus );
	void			HasBeenPressed( BOOL bHasBeenPressed );
	void			RecomputePressedColor();
	void			TurnOnSpecialState();
	void			TurnOffSpecialState();
	BOOL			SetPosition( int x, int y, CWnd* pParentWnd );
	void			Reinitialize();

	int				GetNewScrollPosition( int ScrollCode, int ScrollValue );
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TomScrollBar)
	public:
		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TomScrollBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(TomScrollBar)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

/////////////////////////////////////////////////////////////////////////////
