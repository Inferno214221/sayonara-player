/* GoogleCoverFetcher.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "Google.h"

#include <QStringList>
#include <QRegExp>
#include <QString>
#include <QUrl>

using namespace Cover::Fetcher;

bool Google::canFetchCoverDirectly() const
{
	return false;
}

QStringList Google::parseAddresses(const QByteArray& website) const
{
	if(website.isEmpty())
	{
		return QStringList {};
	}

	const auto websiteData = QString::fromLocal8Bit(website);

	auto re = QRegExp("(https://encrypted-tbn.+)\"");
	re.setMinimal(true);

	QStringList addresses;

	auto idx = re.indexIn(websiteData, 500);
	while(idx >= 0)
	{
		auto caption = re.cap(0);
		caption.remove("\"");
		addresses << caption;

		idx = re.indexIn(websiteData, idx + caption.length());
	}

	return addresses;
}

QString Google::artistAddress(const QString& artist) const
{
	return fulltextSearchAddress(QUrl::toPercentEncoding(artist));
}

QString Google::albumAddress(const QString& artist, const QString& album) const
{
	const auto regex = QRegExp(QString("(\\s)?-?(\\s)?((cd)|(CD)|((d|D)((is)|(IS))(c|C|k|K)))(\\d|(\\s\\d))"));

	auto albumCopy = album;
	albumCopy = albumCopy.toLower();
	albumCopy = albumCopy.remove(regex);
	albumCopy = albumCopy.replace("()", "");
	albumCopy = albumCopy.replace("( )", "");
	albumCopy = albumCopy.trimmed();
	albumCopy = QUrl::toPercentEncoding(album);

	auto searchString = (artist.compare("various", Qt::CaseInsensitive) != 0)
	                     ? QUrl::toPercentEncoding(artist) + "+" + albumCopy
	                     : albumCopy;
	
	return fulltextSearchAddress(searchString);
}

QString Google::fulltextSearchAddress(const QString& str) const
{
	auto searchString = str;
	searchString.replace(" ", "%20");
	searchString.replace("/", "%2F");
	searchString.replace("&", "%26");
	searchString.replace("$", "%24");

	return QString("https://www.google.de/search?num=20&hl=de&site=imghp&tbm=isch&source=hp")
	       + QString("&q=%1").arg(searchString)
	       + QString("&oq=%1").arg(searchString);
}

QString Google::radioSearchAddress(const QString& stationName, const QString& stationUrl) const
{
	const auto searchString = searchStringFromRadioStation(stationName, stationUrl);
	return (searchString.isEmpty())
		? QString()
		: fulltextSearchAddress(searchString);
}

int Google::estimatedSize() const
{
	return 150;
}

QString Google::privateIdentifier() const
{
	// DO NOT EDIT THAT! It's also used in CoverFetchManager
	return "google";
}
