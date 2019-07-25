#ifndef METROLYRICS_H
#define METROLYRICS_H

#include "LyricServer.h"

namespace Lyrics
{
	class MetroLyrics : public Server
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
#endif // METROLYRICS_H
