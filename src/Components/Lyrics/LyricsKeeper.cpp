#include "LyricsKeeper.h"

QString Lyrics::LyricsKeeper::name() const
{
	return "Lyrics Keeper";
}

QString Lyrics::LyricsKeeper::address() const
{
	return "http://lyrics-keeper.com";
}

QMap<QString, QString> Lyrics::LyricsKeeper::replacements() const
{
	return QMap<QString, QString>
	{
		{"&", ""},
		{" ", "-"},
		{"'", "-"},
		{".", "-"},
		{"--", "-"}
	};
}

QString Lyrics::LyricsKeeper::call_policy() const
{
	return "<SERVER>/en/<ARTIST>/<TITLE>.html";
}

QMap<QString, QString> Lyrics::LyricsKeeper::start_end_tag() const
{
	return QMap<QString, QString>
	{
		{"<div id=\"lyrics\">", "</div>"}
	};
}

bool Lyrics::LyricsKeeper::is_start_tag_included() const
{
	return false;
}

bool Lyrics::LyricsKeeper::is_end_tag_included() const
{
	return false;
}

bool Lyrics::LyricsKeeper::is_numeric() const
{
	return false;
}

bool Lyrics::LyricsKeeper::is_lowercase() const
{
	return true;
}

QString Lyrics::LyricsKeeper::error_string() const
{
	return QString("page cannot be found");
}
