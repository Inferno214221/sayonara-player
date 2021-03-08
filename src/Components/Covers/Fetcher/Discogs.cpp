/* DiscogsCoverFetcher.cpp */

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

#include "Discogs.h"

#include <QRegExp>
#include <QStringList>
#include <QUrl>

using namespace Cover::Fetcher;

namespace
{
	QString basicUrl(const QString& str)
	{
		auto stringCopy = str;
		stringCopy = stringCopy.replace(" ", "+");

		return QString("https://%1/search/?q=%2")
			.arg("www.discogs.com")
			.arg(QString(QUrl::toPercentEncoding(stringCopy)));
	}
}

bool Discogs::canFetchCoverDirectly() const
{
	return false;
}

QStringList Discogs::parseAddresses(const QByteArray& website) const
{
	QStringList ret;

	auto regExp = QRegExp("class=\"thumbnail_center\">\\s*<img\\s*data-src\\s*=\\s*\"(.+)\"");
	regExp.setMinimal(true);

	const auto websiteData = QString::fromLocal8Bit(website);
	auto idx = regExp.indexIn(websiteData);
	while(idx > 0)
	{
		const auto caption = regExp.cap(1);
		ret << caption;
		idx = regExp.indexIn(websiteData, idx + caption.size());
	}

	return ret;
}

QString Discogs::artistAddress(const QString& artist) const
{
	return basicUrl(artist) + "&type=artist";
}

QString Discogs::albumAddress(const QString& artist, const QString& album) const
{
	return basicUrl(artist + "+" + album) + "&type=all";
}

QString Discogs::fulltextSearchAddress(const QString& str) const
{
	return basicUrl(str) + "&type=all";
}

int Discogs::estimatedSize() const
{
	return 350;
}

QString Discogs::privateIdentifier() const
{
	return "discogs";
}
