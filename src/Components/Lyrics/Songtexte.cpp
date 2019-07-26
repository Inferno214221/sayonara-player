#include "Songtexte.h"
#include "Utils/Logger/Logger.h"

QString Lyrics::Songtexte::name() const
{
	return "Songtexte.com";
}

Lyrics::Server::StartEndTags Lyrics::Songtexte::start_end_tag() const
{
	return Lyrics::Server::StartEndTags
	{
		{"class=\"blockedArtist\">", "</p>"},
		{"<div id=\"lyrics\">", "</div> <div"}
	};
}

bool Lyrics::Songtexte::is_start_tag_included() const
{
	return false;
}

bool Lyrics::Songtexte::is_end_tag_included() const
{
	return false;
}

bool Lyrics::Songtexte::is_numeric() const
{
	return false;
}

bool Lyrics::Songtexte::is_lowercase() const
{
	return true;
}

QString Lyrics::Songtexte::error_string() const
{
	return "404";
}

bool Lyrics::Songtexte::can_fetch_directly() const
{
	return false;
}

QString Lyrics::Songtexte::search_address(QString artist, QString title) const
{
	artist = apply_replacements(artist);
	title = apply_replacements(title);

	return QString("https://www.songtexte.com/search?q=%1+%2&c=all").arg(title).arg(artist);
}

QString Lyrics::Songtexte::parse_search_result(const QString& search_result)
{
	QRegExp re("href=\"(songtext.+\\.html)\"");
	re.setMinimal(true);

	int idx = re.indexIn(search_result);
	if(idx < 0){
		return QString();
	}

	QString part_url = re.cap(1);

	return "https://www.songtexte.com/" + part_url;
}


Lyrics::Server::Replacements Lyrics::Songtexte::replacements() const
{
	return Lyrics::Server::Replacements
	{
		{"  ", " "},
		{" ", "+"},
		{"?", "+"},
		{"=", "+"},
		{"&", "+"},
		{"/", "+"},
		{"(", ""},
		{")", ""},
		{"\"", ""},
		{"[", ""},
		{"]", ""},
		{"{", ""},
		{"}", ""}
	};
}
