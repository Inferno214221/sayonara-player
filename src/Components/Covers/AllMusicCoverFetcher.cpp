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
	QRegExp re_links("<img.*src\"(http.*://.*)\"");

	while(true)
	{
		int i = re_links.indexIn(website);
		if(i < 0){
			break;
		}

		sp_log(Log::Info, this) << re_links.cap(1);
	}

	QRegExp re("<img.*src=\"(http.*://.*f=.)\"");
	re.setMinimal(true);
	int idx = re.indexIn(website);
	if(idx < 0){
		return QStringList();
	}

	QString link = re.cap(1);

	QStringList ret;
	ret << link;
	link.remove(link.size() - 3, 3);

	for(int i=3; i<5; i++)
	{
		ret << link + "f=" + QString::number(i);
	}

	return ret;
}

QString Cover::Fetcher::AllMusicCoverFetcher::keyword() const
{
	return "Allmusic";
}

QString Cover::Fetcher::AllMusicCoverFetcher::artist_address(const QString& artist) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist));
	return QString("https://www.allmusic.com/search/all/%1").arg(str);
}

QString Cover::Fetcher::AllMusicCoverFetcher::album_address(const QString& artist, const QString& album) const
{
	return QString();
}

QString Cover::Fetcher::AllMusicCoverFetcher::search_address(const QString& str) const
{
	return QString();
}

bool Cover::Fetcher::AllMusicCoverFetcher::is_search_supported() const
{
	return false;
}

bool Cover::Fetcher::AllMusicCoverFetcher::is_album_supported() const
{
	return false;
}

bool Cover::Fetcher::AllMusicCoverFetcher::is_artist_supported() const
{
	return true;
}

int Cover::Fetcher::AllMusicCoverFetcher::estimated_size() const
{
	return 500;
}
