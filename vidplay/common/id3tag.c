/*****************************************************************************
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: id3tag.c 115 2004-11-27 12:55:48Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"
#include "libid3tag/id3tag.h"

// for palm simulator
#undef free

const char* const Id3[] =
{
	ID3_FRAME_TITLE, "TITLE=",
	ID3_FRAME_ARTIST, "ARTIST=",
	ID3_FRAME_ALBUM,  "ALBUM=",
	ID3_FRAME_GENRE,  "GENRE=",
	ID3_FRAME_COMMENT,  "COMMENT=",
	NULL,NULL
};

int Id3TagQuery(const void* Ptr, int Len)
{
	return id3_tag_query((const uint8_t*)Ptr,Len);
}

void Id3TagParse(const void* Ptr, int Len, pin* Pin)
{
	struct id3_tag *Tag;
	Tag = id3_tag_parse(Ptr, Len);
	if (Tag) 
	{
		struct id3_frame const *Frame;
		union id3_field const *Field;
		id3_ucs4_t const *UCS4;
		id3_ucs4_t const *p;
		int n,Count;
		const char* const* i;
		tchar_t Comment[256];

		for (i=Id3;i[0];i+=2)
		{
			Frame = id3_tag_findframe(Tag, i[0], 0);
			if (Frame)
			{
				Field = &Frame->fields[1];
				Count = id3_field_getnstrings(Field);

				for (n=0;n<Count;++n) 
				{
					bool_t Latin1 = 1;

					UCS4 = id3_field_getstrings(Field,n);

					for (p=UCS4;*p;++p)
						if (*p >= 256)
						{
							Latin1 = 0;
							break;
						}

					if (Latin1 || (Tag->restrictions & ID3_TAG_RESTRICTION_TEXTENCODING_LATIN1_UTF8))
					{
						id3_latin1_t *Latin1 = id3_ucs4_latin1duplicate(UCS4);
						if (Latin1)
						{
							AsciiToTcs(Comment,i[1],256);
							StrToTcs(Comment+tcslen(Comment),(char*)Latin1,256-tcslen(Comment));
							free(Latin1);
							Pin->Node->Set(Pin->Node,Pin->No,Comment,sizeof(Comment));
						}
					}
					else
					{
						id3_utf16_t *UTF16 = id3_ucs4_utf16duplicate(UCS4);
						if (UTF16)
						{
							AsciiToTcs(Comment,i[1],256);
							WcsToTcs(Comment+tcslen(Comment),UTF16,256-tcslen(Comment));
							free(UTF16);
							Pin->Node->Set(Pin->Node,Pin->No,Comment,sizeof(Comment));
						}
					}
				}
			}
		}

		id3_tag_delete(Tag);
	}
}

bool_t Id3Genre(int No, tchar_t* Genre, int Size)
{
	id3_utf16_t *UTF16;
	id3_ucs4_t const *UCS4 = No>=0 ? id3_genre_index(No):NULL;
	if (!UCS4) 
		return 0;

	UTF16 = id3_ucs4_utf16duplicate(UCS4);
	if (!UTF16)
		return 0;

	WcsToTcs(Genre,UTF16,Size);
	free(UTF16);
	return 1;
}
