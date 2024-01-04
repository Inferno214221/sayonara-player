/* FMStreamParser.cpp */

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

#include "FMStreamParser.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"

#include <QRegExp>
#include <map>

namespace
{
	QString extractRegexp(const QString& data, const QString& regexString)
	{
		const auto replaceMap = std::map<QString, QString> {
			{"  ",  " "},
			{"??",  ""},
			{"? ?", ""}};

		auto re = QRegExp(regexString);
		re.setMinimal(true);

		auto index = re.indexIn(data);
		if(index >= 0)
		{
			auto ret = re.cap(1).trimmed();
			ret.remove("<br>", Qt::CaseInsensitive);

			for(const auto& [key, value]: replaceMap)
			{
				while(ret.contains(key))
				{
					ret.replace(key, value);
				}
			}

			return ret;
		}

		return {};
	}

	RadioStation parseStn1Block(const QString& data)
	{
		auto station = RadioStation {};

		station.name = extractRegexp(data, "<h3>(.*)</h3>");
		station.image = extractRegexp(data, "<img.+src=\"(.*)\"");
		station.location = extractRegexp(data, "class=\"loc\".*>(.*)<.{0,1}span");
		station.style = extractRegexp(data, "class=\"sty\".*>(.*)<.{0,1}span");
		station.frequency = extractRegexp(data, "class=\"frq\".*>(.*)<.{0,1}span");
		station.description = extractRegexp(data, "class=\"desc\".*>(.*)<.{0,1}span");
		station.short_description = extractRegexp(data, "class=\"bra\".*>(.*)<.{0,1}span");
		station.home_url = extractRegexp(data, "href=\"(.+)\"");

		if(station.description.isEmpty())
		{
			station.description = station.short_description;
		}

		if(station.description.isEmpty())
		{
			station.description = station.style;
		}

		if(station.short_description.isEmpty())
		{
			station.short_description = station.description;
		}

		return station;
	}

	RadioStation parseStnBlock(const QString& stnBlock)
	{
		const auto stn1block = extractRegexp(stnBlock, "<div class=\"stn1\".*>(.*)</div");

		auto station = parseStn1Block(stn1block);
		station.index = extractRegexp(stnBlock, "tf\\(([0-9]+),this").toInt();

		return station;
	}

	QString formatItem(QString item)
	{
		item = item.trimmed();

		if(item.startsWith("'"))
		{
			item.remove(0, 1);
		}

		if(item.endsWith("'"))
		{
			item.remove(item.size() - 1, 1);
		}

		return item.trimmed();
	}

	QString formatItemUrl(QString item)
	{
		item = item.trimmed();
		item = formatItem(item);
		item.replace("\\/", "/");
		if(!item.startsWith("http://"))
		{
			item.prepend("http://");
		}

		if(item.endsWith(';'))
		{
			item.remove(item.size() - 1, 1);
		}

		return item;
	}

	RadioUrl extractStream(const QStringList& items)
	{
		auto stream = RadioUrl {};

		if(items.size() >= 7) // NOLINT(readability-magic-numbers)
		{
			stream.url = formatItemUrl(items[0]);
			stream.type = formatItem(items[1]);
			stream.bitrate = formatItem(items[2]);
			stream.index = formatItem(items[6]).toInt(); // NOLINT(readability-magic-numbers)
		}

		return stream;
	}

	QList<RadioUrl> extractStreams(const QString& data)
	{
		auto streams = QList<RadioUrl> {};
		auto re = QRegExp("\\[(\\'.+)\\]"); // NOLINT
		re.setMinimal(true);

		auto index = re.indexIn(data);
		while(index >= 0)
		{
			const auto items = re.cap(1).split(",");
			const auto stream = extractStream(items);
			if(!stream.url.isEmpty())
			{
				streams << stream;
			}

			index = re.indexIn(data, index + 1);
		}

		return streams;
	}

	QList<RadioStation> extractStations(const QString& text)
	{
		const auto re = QRegExp("stnblock.*>(.*)");
		auto stations = QList<RadioStation> {};

		auto index = re.indexIn(text);
		while(index >= 0)
		{
			stations << parseStnBlock(re.cap(1));
			index = re.indexIn(text, index + 8); // NOLINT(readability-magic-numbers)
		}

		return stations;
	}
} // namespace


QList<RadioStation> FMStreamParser::parse(const QByteArray& data) const
{
	const auto text = QString::fromUtf8(data);
	const auto streams = extractStreams(text);
	auto stations = extractStations(text);

	for(const auto& stream: streams)
	{
		for(auto& station: stations)
		{
			if(station.index == stream.index)
			{
				station.streams << stream;
			}
		}
	}

	Util::Algorithm::removeIf(stations, [](const auto& station) {
		return station.streams.isEmpty();
	});

	return stations;
}
