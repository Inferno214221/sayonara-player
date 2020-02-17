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

#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QRegExp>
#include <QMap>
#include <QFile>
#include <QDir>

namespace Algorithm=Util::Algorithm;
using namespace Lyrics;

static QString calcUrl(Server* server, const QString& url_template, QString artist, QString song);
static QString calcSearchUrl(Server* server, QString artist, QString song);
static QString calcServerUrl(Server* server, QString artist, QString song);

struct LookupThread::Private
{
	bool					hasError;
	QString					artist;
	QString					title;
	int						currentServerIndex;
	QList<Server*>			servers;
	QString					finalWp;
	QMap<QString, QString>  regexConversions;
	QString					lyricHeader;
	AsyncWebAccess*			currentAwa=nullptr;

	Private()
	{
		currentServerIndex = -1;
		hasError = false;
	}
};


LookupThread::LookupThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<LookupThread::Private>();

	initServerList();

	m->currentServerIndex = 0;
	m->finalWp.clear();

	m->regexConversions.insert("$", "\\$");
	m->regexConversions.insert("*", "\\*");
	m->regexConversions.insert("+", "\\+");
	m->regexConversions.insert("?", "\\?");
	m->regexConversions.insert("[", "\\[");
	m->regexConversions.insert("]", "\\]");
	m->regexConversions.insert("(", "\\(");
	m->regexConversions.insert(")", "\\)");
	m->regexConversions.insert("{", "\\{");
	m->regexConversions.insert("}", "\\}");
	m->regexConversions.insert("^", "\\^");
	m->regexConversions.insert("|", "\\|");
	m->regexConversions.insert(".", "\\.");
}

LookupThread::~LookupThread()=default;

void LookupThread::run(const QString& artist, const QString& title, int server_idx)
{
	m->artist = artist;
	m->title = title;

	m->currentServerIndex = std::max(0, server_idx);
	m->currentServerIndex = std::min(server_idx, m->servers.size() - 1);

	if(m->artist.isEmpty() && m->title.isEmpty()) {
		m->finalWp = "No track selected";
		return;
	}

	m->finalWp.clear();

	Server* server = m->servers[m->currentServerIndex];
	if(server->canFetchDirectly())
	{
		QString url = calcServerUrl(server, artist, title);
		callWebsite(url);
	}

	else if(server->canSearch())
	{
		QString url = calcSearchUrl(server, artist, title);
		startSearch(url);
	}

	else {
		spLog(Log::Warning, this) << "Search server " << server->name() << " cannot do anything at all!";
		emit sigFinished();
	}
}

void LookupThread::startSearch(const QString& url)
{
	spLog(Log::Debug, this) << "Search Lyrics from " << url;

	AsyncWebAccess* awa = new AsyncWebAccess(this, QByteArray(), AsyncWebAccess::Behavior::AsBrowser);
	connect(awa, &AsyncWebAccess::sigFinished, this, &LookupThread::searchFinished);
	awa->run(url);
}


void LookupThread::searchFinished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());

	QString url;
	{ // extract url out of the search result
		Server* server = m->servers[m->currentServerIndex];
		QByteArray data = awa->data();
		QString website = QString::fromLocal8Bit(data);
		QRegExp re(server->searchResultRegex());
		re.setMinimal(true);
		if(re.indexIn(website) > 0)
		{
			QString parsed_url = re.cap(1);
			url = server->searchResultUrlTemplate();
			url.replace("<SERVER>", server->address());
			url.replace("<SEARCH_RESULT_CAPTION>", parsed_url);
		}
	}

	if(!url.isEmpty())
	{
		callWebsite(url);
	}

	else
	{
		spLog(Log::Debug, this) << "Search Lyrics not successful ";
		m->finalWp = tr("Cannot fetch lyrics from %1").arg(awa->url());
		m->hasError = true;
		emit sigFinished();
	}

	awa->deleteLater();
}


void LookupThread::callWebsite(const QString& url)
{
	stop();

	spLog(Log::Debug, this) << "Fetch Lyrics from " << url;
	m->currentAwa = new AsyncWebAccess(this);
	connect(m->currentAwa, &AsyncWebAccess::sigFinished, this, &LookupThread::contentFetched);
	m->currentAwa->run(url);
}


void LookupThread::contentFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QString url = awa->url();

	Server* server = m->servers[m->currentServerIndex];

	m->currentAwa = nullptr;
	m->lyricHeader =
			"<b>" + m->artist + " - " +  m->title + " </b><br />" +
			server->name() + ": " + url;


	if(!awa->hasData() || awa->hasError())
	{
		m->finalWp = tr("Cannot fetch lyrics from %1").arg(awa->url());
		m->hasError = true;
		emit sigFinished();
		return;
	}

	m->finalWp = Lyrics::WebpageParser::parseWebpage(awa->data(), m->regexConversions, server);

	if ( m->finalWp.isEmpty() )
	{
		m->finalWp = tr("No lyrics found") + "<br />" + url;
		m->hasError = true;

		emit sigFinished();

		return;
	}

	m->hasError = false;
	emit sigFinished();
}


void LookupThread::stop()
{
	if(m->currentAwa)
	{
		disconnect(m->currentAwa, &AsyncWebAccess::sigFinished,
				   this, &LookupThread::contentFetched);

		m->currentAwa->stop();
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

	QList<Server*> servers = Lyrics::ServerJsonReader::parseJsonFile(":/lyrics/lyrics.json");
	for(Server* server : servers)
	{
		addServer(server);
	}

	initCustomServers();
}

void LookupThread::initCustomServers()
{
	const QString lyrics_path = Util::sayonaraPath("lyrics");
	const QDir dir(lyrics_path);
	const QStringList json_files = dir.entryList(QStringList{"*.json"}, QDir::Files);

	for(QString json_file : json_files)
	{
		json_file.prepend(lyrics_path + "/");
		QList<Server*> servers = Lyrics::ServerJsonReader::parseJsonFile(json_file);
		for(Server* server : servers)
		{
			addServer(server);
		}
	}
}

void LookupThread::addServer(Server* server)
{
	if(!server) {
		return;
	}

	if(!server->canFetchDirectly() && !server->canSearch()){
		return;
	}

	const QString name = server->name();
	bool found = false;
	for(int i=0; i<m->servers.size(); i++)
	{
		Server* s = m->servers[i];
		if(s->name() == name)
		{
			delete s;

			m->servers.removeAt(i);
			m->servers.insert(i, server);

			found = true;
			break;
		}
	}

	if(!found)
	{
		m->servers << server;
	}
}

QStringList LookupThread::servers() const
{
	QStringList lst;
	for(Server* server : Algorithm::AsConst(m->servers))
	{
		lst << server->name();
	}

	return lst;
}

QString LookupThread::lyricHeader() const
{
	return m->lyricHeader;
}

QString LookupThread::lyricData() const
{
	return m->finalWp;
}


QString calcUrl(Server* server, const QString& url_template, QString artist, QString song)
{
	artist = Server::applyReplacements(artist, server->replacements());
	song =  Server::applyReplacements(song, server->replacements());

	QString url = url_template;
	url.replace("<SERVER>", server->address());
	url.replace("<FIRST_ARTIST_LETTER>", QString(artist[0]).trimmed());
	url.replace("<ARTIST>", artist.trimmed());
	url.replace("<TITLE>", song.trimmed());

	if(server->isLowercase()){
		return url.toLower();
	}

	return url;
}

QString calcSearchUrl(Server* server, QString artist, QString song)
{
	return calcUrl(server, server->searchUrlTemplate(), artist, song);
}

QString calcServerUrl(Server* server, QString artist, QString song)
{
	return calcUrl(server, server->directUrlTemplate(), artist, song);
}
