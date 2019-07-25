#include "AmazonCoverFetcher.h"
#include "Utils/Logger/Logger.h"
#include <QString>
#include <QUrl>
#include <QRegExp>
#include <QMap>

bool Cover::Fetcher::Amazon::can_fetch_cover_directly() const
{
	return false;
}

QStringList Cover::Fetcher::Amazon::calc_addresses_from_website(const QByteArray& website) const
{
	QRegExp re("<img.*class=\"s-image\".*srcset=\"(.+[0-9]+x)\"");
	re.setMinimal(true);
	int idx = re.indexIn(website);
	if(idx < 0){
		return QStringList();
	}

	sp_log(Log::Info, this) << re.cap(1);

	QStringList sources;
	QMap<QString, double> item_sources;

	int offset = 0;
	while(idx > 0)
	{
		QString caption = re.cap(1);
		QRegExp item_re("(http[s]*://\\S+\\.jpg)\\s([0-9+](\\.[0-9]+)*)x");
		int item_idx = item_re.indexIn(website, offset);
		int item_offset = 0;
		while(item_idx >= 0)
		{
			QString item_caption = item_re.cap(1);
			QString val = item_re.cap(2);

			item_sources.insert(item_caption, val.toDouble());
			item_idx = item_re.indexIn(caption, item_offset);
			item_offset = item_idx + item_caption.size();
		}

		double max_val=0;
		QString max_str;
		for(auto it=item_sources.begin(); it!=item_sources.end(); it++)
		{
			if(it.value() > max_val)
			{
				max_str = it.key();
				max_val = it.value();
			}
		}

		sources << max_str;

		idx = re.indexIn(website, offset);
		offset = idx + caption.size();
	}

	sources.removeDuplicates();

	return sources;
}

QString Cover::Fetcher::Amazon::identifier() const
{
	return "Amazon";
}

QString Cover::Fetcher::Amazon::artist_address(const QString& artist) const
{
	Q_UNUSED(artist)
	return QString();
}

QString Cover::Fetcher::Amazon::album_address(const QString& artist, const QString& album) const
{
	QString str(artist + "+" + album);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(str));

	return QString("https://www.amazon.com/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
	//return QString("https://www.amazon.com/s?k=%1&i=music-intl-ship&ref=nb_sb_noss_1").arg(str);
}

QString Cover::Fetcher::Amazon::search_address(const QString& search_string) const
{
	QString str(search_string);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(search_string));

	return QString("https://www.amazon.com/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

bool Cover::Fetcher::Amazon::is_search_supported() const
{
	return true;
}

bool Cover::Fetcher::Amazon::is_album_supported() const
{
	return true;
}

bool Cover::Fetcher::Amazon::is_artist_supported() const
{
	return false;
}

int Cover::Fetcher::Amazon::estimated_size() const
{
	return 400;
}
