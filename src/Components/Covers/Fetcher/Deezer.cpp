/* Deezer.cpp, (Created on 19.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#include "Deezer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>
#include <QUrl>

using namespace Cover::Fetcher;

namespace
{
	QString calcUrl(const QString& str)
	{
		return QString("https://www.deezer.com/search/%1")
			.arg(QString(QUrl::toPercentEncoding(str)));
	}

	QByteArray extractJson(const QByteArray& website)
	{
		const auto delimStart = QByteArray("<script>window.__DZR_APP_STATE__ = ");
		const auto delimEnd = QByteArray("</script>");

		const auto index = website.indexOf(delimStart);
		const auto start = index + delimStart.size();

		return website.mid(start, website.indexOf(delimEnd, index) - start);
	}

	QStringList parseImageFields(const QJsonArray& arr, const QStringList& fields)
	{
		auto ret = QStringList {};
		for(const auto& item: arr)
		{
			for(const auto& field: fields)
			{
				const auto picture = item[field].toString();
				if(!picture.isEmpty())
				{
					ret << QString("https://e-cdns-images.dzcdn.net/images/cover/%1/800x800-000000-80-0-0.jpg")
						.arg(picture);
				}
			}
		}

		return ret;
	}

	QStringList parseAlbum(const QJsonDocument& document)
	{
		const auto album = document["ALBUM"].toObject();
		return parseImageFields(album["data"].toArray(), {"ALB_PICTURE", "ART_PICTURE"});
	}

	QStringList parseTracks(const QJsonDocument& document)
	{
		const auto tracks = document["TRACK"].toObject();
		return parseImageFields(tracks["data"].toArray(), {"ALB_PICTURE"});
	}
}

QStringList Deezer::parseAddresses(const QByteArray& website) const
{
	const auto json = extractJson(website);

	const auto document = QJsonDocument::fromJson(json);

	auto result = QStringList {} << parseAlbum(document) << parseTracks(document);
	result.removeDuplicates();

	return result;
}

QString Deezer::artistAddress(const QString& artist) const { return calcUrl(artist); }

QString Deezer::albumAddress(const QString& artist, const QString& album) const
{
	return calcUrl(artist + " " + album);
}

QString Deezer::fulltextSearchAddress(const QString& str) const { return calcUrl(str); }

int Deezer::estimatedSize() const { return 800; }

QString Deezer::privateIdentifier() const { return "Deezer"; }

bool Deezer::canFetchCoverDirectly() const { return false; }
