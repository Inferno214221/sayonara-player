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

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Parser/PodcastParser.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"
#include "Utils/WebAccess/IcyWebAccess.h"
#include "Utils/WebAccess/WebClientFactory.h"
#include "Utils/WebAccess/WebClient.h"

#include <QFile>
#include <QDir>
#include <QUrl>
#include <utility>

namespace
{
	using Urls = QStringList;

	constexpr const auto MaxSizeUrls = 5000;
	constexpr const auto MinimumUrlCharacters = 7;

	QStringList streamableExtensions()
	{
		return QStringList()
			<< Util::soundfileExtensions(false)
			<< Util::playlistExtensions(false);
	}

	QRegExp createRegularExpression()
	{
		const auto validExtensions = streamableExtensions();
		const auto* rePrefix = "(http[s]*://|\"/|'/)";
		const auto rePath = "\\S+\\.(" + validExtensions.join("|") + ")";
		const auto reString = QString("(%1%2)").arg(rePrefix).arg(rePath);
		return QRegExp(reString);
	}

	bool isUrlForbidden(const QUrl& url, const Urls& ignoredUrls)
	{
		constexpr const auto HttpPort = 80;
		return Util::Algorithm::contains(ignoredUrls, [&](const auto& ignoredUrlString) {
			const auto ignoredUrl = QUrl(ignoredUrlString);
			const auto ignoredHost = ignoredUrl.host();

			return ((ignoredHost.compare(url.host(), Qt::CaseInsensitive) == 0) &&
			        (ignoredUrl.port(HttpPort) == url.port(HttpPort)) &&
			        (ignoredUrl.path() == url.path()) &&
			        (ignoredUrl.fileName() == url.path()));
		});
	}

	MetaData setMetadataTag(MetaData track, const QString& stationName, const QString& streamUrl,
	                        const QString& coverUrl = QString())
	{
		track.setRadioStation(streamUrl, stationName);

		if(track.filepath().trimmed().isEmpty())
		{
			track.setFilepath(streamUrl);
		}

		if(!coverUrl.isEmpty())
		{
			track.setCoverDownloadUrls({coverUrl});
		}

		return track;
	}

	QString makeAbsolute(const QString& urlString, const QUrl& baseUrl)
	{
		auto url = QUrl(urlString);
		if(url.isRelative())
		{
			url.setScheme(baseUrl.scheme());
			url.setHost(baseUrl.host());
		}

		return url.toString();
	}

	QStringList
	checkCapturedStrings(const QStringList& capturedStrings, const QUrl& baseUrl, const Urls& ignoredUrls)
	{
		auto foundUrls = QStringList {};
		for(auto capturedString: capturedStrings)
		{
			if((capturedString.size() > MinimumUrlCharacters) &&
			   !isUrlForbidden(QUrl(capturedString), ignoredUrls))
			{
				if(capturedString.startsWith("\"") || capturedString.startsWith("'"))
				{
					capturedString.remove(0, 1);
				}

				foundUrls << makeAbsolute(capturedString, baseUrl);
			}
		}

		return foundUrls;
	}

	QPair<MetaDataList, Urls>
	splitUrlsIntoTracksAndPlaylists(const Urls& urls, const QUrl& baseUrl, const QString& stationName)
	{
		auto result = QPair<MetaDataList, Urls> {};
		auto& [parsedTracks, parsedUrls] = result;

		for(const auto& urlString: urls)
		{
			if(Util::File::isPlaylistFile(urlString))
			{
				parsedUrls << urlString;
			}

			else if(Util::File::isSoundFile(urlString))
			{
				const auto path = QUrl(urlString).path();
				const auto filename = Util::File::getFilenameOfPath(path);
				if(!filename.trimmed().isEmpty())
				{
					auto track = setMetadataTag(MetaData {}, stationName, urlString);
					track.setTitle(filename);
					track.setCoverDownloadUrls({baseUrl.toString()});

					parsedTracks << std::move(track);
				}
			}
		}

		return result;
	}

	QPair<MetaDataList, Urls>
	tryParseWebsite(const QByteArray& data, const QUrl& baseUrl, const QString& stationName, const Urls& ignoredUrls)
	{
		const auto regExp = createRegularExpression();
		auto foundUrls = Urls {};

		const auto website = QString::fromLocal8Bit(data);
		auto index = regExp.indexIn(website);
		while(index >= 0)
		{
			const auto capturedStrings = regExp.capturedTexts();
			foundUrls << checkCapturedStrings(capturedStrings, baseUrl, ignoredUrls);

			index = regExp.indexIn(website, index + 1);
		}

		foundUrls.removeDuplicates();
		return splitUrlsIntoTracksAndPlaylists(foundUrls, baseUrl, stationName);
	}

	[[nodiscard]] QString writePlaylistFile(const QString& extension, const QByteArray& data)
	{
		auto filename = Util::tempPath("ParsedPlaylist");
		if(!extension.isEmpty())
		{
			filename += "." + extension;
		}

		Util::File::writeFile(data, filename);

		return filename;
	}

	MetaDataList tryParsePlaylist(const QString& url, const QByteArray& data)
	{
		const auto extension = Util::File::getFileExtension(url);
		const auto filename = writePlaylistFile(extension, data);
		auto result = PlaylistParser::parsePlaylist(filename, false);

		Util::File::deleteFiles({filename});

		return result;
	}

	MetaDataList tryParsePodcastFile(const QByteArray& data)
	{
		return PodcastParser::parsePodcastXmlFile(data);
	}

	QPair<MetaDataList, Urls>
	parseContent(const QString& url, const QByteArray& data, const QString& coverUrl, const QString& stationName,
	             const Urls& forbiddenUrls)
	{
		auto result = QPair<MetaDataList, Urls> {};
		auto& [tracks, _] = result;

		tracks = tryParsePodcastFile(data);

		if(tracks.isEmpty())
		{
			tracks = tryParsePlaylist(url, data);
		}

		if(tracks.isEmpty())
		{
			result = tryParseWebsite(data, url, stationName, forbiddenUrls);
		}

		Util::Algorithm::transform(tracks, [&](const auto& track) {
			return setMetadataTag(track, stationName, url, coverUrl);
		});

		return result;
	}

	template<typename Callback>
	WebClient* createWebClient(WebClientFactory& webClientFactory, StreamParser* parent, Callback callback)
	{
		auto* webClient = webClientFactory.createClient(parent);
		webClient->setMode(WebClient::Mode::AsSayonara);
		QObject::connect(webClient, &WebClient::sigFinished, parent, callback);
		QObject::connect(parent, &StreamParser::sigStopped, webClient, &WebClient::stop);
		QObject::connect(parent, &StreamParser::sigStopped, webClient, &WebClient::deleteLater);

		return webClient;
	}

	template<typename Callback>
	IcyWebAccess* createIcyClient(StreamParser* parent, Callback callback)
	{
		auto* icyWebAccess = new IcyWebAccess(parent);
		QObject::connect(icyWebAccess, &IcyWebAccess::sigFinished, parent, [icyWebAccess, callback]() {
			callback(icyWebAccess);
		});
		QObject::connect(parent, &StreamParser::sigStopped, icyWebAccess, &IcyWebAccess::stop);
		QObject::connect(parent, &StreamParser::sigStopped, icyWebAccess, &IcyWebAccess::deleteLater);

		return icyWebAccess;
	}

	MetaDataList removeDuplicates(MetaDataList tracks)
	{
		for(auto it = tracks.begin(); it != tracks.end(); it++)
		{
			auto rit = std::remove_if(std::next(it), tracks.end(), [filepath = it->filepath()](const auto& track) {
				return (filepath == track.filepath());
			});

			tracks.erase(rit, tracks.end());
		}

		return tracks;
	}
}

struct StreamParser::Private
{
	// If an url leads me to some website content and I have to parse it
	// and this Url is found again during parsing, it cannot be a stream
	// and so, it cannot be a metadata object
	Urls forbiddenUrls;
	QString stationName;
	QString coverUrl;
	MetaDataList tracks;
	Urls urls;
	std::shared_ptr<WebClientFactory> webClientFactory;
	int timeout {0};
	bool stopped {false};

	explicit Private(std::shared_ptr<WebClientFactory> webClientFactory) :
		webClientFactory {std::move(webClientFactory)} {}
};

StreamParser::StreamParser(const std::shared_ptr<WebClientFactory>& webClientFactory, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(webClientFactory);
}

StreamParser::~StreamParser() = default;

void StreamParser::parse(const QString& stationName, const QString& stationUrl, const int timeout)
{
	m->stationName.clear();

	if(!stationUrl.isEmpty())
	{
		m->stationName = stationName;
		parse(Urls {stationUrl}, timeout);
	}
}

void StreamParser::parse(const Urls& urls, const int timeout)
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
		emit sigFinished(!m->tracks.empty());
		return false;
	}

	auto* webClient = createWebClient(*m->webClientFactory, this, &StreamParser::webClientFinished);
	webClient->run(m->urls.takeFirst(), m->timeout);

	return true;
}

void StreamParser::webClientFinished()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());

	const auto status = webClient->status();
	const auto url = webClient->url();
	const auto data = webClient->data();

	webClient->deleteLater();

	switch(status)
	{
		case WebClient::Status::GotData:
		{
			m->forbiddenUrls << url;
			spLog(Log::Develop, this) << "Got data. Try to parse content";

			auto [tracks, urls] = parseContent(url, data, m->coverUrl, m->stationName, m->forbiddenUrls);
			m->tracks = removeDuplicates(std::move(tracks));
			m->urls << urls;
			m->urls.removeDuplicates();
		}
			break;

		case WebClient::Status::NoHttp:
		{
			spLog(Log::Develop, this) << "No correct http was found. Maybe Icy?";

			auto* icyWebAccess = createIcyClient(this, [&, url](auto* icy) {
				this->icyFinished(url, icy);
			});

			icyWebAccess->check(QUrl(url));
		}
			return;

		case WebClient::Status::AudioStream:
		{
			spLog(Log::Develop, this) << "Found audio stream";

			m->tracks << setMetadataTag(MetaData {}, m->stationName, url, m->coverUrl);
			m->tracks = removeDuplicates(std::move(m->tracks));
		}
			break;

		default:
			spLog(Log::Develop, this) << "Web Access finished: " << int(status);
	}

	if(m->urls.size() > MaxSizeUrls)
	{
		emit sigUrlCountExceeded(m->urls.size(), MaxSizeUrls);
	}

	else
	{
		parseNextUrl();
	}
}

void StreamParser::icyFinished(const QString& url, IcyWebAccess* icyWebAccess)
{
	const auto status = icyWebAccess->status();
	if(status == IcyWebAccess::Status::Success)
	{
		spLog(Log::Develop, this) << "Stream is icy stream";

		m->tracks << setMetadataTag(MetaData {}, m->stationName, url, m->coverUrl);
		m->tracks = removeDuplicates(std::move(m->tracks));
	}

	else
	{
		spLog(Log::Develop, this) << "Stream is no icy stream";
	}

	icyWebAccess->deleteLater();

	parseNextUrl();
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
	emit sigStopped();
}

bool StreamParser::isStopped() const { return m->stopped; }
