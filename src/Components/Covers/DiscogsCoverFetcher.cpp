/* DiscogsCoverFetcher.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "DiscogsCoverFetcher.h"
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include "Helper/Logger/Logger.h"

static QString get_basic_url(const QString& str)
{
	QString str2 = str;
	str2 = str2.replace(" ", "+");

	QString domain = "www.discogs.com";

	return "https://" + domain + "/search/?q=" +
			QUrl::toPercentEncoding(str2);
}

bool DiscogsCoverFetcher::can_fetch_cover_directly() const
{
	return false;
}

QStringList DiscogsCoverFetcher::calc_addresses_from_website(const QByteArray& website) const
{
	QStringList ret;

	QRegExp re("class=\"thumbnail_center\">\\s*<img\\s*data-src\\s*=\\s*\"(.+)\"");
	re.setMinimal(true);
	QString website_str = QString::fromLocal8Bit(website);
	int idx = re.indexIn(website_str);
	while(idx > 0){
		ret << re.cap(1);
		website_str.remove(0, idx + 5);
		idx = re.indexIn(website_str);
	}

	return ret;
}

QString DiscogsCoverFetcher::get_artist_address(const QString& artist) const
{

	return get_basic_url(artist) + "&type=artist";
}

QString DiscogsCoverFetcher::get_album_address(const QString& artist, const QString& album) const
{
	return get_basic_url(artist + "+" + album) + "&type=all";
}

QString DiscogsCoverFetcher::get_search_address(const QString& str) const
{
	return get_basic_url(str) + "&type=all";
}

bool DiscogsCoverFetcher::is_search_supported() const
{
	return true;
}

bool DiscogsCoverFetcher::is_album_supported() const
{
	return true;
}

bool DiscogsCoverFetcher::is_artist_supported() const
{
	return true;
}

int DiscogsCoverFetcher::get_estimated_size() const
{
	return 350;
}

QString DiscogsCoverFetcher::get_keyword() const
{
	return "discogs";
}