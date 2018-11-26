#ifndef TAGGINGLYRICS_H
#define TAGGINGLYRICS_H

class MetaData;
class QString;

namespace Tagging
{
	namespace Lyrics
	{
		bool write_lyrics(const MetaData& md, const QString& lyrics);
		bool extract_lyrics(const MetaData& md, QString& lyrics);
		bool is_lyrics_supported(const QString& filepath);
	}
}

#endif // TAGGINGLYRICS_H
