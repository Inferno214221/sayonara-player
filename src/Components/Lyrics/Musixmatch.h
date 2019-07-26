#ifndef MUSIXMATCH_H
#define MUSIXMATCH_H

#include "LyricServer.h"

namespace Lyrics
{
	class Musixmatch : public SearchableServer
	{
		public:
			QString name() const override;

			Server::StartEndTags start_end_tag() const override;
			bool is_start_tag_included() const override;
			bool is_end_tag_included() const override;
			bool is_numeric() const override;
			bool is_lowercase() const override;
			QString error_string() const override;

			bool can_fetch_directly() const;
			QString search_address(QString artist, QString title) const;
			QString parse_search_result(const QString& search_result);

			Lyrics::Server::Replacements replacements() const;
	};
}

#endif // MUSIXMATCH_H
