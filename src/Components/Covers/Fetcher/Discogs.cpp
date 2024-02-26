/* DiscogsCoverFetcher.cpp */

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

#include "Discogs.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QRegExp>
#include <QStringList>
#include <QUrl>

using namespace Cover::Fetcher;

namespace
{
	QString basicUrl(const std::map<QString, QString>& params)
	{
		constexpr const auto* baseUrl = "https://api.discogs.com/database/search?";

		auto paramStrings = QStringList {"token=BuBsUyZsFtoaEGmquLQIbfXduYbgooGThIpRivUe"};
		for(const auto& [key, value]: params)
		{
			paramStrings << QString("%1=%2")
				.arg(key)
				.arg(QString(QUrl::toPercentEncoding(value)));
		}

		return baseUrl + paramStrings.join("&");
	}
}

bool Discogs::canFetchCoverDirectly() const { return false; }

QStringList Discogs::parseAddresses(const QByteArray& website) const
{
	auto ret = QStringList {};

	const auto doc = QJsonDocument::fromJson(website);
	const auto results = doc["results"].toArray();

	for(const auto& result: results)
	{
		ret << result["cover_image"].toString();
	}

	return ret;
}

QString Discogs::artistAddress(const QString& artist) const
{
	return basicUrl(
		{
			{"artist", artist},
			{"type",   "artist"}
		});
}

QString Discogs::fulltextSearchAddress(const QString& str) const
{
	return basicUrl(
		{
			{"artist", str},
			{"type",   "master"}
		});
}

int Discogs::estimatedSize() const { return 600; } // NOLINT(*-magic-numbers)

QString Discogs::privateIdentifier() const { return "discogs"; }
