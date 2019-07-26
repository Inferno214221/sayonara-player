#include "Genius.h"

QString Lyrics::Genius::name() const
{
	return "Genius";
}

QString Lyrics::Genius::address() const
{
	return "https://genius.com";
}

Lyrics::Server::Replacements Lyrics::Genius::replacements() const
{
	return Lyrics::Server::Replacements
	{
		{"  ", " "},
		{" ", "-"},
		{"(", ""},
		{")", ""},
		{",", "-"},
		{"&", "-and-"},
		{"/", "-"},
		{"'", " "},
		{"--", "-"}
	};
}

QString Lyrics::Genius::call_policy() const
{
	return "<SERVER>/<ARTIST>-<TITLE>-lyrics";
}

Lyrics::Server::StartEndTags Lyrics::Genius::start_end_tag() const
{
	return Lyrics::Server::StartEndTags
	{
		{"<div class=\"lyrics\">", "</div>"}
	};
}

bool Lyrics::Genius::is_start_tag_included() const
{
	return false;
}

bool Lyrics::Genius::is_end_tag_included() const
{
	return false;
}

bool Lyrics::Genius::is_numeric() const
{
	return false;
}

bool Lyrics::Genius::is_lowercase() const
{
	return true;
}

QString Lyrics::Genius::error_string() const
{
	return QString("Page not found");
}
