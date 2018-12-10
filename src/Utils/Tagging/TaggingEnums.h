#ifndef TAGGINGENUMS
#define TAGGINGENUMS

#include <taglib/audioproperties.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4tag.h>
#include <taglib/xiphcomment.h>

namespace Tagging
{
		/**
		 * @brief The Quality enum
		 */
		enum class Quality : unsigned char
		{
			Fast=TagLib::AudioProperties::Fast,
			Standard=TagLib::AudioProperties::Average,
			Quality=TagLib::AudioProperties::Accurate,
			Dirty
		};

		enum class TagType : unsigned char
		{
			ID3v1=0,
			ID3v2,
			Xiph,
			MP4,
			Unsupported,
			Unknown
		};

		struct ParsedTag
		{
			TagLib::Tag* tag;
			TagType type;

			TagLib::MP4::Tag* mp4_tag() const
			{
				return dynamic_cast<TagLib::MP4::Tag*>(this->tag);
			}

			TagLib::ID3v2::Tag* id3_tag() const
			{
				return dynamic_cast<TagLib::ID3v2::Tag*>(this->tag);
			}

			TagLib::Ogg::XiphComment* xiph_tag() const
			{
				return dynamic_cast<TagLib::Ogg::XiphComment*>(this->tag);
			}
		};
}
#endif // TAGGINGENUMS

