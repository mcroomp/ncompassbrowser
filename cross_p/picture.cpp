#include "cross_p.h"

#ifndef _BITBLT_H_
#include "bitblt.h"
#endif

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif

#ifndef _PICTURE_H_
#include "picture.h"	
#endif

CPicture::CPicture( LPCSTR url, LPCSTR mime_type, const PICTURE_FORMAT_INFO& request_format )
	: CMimeObject(MIME_OBJECT_PICTURE, url, mime_type)
	{
	}

CPicture::~CPicture()
	{
	}

static CTagImage tag_image(ALIGN_BASELINE, "", 0, 0, 0,0, 0, FALSE, TRUE );
static CTag tag_newline( TAG_NEWLINE );


POSITION CPicture::FindTagPos( const CTag *tag) const
	{
	ASSERT_LOCKED(this);

	if (tag == &tag_image)
		return (POSITION)1;
	else if (tag == &tag_newline)
		return (POSITION)2;
	else
		{
		ASSERT(FALSE);
		return NULL;
		}
	}

POSITION CPicture::GetFirstTagPos() const
	{
	ASSERT_LOCKED(this);

	return (POSITION)1;
	}

const CTag * CPicture::GetNextTag(POSITION &walk) const
	{
	ASSERT_LOCKED(this);

	if ( walk == (POSITION)1)
		{
		tag_image.m_url = GetURL();
		walk = (POSITION)2;
		return &tag_image;
		}
	else if (walk == (POSITION)2)
		{
		walk = NULL;
		return &tag_newline;
		}
	else
		{
		ASSERT(FALSE);
		return NULL;
		}
	}

	
