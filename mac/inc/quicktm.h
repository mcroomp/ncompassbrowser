class CFormatQuickTimeItem : public CFormatItem
	{
public:
	CFormatQuickTimeItem( LPCSTR filename, BOOL has_controller, const CRect& extent);
	~CFormatQuickTimeItem();

	// platform dependent
	virtual void OnRedraw(const char *plain_text, BOOL hilight);
	virtual void OnClick(long x,long y);
	virtual void OnScroll();
	virtual void OnShow(BOOL flag);
	
private:
	Movie m_movie;
	BOOL m_broken;
	};