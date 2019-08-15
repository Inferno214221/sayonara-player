/* AllMusicCoverFetcher.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Allmusic.h"
#include "Utils/Logger/Logger.h"
#include <QString>
#include <QUrl>
#include <QRegExp>

using Cover::Fetcher::Allmusic;

bool Allmusic::can_fetch_cover_directly() const
{
	return false;
}

QStringList Allmusic::parse_addresses(const QByteArray& website) const
{
	QStringList ret;

	QRegExp re("<img.*lazy.*src=\"(https?://.+\\.[a-z]{3}(.+f=.)?)\"");
	re.setMinimal(true);
	int idx = re.indexIn(website);

	while(idx > 0)
	{
		QString link = re.cap(1);

		if(link.contains("f="))
		{
			link = link.left(link.size() - 3);

			ret << link + "f=5";
			ret << link + "f=4";
		}

		else {
			ret << link;
		}

		ret.removeDuplicates();

		if(ret.size() > 5)
		{
			break;
		}

		idx = re.indexIn(website, idx + link.size());
	}

	return ret;
}

QString Allmusic::priv_identifier() const
{
	return "allmusic";
}

QString Allmusic::artist_address(const QString& artist) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist));
	return QString("https://www.allmusic.com/search/artists/%1").arg(str);
}

QString Allmusic::album_address(const QString& artist, const QString& album) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(artist + " " + album));
	return QString("https://www.allmusic.com/search/albums/%1").arg(str);
}

QString Allmusic::search_address(const QString& searchstring) const
{
	QString str = QString::fromLocal8Bit(QUrl::toPercentEncoding(searchstring));
	return QString("https://www.allmusic.com/search/all/%1").arg(str);
}

int Allmusic::estimated_size() const
{
	return 500;
}
