#include "cross_p.h"

#include "mimeload.h"
#include "readhtml.h"


CMimeObject::CMimeObject(MIME_OBJECT_TYPE object_type, LPCSTR url,
	LPCSTR mime_type)
	: m_mime_type(mime_type), m_url(url), m_mime_object_type(object_type)
	{
	m_load_state = LOAD_STATE_LOADING;
	}


CMimeObject::~CMimeObject()
	{
	}


void CMimeObject::Notify(INT32)
	{
	}


POSITION CMimeObject::GetFirstTagPos() const
	{
	ASSERT_LOCKED(this);
	return NULL;
	}


const CTag *CMimeObject::GetNextTag(POSITION& walk) const
	{
	ASSERT_LOCKED(this);
	walk = NULL;
	return NULL;
	}


CBigString CMimeObject::GetPlainText() const
	{
	ASSERT_LOCKED(this);
	return CBigString();
	}


POSITION CMimeObject::FindTagPos(const CTag *) const
	{
	ASSERT_LOCKED(this);
	return NULL;
	}


CString CMimeObject::GetTitle() const
	{
	ASSERT_LOCKED(this);
	return CString();
	}


INT32 CMimeObject::GetBackgroundColor() const
	{
	ASSERT_LOCKED(this);
	return -1;
	}


INT32 CMimeObject::GetTextColor() const
	{
	ASSERT_LOCKED(this);
	return -1;
	}


INT32 CMimeObject::GetHotlinkColor() const
	{
	ASSERT_LOCKED(this);
	return -1;
	}


INT32 CMimeObject::GetOldHotlinkColor() const
	{
	ASSERT_LOCKED(this);
	return -1;
	}


CString CMimeObject::GetBackgroundPicture() const
	{
	ASSERT_LOCKED(this);
	return CString();
	}


BOOL CMimeObject::UsesInternalViewer() const
	{
	return TRUE;
	}


void CMimeObject::LaunchViewer()
	{
	}


CString CombineURL(LPCSTR old_url, LPCSTR new_url)
	{
	LPCSTR p, p2;
	CString old_sitename;
	CString old_dir;

	p = strstr(old_url, "//");
	if (!p)
		goto give_up;

	p2 = strchr(p + 2, '/');
	if (!p2)
		{
		old_sitename = old_url;
		old_dir = "/";
		}
	else
		{
		old_sitename = CString(old_url, p2 - old_url);
		old_dir = p2;
		}

	if (new_url[0] == '/' && new_url[1] != '/')
		return old_sitename + new_url;
	else if (new_url[0] != '/' && strstr(new_url, "//") == 0 &&
		strchr(new_url, ':') == 0)
		{
		const char *new_url_p = new_url;
		const char *begin_old_dir = old_dir;
		const char *directory = strrchr(begin_old_dir, '/');

		while(directory >= begin_old_dir &&
			memcmp(new_url_p, "../", 3) == 0)
			{
			directory--;
			while(directory >= begin_old_dir)
				{
				if (*directory == '/')
					break;
				directory--;
				}
			new_url_p += 3;
			}

		if (directory < begin_old_dir)
			goto give_up;

		return old_sitename +
			CString(begin_old_dir, directory - begin_old_dir + 1) +
			CString(new_url_p);
		}

give_up:
	return CString(new_url);
	}
