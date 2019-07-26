#ifndef OLDIELYRICS_H
#define OLDIELYRICS_H

#include "LyricServer.h"

namespace Lyrics
{
	class OldieLyrics : public Server
	{
		public:
			QString name() const override;
			QString address() const override;
			Server::Replacements replacements() const override;
			QString call_policy() const override;
			Server::StartEndTags start_end_tag() const override;
			bool is_start_tag_included() const override;
			bool is_end_tag_included() const override;
			bool is_numeric() const override;
			bool is_lowercase() const override;
			QString error_string() const override;
	};
}

#endif // OLDIELYRICS_H
