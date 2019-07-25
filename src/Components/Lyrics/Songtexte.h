#ifndef SONGTEXTE_H
#define SONGTEXTE_H

#include "LyricServer.h"

namespace Lyrics
{
	class Songtexte : public Lyrics::SearchableServer
	{
		public:
			QString name() const;
			QMap<QString, QString> start_end_tag() const;
			bool is_start_tag_included() const;
			bool is_end_tag_included() const;
			bool is_numeric() const;
			bool is_lowercase() const;
			QString error_string() const;
			bool can_fetch_directly() const;
			QString search_address(QString artist, QString title) const;
			QString parse_search_result(const QString& search_result);
	};
}

#endif // SONGTEXTE_H
