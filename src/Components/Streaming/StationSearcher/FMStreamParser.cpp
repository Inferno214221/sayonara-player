/* FMStreamParser.cpp */

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

#include "FMStreamParser.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QRegExp>

namespace Algorithm=Util::Algorithm;

struct FMStreamParser::Private
{
	QList<RadioStation> stations;
};

static QString extract_regexp(const QString& data, const QString& re_str)
{
	QRegExp re(re_str);
	re.setMinimal(true);
	int idx = re.indexIn(data);
	if(idx >= 0)
	{
		QString ret = re.cap(1).trimmed();
		ret.remove("<br>", Qt::CaseInsensitive);
		while(ret.contains("  ")){
			ret.replace("  ", " ");
		}

		while(ret.contains("??")){
			ret.replace("??", "");
		}

		while(ret.contains("? ?")){
			ret.replace("? ?", "");
		}

		return ret;
	}

	return QString();
}


static void parse_stn1_block(const QString& data, RadioStation& station)
{
	station.name =				Util::stringToFirstUpper(extract_regexp(data, "<h3>(.*)</h3>"));
	station.image =				extract_regexp(data, "<img.+src=\"(.*)\"");
	station.location =			Util::stringToFirstUpper(extract_regexp(data, "class=\"loc\".*>(.*)<.{0,1}span"));
	station.style =				extract_regexp(data, "class=\"sty\".*>(.*)<.{0,1}span");
	station.frequency =			extract_regexp(data, "class=\"frq\".*>(.*)<.{0,1}span");
	station.description =		Util::stringToFirstUpper(extract_regexp(data, "class=\"desc\".*>(.*)<.{0,1}span"));
	station.short_description = Util::stringToFirstUpper(extract_regexp(data, "class=\"bra\".*>(.*)<.{0,1}span"));
	station.home_url =			extract_regexp(data, "href=\"(.+)\"");

	if(station.description.isEmpty()){
		station.description = station.short_description;
	}

	if(station.description.isEmpty()){
		station.description = station.style;
	}

	if(station.short_description.isEmpty()){
		station.short_description = station.description;
	}
}

static RadioStation parse_stnblock(const QString& data)
{
	RadioStation station;

	QString stn1block = extract_regexp(data, "<div class=\"stn1\".*>(.*)<div");

	parse_stn1_block(stn1block, station);
	station.index = extract_regexp(data, "tf\\(([0-9]+),this").toInt();

	return station;
}

static QList<RadioUrl> extract_streams(const QString& data)
{
	QList<RadioUrl> streams;
	QRegExp re("\\[(\\'.+)\\]");
	re.setMinimal(true);

	int offset = 0;
	int index = re.indexIn(data, offset);
	while(index >= 0)
	{
		QString cap = re.cap(1);
		QStringList items = cap.split(",");
		if(items.size() < 8){
			offset += index + 1;
			index = re.indexIn(data, offset);
			continue;
		}

		for(QString& item : items)
		{
			if(item.startsWith("'")){
				item.remove(0, 1);
			}

			if(item.endsWith("'")){
				item.remove(item.size()-1, 1);
			}
		}

		RadioUrl stream;
		stream.url = items[0].trimmed();
		stream.url.replace("\\/", "/");
		stream.type = items[1].trimmed();
		stream.bitrate = items[2].trimmed();
		stream.index = items[7].trimmed().toInt();

		if(!stream.type.isEmpty()){
			streams << stream;
		}

		offset = index + 1;

		index = re.indexIn(data, offset);
	}

	return streams;
}


QStringList convert(QByteArray data, FMStreamParser::Encoding from, FMStreamParser::Encoding to, FMStreamParser::Encoding from2, FMStreamParser::Encoding to2)
{
	QString s1;
	switch(from)
	{
		case FMStreamParser::Utf8:
			s1 = QString::fromUtf8(data);
			break;
		case FMStreamParser::Latin1:
			s1 = QString::fromLatin1(data);
			break;
		case FMStreamParser::Local8Bit:
			s1 = QString::fromLocal8Bit(data);
			break;
	}

	QByteArray d1;
	switch(to)
	{
		case FMStreamParser::Utf8:
			d1 = s1.toUtf8();
			break;
		case FMStreamParser::Latin1:
			d1 = s1.toLatin1();
			break;
		case FMStreamParser::Local8Bit:
			d1 = s1.toLocal8Bit();
			break;
	}

	QString s2;
	switch(from2)
	{
		case FMStreamParser::Utf8:
			s2 = QString::fromUtf8(d1);
			break;
		case FMStreamParser::Latin1:
			s2 = QString::fromLatin1(d1);
			break;
		case FMStreamParser::Local8Bit:
			s2 = QString::fromLocal8Bit(d1);
			break;
	}

	QByteArray d2;
	switch(to2)
	{
		case FMStreamParser::Utf8:
			d2 = s2.toUtf8();
			break;
		case FMStreamParser::Latin1:
			d2 = s2.toLatin1();
			break;
		case FMStreamParser::Local8Bit:
			d2 = s2.toLocal8Bit();
			break;
	}

	QStringList ret{
		QString(d2),
		QString::fromUtf8(d2),
		QString::fromLatin1(d2),
		QString::fromLocal8Bit(d2),
		QString::fromStdString(std::string(d2.data())),
	};

	return ret;
}


FMStreamParser::FMStreamParser(const QByteArray& data, FMStreamParser::EncodingTuple encodings, int encoding_version)
{
	m = Pimpl::make<Private>();

	QString text = convert(data, encodings[0], encodings[1], encodings[2], encodings[3])[encoding_version];

	QList<RadioStation> stations;

	const QList<RadioUrl> streams = extract_streams(text);

	int offset = 0;

	QRegExp re("stnblock.*>(.*)");

	int index = re.indexIn(text);
	while(index >= 0)
	{
		stations << parse_stnblock(re.cap(1));

		offset = index + 1;
		index = re.indexIn(text, offset);
	}

	Algorithm::sort(stations, [](const RadioStation& s1, const RadioStation& s2){
		return (s1.name < s2.name);
	});

	for(const RadioUrl& stream : streams)
	{
		for(RadioStation& station : stations)
		{
			if(station.index == stream.index)
			{
				station.streams << stream;
			}
		}
	}

	for(RadioStation& station : stations)
	{
		if(station.streams.count() > 0)
		{
			m->stations << station;
		}
	}
}


FMStreamParser::FMStreamParser(const QByteArray& data) :
	FMStreamParser(data, EncodingTuple{{Utf8, Utf8, Utf8, Latin1}}, 0)
{}

FMStreamParser::~FMStreamParser() {}

QList<RadioStation> FMStreamParser::stations() const
{
	return m->stations;
}
