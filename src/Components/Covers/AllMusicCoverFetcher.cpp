#include "AllMusicCoverFetcher.h"
#include "Utils/Logger/Logger.h"
#include <QString>
#include <QUrl>
#include <QRegExp>

bool Cover::Fetcher::AllMusic::can_fetch_cover_directly() const
{
	return false;
}

QStringList Cover::Fetcher::AllMusic::parse_addresses(const QByteArray& website) const
{
	QRegExp re("<img.*src=\"(http.*://.*f=.)\"");
	re.setMinimal(true);

	int idx = re.indexIn(website);
	if(idx < 0){
		return QStringList();
	}

	QString link = re.cap(1);
	link = link.left(link.size() - 3);

	QStringList ret
	{
		link + "f=5",
		link + "f=4",
		link + "f=3"
	};

	return ret;
}

QString Cover::Fetcher::AllMusic::priv_identifier() const
{
	return "allmusic";
}

QString Cover::Fetcher::AllMusic::artist_address(const QString& artist) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist));
	return QString("https://www.allmusic.com/search/artists/%1").arg(str);
}

QString Cover::Fetcher::AllMusic::album_address(const QString& artist, const QString& album) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist + " " + album));
	return QString("https://www.allmusic.com/search/albums/%1").arg(str);
}

QString Cover::Fetcher::AllMusic::search_address(const QString& searchstring) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(searchstring));
	return QString("https://www.allmusic.com/search/all/%1").arg(str);
}

int Cover::Fetcher::AllMusic::estimated_size() const
{
	return 500;
}
