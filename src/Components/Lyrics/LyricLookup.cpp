/* LookupThread.cpp */

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

/*
 * LookupThread.cpp
 *
 *  Created on: May 21, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "LyricLookup.h"
#include "LyricServer.h"
#include "LyricWebpageParser.h"
#include "LyricServerJsonWriter.h"

#include "Utils/WebAccess/WebClientImpl.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"

#include <QStringList>
#include <QRegExp>
#include <QMap>
#include <QFile>
#include <QDir>

using namespace Lyrics;

namespace
{
	QString extractUrlFromSearchResults(Server* server, const QString& website)
	{
		if(website.isEmpty())
		{
			return {};
		}

		QString url;

		auto re = QRegExp(server->searchResultRegex());
		re.setMinimal(true);
		if(re.indexIn(website) > 0)
		{
			const auto parsedUrl = re.cap(1);
			url = server->searchResultUrlTemplate();
			url.replace("<SERVER>", server->address());
			url.replace("<SEARCH_RESULT_CAPTION>", parsedUrl);
		}

		return url;
	}

	void addServer(Server* server, QList<Server*>& servers)
	{
		if(!server || (!server->canFetchDirectly() && !server->canSearch()))
		{
			return;
		}

		const auto name = server->name();
		const auto index = Util::Algorithm::indexOf(servers, [&](auto* s) {
			return (s->name() == name);
		});

		const auto found = (index >= 0);
		if(found)
		{
			servers.replace(index, server);
		}

		else
		{
			servers << server;
		}
	}

	QString calcUrl(Server* server, const QString& urlTemplate, const QString& artist, const QString& song)
	{
		const auto replacedArtist = Server::applyReplacements(artist, server->replacements());
		const auto replacedSong = Server::applyReplacements(song, server->replacements());

		auto url = urlTemplate;
		url.replace("<SERVER>", server->address());
		url.replace("<FIRST_ARTIST_LETTER>", QString(replacedArtist).trimmed());
		url.replace("<ARTIST>", replacedArtist.trimmed());
		url.replace("<TITLE>", replacedSong.trimmed());

		return server->isLowercase()
		       ? url.toLower()
		       : url;
	}

	QString calcSearchUrl(Server* server, const QString& artist, const QString& song)
	{
		return calcUrl(server, server->searchUrlTemplate(), artist, song);
	}

	QString calcServerUrl(Server* server, const QString& artist, const QString& song)
	{
		return calcUrl(server, server->directUrlTemplate(), artist, song);
	}

	QString getLyricHeader(const QString& artist, const QString& title, const QString& serverName, const QString& url)
	{
		return QString("<b>%1 - %2</b><br>%3: %4")
			.arg(artist)
			.arg(title)
			.arg(serverName)
			.arg(url);
	}

	QMap<QString, QString> getRegexConversions()
	{
		return {{"$", "\\$"},
		        {"*", "\\*"},
		        {"+", "\\+"},
		        {"?", "\\?"},
		        {"[", "\\["},
		        {"]", "\\]"},
		        {"(", "\\("},
		        {")", "\\)"},
		        {"{", "\\{"},
		        {"}", "\\}"},
		        {"^", "\\^"},
		        {"|", "\\|"},
		        {".", "\\."}};
	}
}

struct LookupThread::Private
{
	QString artist;
	QString title;

	QList<Server*> servers;
	QString lyricsData;
	QString lyricHeader;

	WebClient* currentWebClient {nullptr};
	int currentServerIndex {-1};
	bool hasError {false};
};

LookupThread::LookupThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<LookupThread::Private>();

	initServerList();
}

LookupThread::~LookupThread() = default;

void LookupThread::run(const QString& artist, const QString& title, int serverIndex)
{
	m->artist = artist;
	m->title = title;

	m->currentServerIndex = std::max(0, serverIndex);
	m->currentServerIndex = std::min(serverIndex, m->servers.size() - 1);

	if(m->artist.isEmpty() && m->title.isEmpty())
	{
		m->lyricsData = "No track selected";
		return;
	}

	m->lyricsData.clear();

	auto* server = m->servers[m->currentServerIndex];
	if(server->canFetchDirectly())
	{
		const auto url = calcServerUrl(server, artist, title);
		callWebsite(url);
	}

	else if(server->canSearch())
	{
		const auto url = calcSearchUrl(server, artist, title);
		startSearch(url);
	}

	else
	{
		spLog(Log::Warning, this) << "Search server " << server->name() << " cannot do anything at all!";
		emit sigFinished();
	}
}

void LookupThread::startSearch(const QString& url)
{
	spLog(Log::Debug, this) << "Search Lyrics from " << url;

	auto* webClient = new WebClientImpl(this);
	connect(webClient, &WebClient::sigFinished, this, &LookupThread::searchFinished);
	webClient->run(url);
}

void LookupThread::searchFinished()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());

	auto* server = m->servers[m->currentServerIndex];

	const auto data = webClient->data();
	const auto url = extractUrlFromSearchResults(server, QString::fromLocal8Bit(data));

	if(!url.isEmpty())
	{
		callWebsite(url);
	}

	else
	{
		spLog(Log::Debug, this) << "Search Lyrics not successful ";
		m->lyricsData = tr("Cannot fetch lyrics from %1").arg(webClient->url());
		m->hasError = true;
		emit sigFinished();
	}

	webClient->deleteLater();
}

void LookupThread::callWebsite(const QString& url)
{
	stop();

	spLog(Log::Debug, this) << "Fetch Lyrics from " << url;

	m->currentWebClient = new WebClientImpl(this);
	connect(m->currentWebClient, &WebClient::sigFinished, this, &LookupThread::contentFetched);
	m->currentWebClient->run(url);
}

void LookupThread::contentFetched()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());
	auto* server = m->servers[m->currentServerIndex];

	m->currentWebClient = nullptr;
	m->lyricHeader = getLyricHeader(m->artist, m->title, server->name(), webClient->url());

	m->hasError = (!webClient->hasData() || webClient->hasError());
	if(m->hasError)
	{
		m->lyricsData = tr("Cannot fetch lyrics from %1").arg(webClient->url());
	}

	else if(webClient->data().isEmpty())
	{
		m->lyricsData = tr("No lyrics found") + "<br />" + webClient->url();
	}

	else
	{
		m->lyricsData = Lyrics::WebpageParser::parseWebpage(webClient->data(), getRegexConversions(), server);
	}

	webClient->deleteLater();
	emit sigFinished();
}

void LookupThread::stop()
{
	if(m->currentWebClient)
	{
		disconnect(m->currentWebClient, &WebClient::sigFinished, this, &LookupThread::contentFetched);
		m->currentWebClient->stop();
	}
}

bool LookupThread::hasError() const
{
	return m->hasError;
}

void LookupThread::initServerList()
{
	// motörhead
	// crosby, stills & nash
	// guns 'n' roses
	// AC/DC
	// the doors
	// the rolling stones
	// petr nalitch
	// eric burdon and the animals
	// Don't speak

	const auto servers = Lyrics::ServerJsonReader::parseJsonFile(":/lyrics/lyrics.json");
	for(auto* server: servers)
	{
		addServer(server, m->servers);
	}

	initCustomServers();

	m->currentServerIndex = 0;
}

void LookupThread::initCustomServers()
{
	const auto lyricsPath = Util::lyricsPath();
	const auto dir = QDir(lyricsPath);
	const auto jsonFiles = dir.entryList(QStringList {"*.json"}, QDir::Files);

	for(auto jsonFile: jsonFiles)
	{
		jsonFile.prepend(lyricsPath + "/");
		auto servers = Lyrics::ServerJsonReader::parseJsonFile(jsonFile);
		for(auto* server: servers)
		{
			addServer(server, m->servers);
		}
	}
}

QStringList LookupThread::servers() const
{
	QStringList serverName;
	Util::Algorithm::transform(m->servers, serverName, [](const auto* server) {
		return server->name();
	});

	return serverName;
}

QString LookupThread::lyricHeader() const
{
	return m->lyricHeader;
}

QString LookupThread::lyricData() const
{
	return m->lyricsData;
}
