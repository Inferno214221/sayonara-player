#include "AllMusicCoverFetcher.h"
#include "Utils/Logger/Logger.h"
#include <QString>
#include <QUrl>
#include <QRegExp>

bool Cover::Fetcher::AllMusicCoverFetcher::can_fetch_cover_directly() const
{
	return false;
}

QStringList Cover::Fetcher::AllMusicCoverFetcher::calc_addresses_from_website(const QByteArray& website) const
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

QString Cover::Fetcher::AllMusicCoverFetcher::identifier() const
{
	return "Allmusic";
}

QString Cover::Fetcher::AllMusicCoverFetcher::artist_address(const QString& artist) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist));
	return QString("https://www.allmusic.com/search/artists/%1").arg(str);
}

QString Cover::Fetcher::AllMusicCoverFetcher::album_address(const QString& artist, const QString& album) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist + " " + album));
	return QString("https://www.allmusic.com/search/albums/%1").arg(str);
}

QString Cover::Fetcher::AllMusicCoverFetcher::search_address(const QString& searchstring) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(searchstring));
	return QString("https://www.allmusic.com/search/all/%1").arg(str);
}

bool Cover::Fetcher::AllMusicCoverFetcher::is_search_supported() const
{
	return true;
}

bool Cover::Fetcher::AllMusicCoverFetcher::is_album_supported() const
{
	return true;
}

bool Cover::Fetcher::AllMusicCoverFetcher::is_artist_supported() const
{
	return true;
}

int Cover::Fetcher::AllMusicCoverFetcher::estimated_size() const
{
	return 500;
}
