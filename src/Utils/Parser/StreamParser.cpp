/* StreamParser.cpp */

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

#include "StreamParser.h"

#include "Utils/Algorithm.h"
#include "Utils/FileSystem.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Parser/PodcastParser.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"
#include "Utils/WebAccess/IcyWebAccess.h"
#include "Utils/WebAccess/WebClient.h"
#include "Utils/WebAccess/WebClientFactory.h"

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
		if((track.radioMode() != RadioMode::Podcast) || track.title().isEmpty())
		{
			track.setRadioStation(streamUrl, stationName);
		}

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

	[[nodiscard]] QString
	writePlaylistFile(const QString& extension, const QByteArray& data, const Util::FileSystemPtr& fileSystem)
	{
		auto filename = Util::tempPath("ParsedPlaylist");
		if(!extension.isEmpty())
		{
			filename += "." + extension;
		}

		fileSystem->writeFile(data, filename);

		return filename;
	}

	MetaDataList tryParsePlaylist(const QString& url, const QByteArray& data, const Util::FileSystemPtr& fileSystem)
	{
		const auto extension = Util::File::getFileExtension(url);
		const auto filename = writePlaylistFile(extension, data, fileSystem);
		auto result = PlaylistParser::parsePlaylistWithoutTags(filename, fileSystem);

		fileSystem->deleteFiles({filename});

		return result;
	}

	MetaDataList tryParsePodcastFile(const QByteArray& data)
	{
		return PodcastParser::parsePodcastXmlFile(data);
	}

	QPair<MetaDataList, Urls>
	parseContent(const QString& url, const QByteArray& data, const QString& coverUrl, const QString& stationName,
	             const Urls& forbiddenUrls,
	             const Util::FileSystemPtr& fileSystem)
	{
		auto result = QPair<MetaDataList, Urls> {};
		auto& [tracks, _] = result;

		tracks = tryParsePodcastFile(data);

		if(tracks.isEmpty())
		{
			tracks = tryParsePlaylist(url, data, fileSystem);
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

class StreamParserImpl :
	public StreamParser
{
	Q_OBJECT

	public:
		StreamParserImpl(const std::shared_ptr<WebClientFactory>& webClientFactory, Util::FileSystemPtr fileSystem,
		                 QObject* parent) :
			StreamParser(parent),
			m_webClientFactory {webClientFactory},
			m_fileSystem {std::move(fileSystem)} {}

		~StreamParserImpl() override = default;

		void parse(const QString& name, const QStringList& urls, const QString& userAgent, const int timeout) override
		{
			m_timeout = timeout;
			m_stopped = false;
			m_tracks.clear();

			m_stationName = name;
			m_urls = urls;
			m_urls.removeDuplicates();
			m_userAgent = userAgent;

			if(m_urls.size() > MaxSizeUrls)
			{
				emit sigUrlCountExceeded(m_urls.size(), MaxSizeUrls);
			}

			else
			{
				parseNextUrl();
			}
		}

		void stopParsing() override
		{
			m_stopped = true;
			emit sigStopped();
		}

		[[nodiscard]] bool isStopped() const override { return m_stopped; }

		bool parseNextUrl()
		{
			if(m_stopped)
			{
				emit sigStopped();
				return false;
			}

			if(m_urls.isEmpty())
			{
				emit sigFinished(!m_tracks.empty());
				return false;
			}

			auto* webClient = m_webClientFactory->createClient(this);

			if(m_userAgent.isEmpty())
			{
				webClient->setMode(WebClient::Mode::AsSayonara);
			}
			else
			{
				webClient->setUserAgent(m_userAgent);
			}

			QObject::connect(webClient, &WebClient::sigFinished, this, &StreamParserImpl::webClientFinished);
			QObject::connect(this, &StreamParser::sigStopped, webClient, &WebClient::stop);
			QObject::connect(this, &StreamParser::sigStopped, webClient, &WebClient::deleteLater);

			webClient->run(m_urls.takeFirst(), m_timeout);

			return true;
		}

		[[nodiscard]] MetaDataList tracks() const override { return m_tracks; }

		void setCoverUrl(const QString& coverUrl) override
		{
			m_coverUrl = coverUrl;

			for(auto& track: m_tracks)
			{
				track.setCoverDownloadUrls({coverUrl});
			}
		}

	private slots:

		void webClientFinished()
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
					m_forbiddenUrls << url;
					spLog(Log::Develop, this) << "Got data. Try to parse content";

					auto [tracks, urls] = parseContent(url,
					                                   data,
					                                   m_coverUrl,
					                                   m_stationName,
					                                   m_forbiddenUrls,
					                                   m_fileSystem);
					m_tracks = removeDuplicates(std::move(tracks));
					m_urls << urls;
					m_urls.removeDuplicates();
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

					m_tracks << setMetadataTag(MetaData {}, m_stationName, url, m_coverUrl);
					m_tracks = removeDuplicates(std::move(m_tracks));
				}
					break;

				default:
					spLog(Log::Develop, this) << "Web Access finished: " << int(status);
			}

			if(m_urls.size() > MaxSizeUrls)
			{
				emit sigUrlCountExceeded(m_urls.size(), MaxSizeUrls);
			}

			else
			{
				parseNextUrl();
			}
		}

		void icyFinished(const QString& url, IcyWebAccess* icyWebAccess)
		{
			const auto status = icyWebAccess->status();
			if(status == IcyWebAccess::Status::Success)
			{
				spLog(Log::Develop, this) << "Stream is icy stream";

				m_tracks << setMetadataTag(MetaData {}, m_stationName, url, m_coverUrl);
				m_tracks = removeDuplicates(std::move(m_tracks));
			}

			else
			{
				spLog(Log::Develop, this) << "Stream is no icy stream";
			}

			icyWebAccess->deleteLater();

			parseNextUrl();
		}

	private: // NOLINT(readability-redundant-access-specifiers)
		Urls m_forbiddenUrls;
		QString m_stationName;
		QString m_coverUrl;
		MetaDataList m_tracks;
		Urls m_urls;
		QString m_userAgent;
		std::shared_ptr<WebClientFactory> m_webClientFactory;
		Util::FileSystemPtr m_fileSystem;
		int m_timeout {0};
		bool m_stopped {false};
};

StreamParser::StreamParser(QObject* parent) :
	QObject(parent) {}

StreamParser::~StreamParser() = default;

void StreamParser::parse(const QString& name, const QStringList& urls)
{
	return parse(name, urls, {});
}

void StreamParser::parse(const QString& name, const QStringList& urls, const QString& userAgent)
{
	constexpr const auto Timeout = 5000;
	return parse(name, urls, userAgent, Timeout);
}

class StationParserFactoryImpl :
	public StationParserFactory
{
	public:
		StationParserFactoryImpl(const std::shared_ptr<WebClientFactory>& webClientFactory, QObject* parent) :
			m_webClientFactory {webClientFactory},
			m_parent {parent} {}

		~StationParserFactoryImpl() override = default;

		[[nodiscard]] StreamParser* createParser() const override
		{
			return new StreamParserImpl(m_webClientFactory, m_fileSystem, m_parent);
		}

	private:
		std::shared_ptr<WebClientFactory> m_webClientFactory;
		Util::FileSystemPtr m_fileSystem {Util::FileSystem::create()};
		QObject* m_parent;
};

std::shared_ptr<StationParserFactory>
StationParserFactory::createStationParserFactory(const std::shared_ptr<WebClientFactory>& webClientFactory,
                                                 QObject* parent)
{
	return std::make_shared<StationParserFactoryImpl>(webClientFactory, parent);
}

#include "StreamParser.moc"