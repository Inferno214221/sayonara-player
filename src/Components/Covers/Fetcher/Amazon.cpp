/* AmazonCoverFetcher.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
	QRegExp re("<img.*class=\"s-image\".*src(set)?=\"(.+[0-9]+x)\"");
	re.setMinimal(true);
	auto index = re.indexIn(website);
	if(index < 0)
	{
		return QStringList();
	}

	QStringList sources;
	QMap<QString, double> itemSources;

	auto offset = 0;
	while(index > 0)
	{
		const auto caption = re.cap(2);
		const auto itemRegExp = QRegExp("(http[s]*://\\S+\\.jpg)\\s([0-9+](\\.[0-9]+)*)x");

		auto itemIndex = itemRegExp.indexIn(website, offset);
		auto itemOffset = 0;
		while(itemIndex >= 0)
		{
			const auto itemCaption = itemRegExp.cap(1);
			const auto val = itemRegExp.cap(2);

			itemSources.insert(itemCaption, val.toDouble());
			itemIndex = itemRegExp.indexIn(caption, itemOffset);
			itemOffset = itemIndex + itemCaption.size();
		}

		double maxVal = 0;
		QString maxStr;
		for(auto it = itemSources.begin(); it != itemSources.end(); it++)
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
	auto str = QString(artist + "+" + album);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(str));

	return QString("https://www.amazon.de/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

QString Amazon::fulltextSearchAddress(const QString& searchString) const
{
	auto str = QString(searchString);
	str.replace(" ", "+");
	str = QString::fromLocal8Bit(QUrl::toPercentEncoding(searchString));

	return QString("https://www.amazon.de/s?k=%1&i=digital-music&ref=nb_sb_noss").arg(str);
}

int Amazon::estimatedSize() const
{
	return 400;
}
