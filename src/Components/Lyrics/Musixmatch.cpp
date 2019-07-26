#include "Musixmatch.h"
#include "Utils/Logger/Logger.h"

#include <QUrl>

QString Lyrics::Musixmatch::name() const
{
	return "Musixmatch";
}

Lyrics::Server::StartEndTags Lyrics::Musixmatch::start_end_tag() const
{
	return Lyrics::Server::StartEndTags
	{
		{"<div class=\"empty-message\">", "</div>"},
		{"<div id=\"selectable-lyrics\"", "</span><span data-reactid"},
		{"<p class=.*content", "</p>"},
		{"\"body\":\"", "\",\""}
	};
}

bool Lyrics::Musixmatch::is_start_tag_included() const
{
	return false;
}

bool Lyrics::Musixmatch::is_end_tag_included() const
{
	return false;
}

bool Lyrics::Musixmatch::is_numeric() const
{
	return false;
}

bool Lyrics::Musixmatch::is_lowercase() const
{
	return false;
}

QString Lyrics::Musixmatch::error_string() const
{
	return QString("404 Not Found");
}


bool Lyrics::Musixmatch::can_fetch_directly() const
{
	return false;
}

QString Lyrics::Musixmatch::search_address(QString artist, QString title) const
{
	artist = apply_replacements(artist);
	title = apply_replacements(title);

	return
		QString("https://www.musixmatch.com/search/%1 %2#")
			.arg(artist)
			.arg(title);
}

QString Lyrics::Musixmatch::parse_search_result(const QString& search_result)
{
	QRegExp re("href=\"/(lyrics/.+)\"");
	re.setMinimal(true);

	int idx = re.indexIn(search_result);
	if(idx < 0){
		return QString();
	}

	QString part_url = re.cap(1);

	return "https://www.musixmatch.com/" + part_url;
}


Lyrics::Server::Replacements Lyrics::Musixmatch::replacements() const
{
	return Lyrics::Server::Replacements
	{
		{"  ", " "},
		{"?", " "},
		{"=", " "},
		{"&", " "},
		{"/", " "},
		{"(", ""},
		{")", ""},
		{"\"", ""},
		{"[", ""},
		{"]", ""},
		{"{", ""},
		{"}", ""}
	};
}
