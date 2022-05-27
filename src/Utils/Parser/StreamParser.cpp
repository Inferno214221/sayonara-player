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
#include "Utils/WebAccess/WebClientImpl.h"
#include "Utils/WebAccess/IcyWebAccess.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Parser/PodcastParser.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"

#include <QFile>
#include <QDir>
#include <QUrl>

namespace Algorithm = Util::Algorithm;

namespace
{
	constexpr const auto MaxSizeUrls = 5000;
	constexpr const auto MinimumUrlCharacters = 7;
}

struct StreamParser::Private
{
	// If an url leads me to some website content and I have to parse it
	// and this Url is found again during parsing, it cannot be a stream
	// and so, it cannot be a metadata object
	QStringList forbiddenUrls;
	QString stationName;
	QString lastUrl;
	QString coverUrl;
	MetaDataList tracks;
	QStringList urls;
	WebClient* activeWebClient {nullptr};
	IcyWebAccess* activeIcy {nullptr};
	int timeout {5'000}; // NOLINT(readability-magic-numbers)
	bool stopped {false};

	[[nodiscard]] bool isUrlForbidden(const QUrl& url) const
	{
		constexpr const auto HttpPort = 80;
		return Util::Algorithm::contains(forbiddenUrls, [&](const auto& forbiddenUrlString) {
			const auto forbiddenUrl = QUrl(forbiddenUrlString);
			const auto forbiddenHost = forbiddenUrl.host();

			return ((forbiddenHost.compare(url.host(), Qt::CaseInsensitive) == 0) &&
			        (forbiddenUrl.port(HttpPort) == url.port(HttpPort)) &&
			        (forbiddenUrl.path() == url.path()) &&
			        (forbiddenUrl.fileName() == url.path()));
		});
	}

	[[nodiscard]] QString writePlaylistFile(const QByteArray& data) const
	{
		const auto extension = Util::File::getFileExtension(lastUrl);
		auto filename = Util::tempPath("ParsedPlaylist");

		if(!extension.isEmpty())
		{
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
		const auto urls = QStringList {stationUrl};
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

	if(m->urls.size() > MaxSizeUrls)
	{
		emit sigUrlCountExceeded(m->urls.size(), MaxSizeUrls);
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
		emit sigFinished(!m->tracks.empty());
		return false;
	}

	m->activeWebClient = new WebClientImpl(this);
	m->activeWebClient->setMode(WebClient::Mode::AsSayonara);

	connect(m->activeWebClient, &WebClient::sigFinished, this, &StreamParser::awaFinished);

	const QString url = m->urls.takeFirst();
	m->activeWebClient->run(url, m->timeout);

	return true;
}

void StreamParser::awaFinished()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());

	const auto status = webClient->status();
	m->lastUrl = webClient->url();
	m->activeWebClient = nullptr;

	if(m->stopped)
	{
		webClient->deleteLater();
		emit sigStopped();
		return;
	}

	switch(status)
	{
		case WebClient::Status::GotData:
		{
			m->forbiddenUrls << m->lastUrl;
			spLog(Log::Develop, this) << "Got data. Try to parse content";

			const auto result = parseContent(webClient->data());

			m->tracks << result.first;
			m->urls << result.second;

			m->tracks.removeDuplicates();
			m->urls.removeDuplicates();
		}
			break;

		case WebClient::Status::NoHttp:
		{
			spLog(Log::Develop, this) << "No correct http was found. Maybe Icy?";

			auto* iwa = new IcyWebAccess(this);
			m->activeIcy = iwa;
			connect(iwa, &IcyWebAccess::sigFinished, this, &StreamParser::icyFinished);
			iwa->check(QUrl(m->lastUrl));

			webClient->deleteLater();
		}
			return;

		case WebClient::Status::AudioStream:
		{
			spLog(Log::Develop, this) << "Found audio stream";
			MetaData md;
			setMetadataTag(md, m->lastUrl, m->coverUrl);

			m->tracks << md;
			m->tracks.removeDuplicates();
		}
			break;

		default:
			spLog(Log::Develop, this) << "Web Access finished: " << int(status);
	}

	webClient->deleteLater();

	if(m->urls.size() > MaxSizeUrls)
	{
		emit sigUrlCountExceeded(m->urls.size(), MaxSizeUrls);
	}

	else
	{
		parseNextUrl();
	}
}

void StreamParser::icyFinished()
{
	auto* iwa = dynamic_cast<IcyWebAccess*>(sender());
	IcyWebAccess::Status status = iwa->status();
	m->activeIcy = nullptr;

	if(m->stopped)
	{
		iwa->deleteLater();
		emit sigStopped();
		return;
	}

	if(status == IcyWebAccess::Status::Success)
	{
		spLog(Log::Develop, this) << "Stream is icy stream";
		auto metadata = MetaData {};
		setMetadataTag(metadata, m->lastUrl, m->coverUrl);

		m->tracks << metadata;
		m->tracks.removeDuplicates();
	}

	else
	{
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
		result.first = PlaylistParser::parsePlaylist(filename, false);
		QFile::remove(filename);
	}

	if(result.first.isEmpty())
	{
		result = parseWebsite(data);
	}

	else
	{
		for(auto& metadata: m->tracks)
		{
			setMetadataTag(metadata, m->lastUrl, m->coverUrl);
		}
	}

	return result;
}

QPair<MetaDataList, PlaylistFiles> StreamParser::parseWebsite(const QByteArray& arr) const
{
	auto tracks = MetaDataList {};
	auto playlistFiles = QStringList {};

	auto validExtensions = QStringList {};
	validExtensions << Util::soundfileExtensions(false);
	validExtensions << Util::playlistExtensions(false);

	const auto* rePrefix = "(http[s]*://|\"/|'/)";
	const auto rePath = "\\S+\\.(" + validExtensions.join("|") + ")";
	const auto reString = QString("(%1%2)").arg(rePrefix).arg(rePath);

	const auto regExp = QRegExp(reString);
	const auto parentUrl = QUrl(m->lastUrl);

	const auto website = QString::fromUtf8(arr);
	auto idx = regExp.indexIn(website);

	auto foundUrls = QStringList {};
	while(idx >= 0)
	{
		const auto foundStrings = regExp.capturedTexts();
		for(auto str: foundStrings)
		{
			if((str.size() > MinimumUrlCharacters) && !m->isUrlForbidden(QUrl(str)))
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

	for(const auto& foundUrl: Algorithm::AsConst(foundUrls))
	{
		auto url = QUrl(foundUrl);
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

	return {tracks, playlistFiles};
}

void StreamParser::setMetadataTag(MetaData& metadata, const QString& streamUrl, const QString& coverUrl) const
{
	metadata.setRadioStation(streamUrl, m->stationName);

	if(metadata.filepath().trimmed().isEmpty())
	{
		metadata.setFilepath(streamUrl);
	}

	if(!coverUrl.isEmpty())
	{
		metadata.setCoverDownloadUrls({coverUrl});
	}
}

MetaDataList StreamParser::tracks() const
{
	return m->tracks;
}

void StreamParser::setCoverUrl(const QString& coverUrl)
{
	m->coverUrl = coverUrl;

	for(auto& track: m->tracks)
	{
		track.setCoverDownloadUrls({coverUrl});
	}
}

void StreamParser::stop()
{
	m->stopped = true;

	if(m->activeWebClient)
	{
		auto* webClient = m->activeWebClient;
		m->activeWebClient = nullptr;
		webClient->stop();
	}

	if(m->activeIcy)
	{
		auto* icy = m->activeIcy;
		m->activeIcy = nullptr;
		icy->stop();
	}
}

bool StreamParser::isStopped() const
{
	return m->stopped;
}
