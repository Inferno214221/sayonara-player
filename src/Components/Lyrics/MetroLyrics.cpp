#include "MetroLyrics.h"

QString Lyrics::MetroLyrics::name() const
{
	return "MetroLyrics";
}

QString Lyrics::MetroLyrics::address() const
{
	return "http://www.metrolyrics.com";
}

QMap<QString, QString> Lyrics::MetroLyrics::replacements() const
{
	return QMap<QString, QString>
	{
		{"&", "and"},
		{" ", "-"}
	};
}

QString Lyrics::MetroLyrics::call_policy() const
{
	return "<SERVER>/<TITLE>-lyrics-<ARTIST>.html";
}

QMap<QString, QString> Lyrics::MetroLyrics::start_end_tag() const
{
	return QMap<QString, QString>
	{
		{"<div id=\"lyrics-body-text\" class=\"js-lyric-text\">", "</div>"}
	};
}

bool Lyrics::MetroLyrics::is_start_tag_included() const
{
	return false;
}

bool Lyrics::MetroLyrics::is_end_tag_included() const
{
	return false;
}

bool Lyrics::MetroLyrics::is_numeric() const
{
	return false;
}

bool Lyrics::MetroLyrics::is_lowercase() const
{
	return true;
}

QString Lyrics::MetroLyrics::error_string() const
{
	return QString("404 page not found");
}
