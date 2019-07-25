#ifndef LYRICSKEEPER_H
#define LYRICSKEEPER_H

#include "LyricServer.h"

namespace Lyrics
{
	class LyricsKeeper : public Server
	{
		public:
			QString name() const override;
			QString address() const override;
			QMap<QString, QString> replacements() const override;
			QString call_policy() const override;
			QMap<QString, QString> start_end_tag() const override;
			bool is_start_tag_included() const override;
			bool is_end_tag_included() const override;
			bool is_numeric() const override;
			bool is_lowercase() const override;
			QString error_string() const override;
	};
}

#endif // LYRICSKEEPER_H
