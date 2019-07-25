#include "Wikia.h"

QString Lyrics::Wikia::name() const
{
	return "Wikia";
}

QString Lyrics::Wikia::address() const
{
	return "http://lyrics.Wikia.com";
}

QMap<QString, QString> Lyrics::Wikia::replacements() const
{
	return QMap<QString, QString>
	{
		{"'", ""},
		{"ö", "o"},
		{" ", "_"},
		{"!", ""},
		{"&", "%26"}
	};
}

QString Lyrics::Wikia::call_policy() const
{
	return "<SERVER>/wiki/<ARTIST>:<TITLE>";
}

QMap<QString, QString> Lyrics::Wikia::start_end_tag() const
{
	return QMap<QString, QString>
	{
		{"<div class='lyricbox'>", "<!--"}
	};
}

bool Lyrics::Wikia::is_start_tag_included() const
{
	return false;
}

bool Lyrics::Wikia::is_end_tag_included() const
{
	return false;
}

bool Lyrics::Wikia::is_numeric() const
{
	return true;
}

bool Lyrics::Wikia::is_lowercase() const
{
	return false;
}

QString Lyrics::Wikia::error_string() const
{
	return QString("this page needs content");
}
