#include "Musixmatch.h"
#include "Utils/Logger/Logger.h"

QString Lyrics::Musixmatch::name() const
{
	return "Musixmatch";
}

QMap<QString, QString> Lyrics::Musixmatch::start_end_tag() const
{
	return QMap<QString, QString>
	{
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

#include <QUrl>
QString Lyrics::Musixmatch::search_address(QString artist, QString title) const
{
	return
		QString("https://www.musixmatch.com/search/") +
		QUrl::toPercentEncoding(artist) +
		"%20" +
		QUrl::toPercentEncoding(title) +
		"#";
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
