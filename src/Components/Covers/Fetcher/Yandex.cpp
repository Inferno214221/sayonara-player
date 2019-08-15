#include "Yandex.h"

#include <QByteArray>
#include <QRegExp>
#include <QUrl>
#include <QStringList>
#include <QString>

using Cover::Fetcher::Yandex;

QString Yandex::priv_identifier() const
{
	return "yandex";
}

bool Yandex::can_fetch_cover_directly() const
{
	return false;
}

QStringList Yandex::parse_addresses(const QByteArray& website) const
{
	QRegExp re("<img.+class=\"serp-item__thumb.+src=\"(.+)\"");
	re.setMinimal(true);
	int idx = re.indexIn(website);

	if(idx < 0){
		return QStringList();
	}

	QStringList ret;
	while(idx >= 0)
	{
		QString url = "https:" + re.cap(1);
		url.replace("&amp;", "&");
		ret << url;
		idx += re.cap(1).size();

		idx = re.indexIn(website, idx);
	}

	return ret;
}


QString Yandex::artist_address(const QString& artist) const
{
	return search_address(artist);
}

QString Yandex::album_address(const QString& artist, const QString& album) const
{
	return search_address(album + " " + artist);
}

QString Yandex::search_address(const QString& str) const
{
	QString pe = QUrl::toPercentEncoding(str);
	return QString("https://yandex.com/images/search?text=%1&iorient=square&from=tabbar").arg(pe);
}

int Yandex::estimated_size() const
{
	return 300;
}

