/* StreamParser.cpp */

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

#include "StreamParser.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/FileUtils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/WebAccess/IcyWebAccess.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Parser/PodcastParser.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"

#include <QFile>
#include <QDir>
#include <QUrl>

namespace Algorithm=Util::Algorithm;

struct StreamParser::Private
{
	// If an url leads me to some website content and I have to parse it
	// and this Url is found again during parsing, it cannot be a stream
	// and so, it cannot be a metadata object
	QStringList		forbiddenUrls;
	QString			stationName;
	QString			lastUrl;
	QString			coverUrl;
	MetaDataList	tracks;
	QStringList 	urls;
	AsyncWebAccess* activeAwa=nullptr;
	IcyWebAccess*	activeIcy=nullptr;
	const int		MaxSizeUrls=5000;
	int				timeout;
	bool			stopped;

	Private() :
		timeout(5000),
		stopped(false)
	{}

	bool isUrlForbidden(const QUrl& url) const
	{
		for(const QString& fu : forbiddenUrls)
		{
			const QUrl forbiddenUrl(fu);
			const QString forbidden_host = forbiddenUrl.host();

			if ((forbidden_host.compare(url.host(), Qt::CaseInsensitive) == 0) &&
				(forbiddenUrl.port(80) == url.port(80)) &&
				(forbiddenUrl.path().compare(url.path()) == 0) &&
				(forbiddenUrl.fileName().compare(url.path()) == 0))
			{
				return true;
			}
		}

		return false;
	}

	QString writePlaylistFile(const QByteArray& data) const
	{
		QString extension = Util::File::getFileExtension(lastUrl);
		QString filename = Util::tempPath("ParsedPlaylist");

		if(!extension.isEmpty()) {
			filename += "." + extension;
		}

		Util::File::writeFile(data, filename);

		return filename;
	}
};

StreamParser::StreamParser(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

StreamParser::~StreamParser() = default;

void StreamParser::parse(const QString& stationName, const QString& stationUrl, int timeout)
{
	m->stationName.clear();

	if(!stationUrl.isEmpty())
	{
		m->stationName = stationName;
		QStringList urls{ stationUrl };
		parse(urls, timeout);
	}
}

void StreamParser::parse(const QStringList& urls, int timeout)
{
	m->timeout = timeout;
	m->stopped = false;
	m->tracks.clear();

	m->urls = urls;
	m->urls.removeDuplicates();

	if(m->urls.size() > m->MaxSizeUrls)
	{
		emit sigUrlCountExceeded(m->urls.size(), m->MaxSizeUrls);
	}

	else
	{
		parseNextUrl();
	}
}


bool StreamParser::parseNextUrl()
{
	if(m->stopped)
	{
		emit sigStopped();
		return false;
	}

	if(m->urls.isEmpty())
	{
		spLog(Log::Develop, this) << "No more urls to parse";
		emit sigFinished( !m->tracks.empty());
		return false;
	}

	m->activeAwa = new AsyncWebAccess(this);
	m->activeAwa->setBehavior(AsyncWebAccess::Behavior::AsSayonara);

	connect(m->activeAwa, &AsyncWebAccess::sigFinished, this, &StreamParser::awaFinished);

	const QString url = m->urls.takeFirst();
	m->activeAwa->run(url, m->timeout);

	return true;
}


void StreamParser::awaFinished()
{
	auto* awa = dynamic_cast<AsyncWebAccess*>(sender());

	AsyncWebAccess::Status status = awa->status();
	m->lastUrl = awa->url();
	m->activeAwa = nullptr;

	if(m->stopped)
	{
		awa->deleteLater();
		emit sigStopped();
		return;
	}

	switch(status)
	{
		case AsyncWebAccess::Status::GotData:
		{
			m->forbiddenUrls << m->lastUrl;
			spLog(Log::Develop, this) << "Got data. Try to parse content";

			QPair<MetaDataList, PlaylistFiles> result = parseContent(awa->data());

			m->tracks << result.first;
			m->urls << result.second;

			m->tracks.removeDuplicates();
			m->urls.removeDuplicates();
		} break;

		case AsyncWebAccess::Status::NoHttp:
		{
			spLog(Log::Develop, this) << "No correct http was found. Maybe Icy?";

			auto* iwa = new IcyWebAccess(this);
			m->activeIcy = iwa;
			connect(iwa, &IcyWebAccess::sigFinished, this, &StreamParser::icyFinished);
			iwa->check(QUrl(m->lastUrl));

			awa->deleteLater();
		} return;

		case AsyncWebAccess::Status::AudioStream:
		{
			spLog(Log::Develop, this) << "Found audio stream";
			MetaData md;
			setMetadataTag(md, m->lastUrl, m->coverUrl);

			m->tracks << md;
			m->tracks.removeDuplicates();
		} break;

		default:
			spLog(Log::Develop, this) << "Web Access finished: " << int(status);
	}

	awa->deleteLater();

	if(m->urls.size() > m->MaxSizeUrls){
		emit sigUrlCountExceeded(m->urls.size(), m->MaxSizeUrls);
	}

	else {
		parseNextUrl();
	}
}


void StreamParser::icyFinished()
{
	auto* iwa = dynamic_cast<IcyWebAccess*>(sender());
	IcyWebAccess::Status status = iwa->status();
	m->activeIcy = nullptr;

	if(m->stopped){
		iwa->deleteLater();
		emit sigStopped();
		return;
	}

	if(status == IcyWebAccess::Status::Success)
	{
		spLog(Log::Develop, this) << "Stream is icy stream";
		MetaData md;
		setMetadataTag(md, m->lastUrl, m->coverUrl);

		m->tracks << md;
		m->tracks.removeDuplicates();
	}

	else {
		spLog(Log::Develop, this) << "Stream is no icy stream";
	}

	iwa->deleteLater();

	parseNextUrl();
}


QPair<MetaDataList, PlaylistFiles> StreamParser::parseContent(const QByteArray& data) const
{
	QPair<MetaDataList, PlaylistFiles> result;

	spLog(Log::Crazy, this) << QString::fromUtf8(data);

	/** 1. try if podcast file **/
	result.first = PodcastParser::parsePodcastXmlFile(data);

	/** 2. try if playlist file **/
	if(result.first.isEmpty())
	{
		const QString filename = m->writePlaylistFile(data);
		result.first = PlaylistParser::parsePlaylist(filename);
		QFile::remove(filename);
	}

	if(result.first.isEmpty())
	{
		result = parseWebsite(data);
	}

	else
	{
		for(MetaData& md : m->tracks)
		{
			setMetadataTag(md, m->lastUrl, m->coverUrl);
		}
	}

	return result;
}

QPair<MetaDataList, PlaylistFiles> StreamParser::parseWebsite(const QByteArray& arr) const
{
	MetaDataList tracks;
	QStringList playlistFiles;

	QStringList validExtensions;
	validExtensions << Util::soundfileExtensions(false);
	validExtensions << Util::playlistExtensions(false);

	const QString rePrefix = "(http[s]*://|\"/|'/)";
	const QString rePath = "\\S+\\.(" + validExtensions.join("|") + ")";
	const QString reString = "(" + rePrefix + rePath + ")";

	QRegExp regExp(reString);
	const QUrl parentUrl(m->lastUrl);

	const QString website = QString::fromUtf8(arr);
	int idx = regExp.indexIn(website);

	QStringList foundUrls;
	while(idx >= 0)
	{
		const QStringList foundStrings = regExp.capturedTexts();
		for(QString str : foundStrings)
		{
			if((str.size() > 7) && !m->isUrlForbidden(QUrl(str)))
			{
				if(str.startsWith("\"") || str.startsWith("'"))
				{
					str.remove(0, 1);
				}

				foundUrls << str;
			}
		}

		idx = regExp.indexIn(website, idx + 1);
	}

	foundUrls.removeDuplicates();

	for(const QString& foundUrl : Algorithm::AsConst(foundUrls))
	{
		QUrl url(foundUrl);
		if(url.isRelative())
		{
			url.setScheme(parentUrl.scheme());
			url.setHost(parentUrl.host());
		}

		if(Util::File::isPlaylistFile(foundUrl))
		{
			playlistFiles << foundUrl;
		}

		else if(Util::File::isSoundFile(foundUrl))
		{
			MetaData track;
			setMetadataTag(track, url.toString());

			const auto filename = Util::File::getFilenameOfPath(url.path());
			if(!filename.trimmed().isEmpty())
			{
				track.setTitle(filename);
			}

			track.setCoverDownloadUrls({m->lastUrl});
			tracks << track;
		}
	}

	spLog(Log::Develop, this) << "Found " << m->urls.size() << " playlists and " << tracks.size() << " streams";

	return QPair<MetaDataList, PlaylistFiles>(tracks, playlistFiles);
}

void StreamParser::setMetadataTag(MetaData& md, const QString& streamUrl, const QString& coverUrl) const
{
	md.setRadioStation(streamUrl, m->stationName);

	if(md.filepath().trimmed().isEmpty()) {
		md.setFilepath(streamUrl);
	}

	if(!coverUrl.isEmpty()) {
		md.setCoverDownloadUrls({coverUrl});
	}
}

MetaDataList StreamParser::tracks() const
{
	return m->tracks;
}

void StreamParser::setCoverUrl(const QString& coverUrl)
{
	m->coverUrl = coverUrl;

	for(MetaData& md : m->tracks){
		md.setCoverDownloadUrls({coverUrl});
	}
}

void StreamParser::stop()
{
	m->stopped = true;

	if(m->activeAwa)
	{
		AsyncWebAccess* awa = m->activeAwa;
		m->activeAwa = nullptr;
		awa->stop();
	}

	if(m->activeIcy)
	{
		IcyWebAccess* icy = m->activeIcy;
		m->activeIcy = nullptr;
		icy->stop();
	}
}

bool StreamParser::isStopped() const
{
	return m->stopped;
}
