#include "MetroLyrics.h"

QString Lyrics::MetroLyrics::name() const
{
	return "MetroLyrics";
}

QString Lyrics::MetroLyrics::address() const
{
	return "http://www.metrolyrics.com";
}

Lyrics::Server::Replacements Lyrics::MetroLyrics::replacements() const
{
	return Lyrics::Server::Replacements
	{
		{"&", "and"},
		{" ", "-"}
	};
}

QString Lyrics::MetroLyrics::call_policy() const
{
	return "<SERVER>/<TITLE>-lyrics-<ARTIST>.html";
}

Lyrics::Server::StartEndTags Lyrics::MetroLyrics::start_end_tag() const
{
	return Lyrics::Server::StartEndTags
	{
		{"class=\"lyric-message\">", "</p>"},
		{"<div id=\"lyrics-body-text\" class=\"js-lyric-text\">", "</p>"}
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
