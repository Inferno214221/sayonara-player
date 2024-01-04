/* RadioBrowserSearcher.cpp, (Created on 03.01.2024) */

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
#include "RadioBrowserSearcher.h"
#include "RadioStation.h"

#include <QString>
#include <QUrl>

#include <map>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	RadioUrl getUrl(const QJsonValue& jsonValue)
	{
		RadioUrl result;
		result.index = 0;
		result.bitrate = QString::number(jsonValue["bitrate"].toInt());
		result.type = jsonValue["tags"].toString();
		result.url = jsonValue["url"].toString();
		result.type = jsonValue["codec"].toString();
		if(const auto bitrate = jsonValue["bitrate"].toInt(); bitrate > 0)
		{
			result.type += QString(", %1 kBit/s").arg(bitrate);
		}

		return result;
	}

	QString getLocation(const QJsonValue& jsonValue)
	{
		const auto state = jsonValue["state"].toString();
		const auto country = jsonValue["country"].toString();
		return (state.isEmpty())
		       ? country
		       : QString("%1, %2").arg(state).arg(country);
	}

	QList<RadioStation> parseMainArray(const QJsonArray& arr)
	{
		auto radioStations = QList<RadioStation> {};
		int index = 0;
		for(const auto& element: arr)
		{
			auto radioStation = RadioStation {};

			radioStation.description = element["tags"].toString();
			radioStation.home_url = element["homepage"].toString();
			radioStation.index = index++;
			radioStation.location = getLocation(element);
			radioStation.name = element["name"].toString();
			radioStation.short_description = element["language"].toString();
			radioStation.streams = {getUrl(element)};
			radioStation.style = element["tags"].toString();
			radioStation.image = element["favicon"].toString();

			radioStations << radioStation;
		}

		return radioStations;
	}

	QString paramsToString(const std::map<QString, QString>& params)
	{
		auto list = QStringList {};
		for(const auto& [key, value]: params)
		{
			list << QString("%1=%2").arg(key).arg(value);
		}

		return list.join("&");
	}
}

QString
RadioBrowserSearcher::buildUrl(const QString& searchtext, const StationSearcher::Mode mode, const int page,
                               const int maxEntries) const
{
	const auto baseUrl = "https://at1.api.radio-browser.info/json/stations/search";
	auto params = std::map<QString, QString> {
		{"limit",      QString::number(maxEntries)},
		{"offset",     QString::number(page * maxEntries)},
		{"hidebroken", "true"},
		{"order",      "clickcount"},
		{"reverse",    "true"}
	};

	if(mode == StationSearcher::ByStyle)
	{
		params["tagList"] = QUrl::toPercentEncoding(searchtext);
	}

	else
	{
		params["name"] = QUrl::toPercentEncoding(searchtext);
	}

	return QString("%1?%2")
		.arg(baseUrl)
		.arg(paramsToString(params));
}

std::unique_ptr<StationParser> RadioBrowserSearcher::createStationParser()
{
	return std::make_unique<RadioBrowserParser>();
}

QList<RadioStation> RadioBrowserParser::parse(const QByteArray& data) const
{
	auto err = QJsonParseError {};
	auto document = QJsonDocument::fromJson(data, &err);
	if(err.error != QJsonParseError::NoError)
	{
		return {};
	}

	if(document.isArray())
	{
		return parseMainArray(document.array());
	}

	return {};
}
