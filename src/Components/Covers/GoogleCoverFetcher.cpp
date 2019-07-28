/* GoogleCoverFetcher.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#include "GoogleCoverFetcher.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QRegExp>
#include <QString>
#include <QUrl>

using namespace Cover::Fetcher;

bool Google::can_fetch_cover_directly() const
{
	return false;
}

QStringList Google::calc_addresses_from_website(const QByteArray& website) const
{
	QString regex = "(https://encrypted-tbn.+)\"";
	QStringList addresses;

	if (website.isEmpty()) {
		sp_log(Log::Error, this) << "Website empty";
		return addresses;
	}

	int idx=500;

	QString website_str = QString::fromLocal8Bit(website);

	while(true)
	{
		QRegExp re(regex);
		re.setMinimal(true);
		idx = re.indexIn(website_str, idx);

		if(idx == -1) {
			break;
		}

		QString str = re.cap(0);

		idx += str.length();
		str.remove("\"");
		addresses << str;
	}

	sp_log(Log::Debug, this) << "Got " << addresses.size() << " Addresses";

	return addresses;
}


QString Google::artist_address(const QString& artist) const
{
	return search_address(QUrl::toPercentEncoding(artist));
}

QString Google::album_address(const QString& artist, const QString& album) const
{
	QString new_album, searchstring;
	QRegExp regex;


	if(searchstring.compare("various", Qt::CaseInsensitive) != 0){
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

	if(searchstring.size() > 0) {
		searchstring += "+";
	}

	searchstring += new_album;

	return search_address(searchstring);
}

QString Google::search_address(const QString& str) const
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

bool Google::is_search_supported() const
{
	return true;
}

bool Google::is_album_supported() const
{
	return true;
}

bool Google::is_artist_supported() const
{
	return true;
}


int Google::estimated_size() const
{
	return 150;
}


QString Google::priv_identifier() const
{
	// DO NOT EDIT THAT! It's also used in CoverFetchManager
	return "google";
}
