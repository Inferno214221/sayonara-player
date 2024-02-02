/* Audioscrobbler.cpp */

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

#include "Audioscrobbler.h"
#include "Utils/Algorithm.h"
#include "Components/Streaming/LastFM/LFMGlobals.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QStringList>
#include <QUrl>

using Cover::Fetcher::Audioscrobbler;

namespace
{
	QStringList mapToStringList(const QMap<QString, QString>& map)
	{
		auto result = QStringList {};
		const auto sizes = std::array {"mega", "extralarge", "large", "medium", "small"};
		for(const auto size: sizes)
		{
			result.push_back(map[size]);
		}

		result.removeAll({});
		result.removeDuplicates();

		return result;
	}

	QStringList parseImageTag(const QJsonObject& artistOrAlbum)
	{
		auto map = QMap<QString, QString> {};

		if(const auto imageTag = artistOrAlbum["image"]; imageTag.isArray())
		{
			const auto images = imageTag.toArray();
			for(const auto& image: images)
			{
				const auto url = image["#text"].toString();
				const auto size = image["size"].toString();
				map[size] = url;
			}
		}

		return mapToStringList(map);
	}

	QString constructWebsite(const std::map<QString, QString>& params)
	{
		auto lst = QStringList {};
		for(const auto& [key, value]: params)
		{
			lst.push_back(QString("%1=%2").arg(key).arg(value));
		}

		return QString(LastFM::BaseUrl) + "?" + lst.join("&");
	}
}

bool Audioscrobbler::canFetchCoverDirectly() const { return false; }

QStringList Audioscrobbler::parseAddresses(const QByteArray& data) const
{
	auto document = QJsonDocument::fromJson(data);
	if(const auto album = document["album"]; album.isObject())
	{
		return parseImageTag(album.toObject());
	}

	if(const auto artist = document["artist"]; artist.isObject())
	{
		return {"https://www.last.fm/music/Metallica/+images"};
	}

	return {};
}

QString Audioscrobbler::albumAddress(const QString& artist, const QString& album) const
{
	const auto params = std::map<QString, QString> {
		{"method",  "album.getinfo"},
		{"album",   QUrl::toPercentEncoding(album)},
		{"artist",  QUrl::toPercentEncoding(artist)},
		{"format",  "json"},
		{"api_key", LastFM::ApiKey}};

	return constructWebsite(params);
}

QString Cover::Fetcher::Audioscrobbler::artistAddress(const QString& /*artist*/) const
{
	// lastfm only sends white star icon for artists because of copyright
	return {};
}

int Audioscrobbler::estimatedSize() const { return 300; }

QString Audioscrobbler::privateIdentifier() const { return "audioscrobbler"; }

