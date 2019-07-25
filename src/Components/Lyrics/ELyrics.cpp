#include "ELyrics.h"

QString Lyrics::ELyrics::name() const
{
	return "eLyrics";
}

QString Lyrics::ELyrics::address() const
{
	return "http://www.elyrics.net/read";
}

QMap<QString, QString> Lyrics::ELyrics::replacements() const
{
	return QMap<QString, QString>
	{
		{" ", "-"},
		{"the ", ""},
		{"The ", ""},
		{"'", "_"}
	};
}

QString Lyrics::ELyrics::call_policy() const
{
	return "<SERVER>/<FIRST_ARTIST_LETTER>/<ARTIST>-lyrics/<TITLE>-lyrics.html";
}

QMap<QString, QString> Lyrics::ELyrics::start_end_tag() const
{
	return QMap<QString, QString>
	{
		{"lyrics</strong><br>", "</div>"},
		{"<div id='inlyr' style='font-size:14px;'>", "</div>"}
	};
}

bool Lyrics::ELyrics::is_start_tag_included() const
{
	return false;
}

bool Lyrics::ELyrics::is_end_tag_included() const
{
	return false;
}

bool Lyrics::ELyrics::is_numeric() const
{
	return false;
}

bool Lyrics::ELyrics::is_lowercase() const
{
	return false;
}

QString Lyrics::ELyrics::error_string() const
{
	return QString("Error 404");
}
