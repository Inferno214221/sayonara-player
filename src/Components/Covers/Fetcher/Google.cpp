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
	QString regex = "(https://encrypted-tbn.+)\"";
	QStringList addresses;

	if(website.isEmpty())
	{
		return addresses;
	}

	int idx = 500;

	QString website_str = QString::fromLocal8Bit(website);

	while(true)
	{
		QRegExp re(regex);
		re.setMinimal(true);
		idx = re.indexIn(website_str, idx);

		if(idx == -1)
		{
			break;
		}

		QString str = re.cap(0);

		idx += str.length();
		str.remove("\"");
		addresses << str;
	}

	return addresses;
}

QString Google::artistAddress(const QString& artist) const
{
	return fulltextSearchAddress(QUrl::toPercentEncoding(artist));
}

QString Google::albumAddress(const QString& artist, const QString& album) const
{
	QString new_album, searchstring;
	QRegExp regex;

	if(searchstring.compare("various", Qt::CaseInsensitive) != 0)
	{
		searchstring = QUrl::toPercentEncoding(artist);
	}

	new_album = album;

	regex = QRegExp(QString("(\\s)?-?(\\s)?((cd)|(CD)|((d|D)((is)|(IS))(c|C|k|K)))(\\d|(\\s\\d))"));

	new_album = new_album.toLower();
	new_album = new_album.remove(regex);
	new_album = new_album.replace("()", "");
	new_album = new_album.replace("( )", "");
	new_album = new_album.trimmed();
	new_album = QUrl::toPercentEncoding(album);

	if(searchstring.size() > 0)
	{
		searchstring += "+";
	}

	searchstring += new_album;

	return fulltextSearchAddress(searchstring);
}

QString Google::fulltextSearchAddress(const QString& str) const
{
	QString searchstring = str;
	searchstring.replace(" ", "%20");
	searchstring.replace("/", "%2F");
	searchstring.replace("&", "%26");
	searchstring.replace("$", "%24");

	QString url = QString("https://www.google.de/search?num=20&hl=de&site=imghp&tbm=isch&source=hp");

	url += QString("&q=") + searchstring;
	url += QString("&oq=") + searchstring;

	return url;
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
