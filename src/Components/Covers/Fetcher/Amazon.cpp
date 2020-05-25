/* AmazonCoverFetcher.cpp */

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

#include "Amazon.h"
#include "Utils/Logger/Logger.h"

#include <QString>
#include <QUrl>
#include <QRegExp>
#include <QMap>

using Cover::Fetcher::Amazon;

bool Amazon::canFetchCoverDirectly() const
{
	return false;
}

QStringList Amazon::parseAddresses(const QByteArray& website) const
{
	QRegExp re("<img.*class=\"s-image\".*srcset=\"(.+[0-9]+x)\"");
	re.setMinimal(true);
	int index = re.indexIn(website);
	if(index < 0){
		return QStringList();
	}

	spLog(Log::Info, this) << re.cap(1);

	QStringList sources;
	QMap<QString, double> itemSources;

	int offset = 0;
	while(index > 0)
	{
		const QString caption = re.cap(1);
		const QRegExp itemRegExp("(http[s]*://\\S+\\.jpg)\\s([0-9+](\\.[0-9]+)*)x");

		int itemIndex = itemRegExp.indexIn(website, offset);
		int itemOffset = 0;
		while(itemIndex >= 0)
		{
			QString item_caption = itemRegExp.cap(1);
			QString val = itemRegExp.cap(2);

			itemSources.insert(item_caption, val.toDouble());
			itemIndex = itemRegExp.indexIn(caption, itemOffset);
			itemOffset = itemIndex + item_caption.size();
		}

		double maxVal=0;
		QString maxStr;
		for(auto it=itemSources.begin(); it != itemSources.end(); it++)
		{
			if(it.value() > maxVal)
			{
				maxStr = it.key();
				maxVal = it.value();
			}
		}

		sources << maxStr;

		index = re.indexIn(website, offset);
		offset = index + caption.size();
	}

	sources.removeDuplicates();

	return sources;
}

QString Amazon::privateIdentifier() const
{
	return "amazon";
}

QString Amazon::albumAddress(const QString& artist, const QString& album) const
{
	QString str(artist + "+" + album);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(str));

	return QString("https://www.amazon.de/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

QString Amazon::fulltextSearchAddress(const QString& search_string) const
{
	QString str(search_string);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(search_string));

	return QString("https://www.amazon.de/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

int Amazon::estimatedSize() const
{
	return 400;
}
