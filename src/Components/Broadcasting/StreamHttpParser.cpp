/* StreamHttpParser.cpp */

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

#include "StreamHttpParser.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QRegExp>
#include <QStringList>

namespace Algorithm = Util::Algorithm;

struct StreamHttpParser::Private
{
	bool icy;
	QString host;

	Private()
	{
		icy = false;
	}
};

QString StreamHttpParser::answerString(StreamHttpParser::HttpAnswer answer)
{
	using Answer = StreamHttpParser::HttpAnswer;
	switch(answer)
	{
		case Answer::BG:
			return "Background image";
		case Answer::Fail:
			return "Fail";
		case Answer::Favicon:
			return "Favicon";
		case Answer::HTML5:
			return "Html5";
		case Answer::Ignore:
			return "Ignore";
		case Answer::MP3:
			return "MP3";
		case Answer::MetaData:
			return "Metadata";
		case Answer::OK:
			return "OK";
		case Answer::Playlist:
			return "Playlist";
		case Answer::Reject:
			return "Reject";
		default:
			return "Unknown answer";
	}
}

StreamHttpParser::StreamHttpParser()
{
	m = Pimpl::make<Private>();
}

StreamHttpParser::~StreamHttpParser() = default;

StreamHttpParser::HttpAnswer StreamHttpParser::parse(const QByteArray& data)
{
	bool getPlaylist = false;
	bool getReceived = false;
	bool getMp3 = false;
	bool getBg = false;
	bool getFavicon = false;
	bool getMetadata = false;
	bool icy = false;
	bool isBrowser = false;

	QString qmsg(data);
	QStringList lst;

	m->icy = false;
	m->host = "";

	if(data.isEmpty())
	{
		spLog(Log::Error, this) << "Fail.. Cannot read from socket";
		return HttpAnswer::Fail;
	}

	lst = qmsg.split("\r\n");

	spLog(Log::Develop, this) << qmsg;

	for(const QString& str: Algorithm::AsConst(lst))
	{
		QRegExp regex("(GET|HEAD)(\\s|/)*HTTP", Qt::CaseInsensitive);
		QRegExp regexPl("(GET)(\\s|/)*(playlist.m3u)(\\s|/)*HTTP", Qt::CaseInsensitive);
		QRegExp regexMp3("(.*GET(\\s|/)*.*(\\.mp3)(\\s|/)*HTTP)", Qt::CaseInsensitive);
		QRegExp regexBg("(GET)(\\s|/)*(bg-checker.png)(\\s|/)*HTTP", Qt::CaseInsensitive);
		QRegExp regexFavicon("(GET)(\\s|/)*(favicon.ico)(\\s|/)*HTTP", Qt::CaseInsensitive);
		QRegExp regexMetadata("(GET)(\\s|/)*(metadata)(\\s|/)*HTTP", Qt::CaseInsensitive);

		if(str.contains(regex))
		{
			getReceived = true;
			continue;
		}

		if(str.contains(regexMetadata))
		{
			getMetadata = true;
		}

		if(str.contains(regexFavicon))
		{
			getFavicon = true;
			continue;
		}

		if(str.contains(regexPl))
		{
			getPlaylist = true;
			continue;
		}

		spLog(Log::Debug, this) << "Client asks for MP3? " << str.contains(regexMp3);
		if(str.contains(regexMp3))
		{
			getMp3 = true;
			continue;
		}

		if(str.contains(regexBg))
		{
			getBg = true;
			continue;
		}

		if(str.contains(QStringLiteral("host:"), Qt::CaseInsensitive))
		{
			const QStringList lst = str.split(":");
			if(lst.size() > 1)
			{
				m->host = lst[1].trimmed();
			}
		}

		if(str.contains(QStringLiteral("icy-metadata:"), Qt::CaseInsensitive))
		{
			if(str.contains(QStringLiteral(":1")) || str.contains(QStringLiteral(": 1")))
			{
				icy = true;
				continue;
			}
		}

		if(str.contains(QStringLiteral("user-agent"), Qt::CaseInsensitive))
		{
			if(str.size() > 11)
			{
				QString userAgent = str.right(str.size() - 11).toLower();
				if(userAgent.contains(QStringLiteral("firefox"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("mozilla"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("gecko"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("webkit"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("safari"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("internet explorer"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("opera"), Qt::CaseInsensitive) ||
				   userAgent.contains(QStringLiteral("chrom"), Qt::CaseInsensitive))
				{
					isBrowser = true;
				}

				if(userAgent.contains(QStringLiteral("sayonara"), Qt::CaseInsensitive))
				{
					getPlaylist = true;
					continue;
				}
			}
		}
	}

	if(isBrowser && getFavicon && !m->host.isEmpty())
	{
		return HttpAnswer::Favicon;
	}

	if(isBrowser && getBg && !m->host.isEmpty())
	{
		return HttpAnswer::BG;
	}

	if(isBrowser && getMetadata && !m->host.isEmpty())
	{
		return HttpAnswer::MetaData;
	}

	if(isBrowser && !getMp3 && !m->host.isEmpty())
	{
		return HttpAnswer::HTML5;
	}

	if(getMp3 && !m->host.isEmpty())
	{
		return HttpAnswer::MP3;
	}

	if(getPlaylist && !m->host.isEmpty())
	{
		return HttpAnswer::Playlist;
	}

	if(getReceived)
	{
		m->icy = icy;

		return HttpAnswer::OK;
	}

	return HttpAnswer::Fail;
}

bool StreamHttpParser::isIcyStream() const
{
	return m->icy;
}

QString StreamHttpParser::host() const
{
	return m->host;
}
