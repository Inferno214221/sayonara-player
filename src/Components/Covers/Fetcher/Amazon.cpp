/* AmazonCoverFetcher.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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

#include "Amazon.h"
#include "Utils/Logger/Logger.h"

#include <QString>
#include <QUrl>
#include <QRegExp>
#include <QMap>

using Cover::Fetcher::Amazon;

bool Amazon::can_fetch_cover_directly() const
{
	return false;
}

QStringList Amazon::parse_addresses(const QByteArray& website) const
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

QString Amazon::priv_identifier() const
{
	return "amazon";
}

QString Amazon::album_address(const QString& artist, const QString& album) const
{
	QString str(artist + "+" + album);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(str));

	return QString("https://www.amazon.de/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

QString Amazon::search_address(const QString& search_string) const
{
	QString str(search_string);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(search_string));

	return QString("https://www.amazon.de/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

int Amazon::estimated_size() const
{
	return 400;
}
