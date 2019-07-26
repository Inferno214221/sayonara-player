#include "ELyrics.h"

QString Lyrics::ELyrics::name() const
{
	return "eLyrics";
}

QString Lyrics::ELyrics::address() const
{
	return "http://www.elyrics.net/read";
}

Lyrics::Server::Replacements Lyrics::ELyrics::replacements() const
{
	return Lyrics::Server::Replacements
	{
		{"the ", ""},
		{"The ", ""},
		{" ", "-"},
		{"'", "_"}
	};
}

QString Lyrics::ELyrics::call_policy() const
{
	return "<SERVER>/<FIRST_ARTIST_LETTER>/<ARTIST>-lyrics/<TITLE>-lyrics.html";
}

Lyrics::Server::StartEndTags Lyrics::ELyrics::start_end_tag() const
{
	return Lyrics::Server::StartEndTags
	{
		{"<div id=\"inlyr\"", "</div>"},
		{"<div id='inlyr'", "</div>"},
		{"not found on eLyrics.net. <br> ", "</strong>"}
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
