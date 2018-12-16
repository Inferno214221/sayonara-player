#include "TaggingLyrics.h"
#include "Tagging.h"
#include <taglib/fileref.h>
#include "Tagging/ID3v2/Lyrics.h"
#include "Tagging/Xiph/LyricsFrame.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QString>


bool Tagging::Lyrics::write_lyrics(const MetaData& md, const QString& lyrics_data)
{
	QString filepath = md.filepath();
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::is_valid_file(f)){
		sp_log(Log::Warning, "Tagging") << "Cannot open tags for " << md.filepath();
		return false;
	}

	bool success = false;

	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
			{
				auto id3v2 = parsed_tag.id3_tag();
				ID3v2::LyricsFrame lyrics_frame(id3v2);
				success = lyrics_frame.write(lyrics_data);
			}

			break;

		case Tagging::TagType::Xiph:
			{
				auto xiph = parsed_tag.xiph_tag();
				Xiph::LyricsFrame lyrics_frame(xiph);
				success = lyrics_frame.write(lyrics_data);
			}

			break;

		default:
			return false;
	}

	Q_UNUSED(success)
	return f.save();
}


bool Tagging::Lyrics::extract_lyrics(const MetaData& md, QString& lyrics_data)
{
	lyrics_data.clear();

	QString filepath = md.filepath();
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));

	if(!Tagging::Utils::is_valid_file(f)){
		sp_log(Log::Warning, "Tagging") << "Cannot open tags for " << md.filepath();
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
			{
				auto id3v2 = parsed_tag.id3_tag();
				ID3v2::LyricsFrame lyrics_frame(id3v2);

				if(!lyrics_frame.is_frame_found()){
					return false;
				}

				lyrics_frame.read(lyrics_data);
			}

			break;

		case Tagging::TagType::Xiph:
			{
				auto xiph = parsed_tag.xiph_tag();
				Xiph::LyricsFrame lyrics_frame(xiph);
				lyrics_frame.read(lyrics_data);
			}

			break;

		default:
			return false;
	}

	return !(lyrics_data.isEmpty());
}


bool Tagging::Lyrics::is_lyrics_supported(const QString& filepath)
{
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));

	if(!Tagging::Utils::is_valid_file(f)){
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	return ((tag_type == Tagging::TagType::ID3v2) ||
			(tag_type == Tagging::TagType::Xiph));
}

