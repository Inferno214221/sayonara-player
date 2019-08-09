/* LookupThread.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
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

static QString calc_url(Server* server, const QString& url_template, QString artist, QString song);
static QString calc_search_url(Server* server, QString artist, QString song);
static QString calc_server_url(Server* server, QString artist, QString song);

struct LookupThread::Private
{
	bool					has_error;
	QString					artist;
	QString					title;
	int						cur_server;
	QList<Server*>			server_list;
	QString					final_wp;
	QMap<QString, QString>  regex_conversions;
	QString					lyric_header;
	AsyncWebAccess*			current_awa=nullptr;

	Private()
	{
		cur_server = -1;
		has_error = false;
	}
};


LookupThread::LookupThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<LookupThread::Private>();

	init_server_list();

	m->cur_server = 0;
	m->final_wp.clear();

	m->regex_conversions.insert("$", "\\$");
	m->regex_conversions.insert("*", "\\*");
	m->regex_conversions.insert("+", "\\+");
	m->regex_conversions.insert("?", "\\?");
	m->regex_conversions.insert("[", "\\[");
	m->regex_conversions.insert("]", "\\]");
	m->regex_conversions.insert("(", "\\(");
	m->regex_conversions.insert(")", "\\)");
	m->regex_conversions.insert("{", "\\{");
	m->regex_conversions.insert("}", "\\}");
	m->regex_conversions.insert("^", "\\^");
	m->regex_conversions.insert("|", "\\|");
	m->regex_conversions.insert(".", "\\.");
}

LookupThread::~LookupThread()=default;


void LookupThread::run(const QString& artist, const QString& title, int server_idx)
{
	m->artist = artist;
	m->title = title;

	m->cur_server = std::max(0, server_idx);
	m->cur_server = std::min(server_idx, m->server_list.size() - 1);

	if(m->artist.isEmpty() && m->title.isEmpty()) {
		m->final_wp = "No track selected";
		return;
	}

	m->final_wp.clear();

	Server* server = m->server_list[m->cur_server];
	if(server->can_fetch_directly())
	{
		QString url = calc_server_url(server, artist, title);
		call_website(url);
	}

	else if(server->can_search())
	{
		QString url = calc_search_url(server, artist, title);
		start_search(url);
	}

	else {
		sp_log(Log::Warning, this) << "Search server " << server->name() << " cannot do anything at all!";
		emit sig_finished();
	}
}

void LookupThread::start_search(const QString& url)
{
	sp_log(Log::Debug, this) << "Search Lyrics from " << url;

	AsyncWebAccess* awa = new AsyncWebAccess(this, QByteArray(), AsyncWebAccess::Behavior::AsBrowser);
	connect(awa, &AsyncWebAccess::sig_finished, this, &LookupThread::search_finished);
	awa->run(url);
}


void LookupThread::search_finished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());

	QString url;
	{ // extract url out of the search result
		Server* server = m->server_list[m->cur_server];
		QByteArray data = awa->data();
		QString website = QString::fromLocal8Bit(data);
		QRegExp re(server->search_result_regex());
		re.setMinimal(true);
		if(re.indexIn(website) > 0)
		{
			QString parsed_url = re.cap(1);
			url = server->search_result_url_template();
			url.replace("<SERVER>", server->address());
			url.replace("<SEARCH_RESULT_CAPTION>", parsed_url);
		}
	}

	if(!url.isEmpty())
	{
		call_website(url);
	}

	else
	{
		sp_log(Log::Debug, this) << "Search Lyrics not successful ";
		m->final_wp = tr("Cannot fetch lyrics from %1").arg(awa->url());
		m->has_error = true;
		emit sig_finished();
	}

	awa->deleteLater();
}


void LookupThread::call_website(const QString& url)
{
	stop();

	sp_log(Log::Debug, this) << "Fetch Lyrics from " << url;
	m->current_awa = new AsyncWebAccess(this);
	connect(m->current_awa, &AsyncWebAccess::sig_finished, this, &LookupThread::content_fetched);
	m->current_awa->run(url);
}


void LookupThread::content_fetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QString url = awa->url();

	Server* server = m->server_list[m->cur_server];

	m->current_awa = nullptr;
	m->lyric_header =
			"<b>" + m->artist + " - " +  m->title + " </b><br />" +
			server->name() + ": " + url;


	if(!awa->has_data() || awa->has_error())
	{
		m->final_wp = tr("Cannot fetch lyrics from %1").arg(awa->url());
		m->has_error = true;
		emit sig_finished();
		return;
	}

	m->final_wp = Lyrics::WebpageParser::parse_webpage(awa->data(), m->regex_conversions, server);

	if ( m->final_wp.isEmpty() )
	{
		m->final_wp = tr("No lyrics found") + "<br />" + url;
		m->has_error = true;

		emit sig_finished();

		return;
	}

	m->has_error = false;
	emit sig_finished();
}


void LookupThread::stop()
{
	if(m->current_awa)
	{
		disconnect(m->current_awa, &AsyncWebAccess::sig_finished,
				   this, &LookupThread::content_fetched);

		m->current_awa->stop();
	}
}

bool LookupThread::has_error() const
{
	return m->has_error;
}


void LookupThread::init_server_list()
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

	QList<Server*> servers = Lyrics::ServerJsonReader::parse_json_file(":/lyrics/lyrics.json");
	for(Server* server : servers)
	{
		add_server(server);
	}

	init_custom_servers();
}

void LookupThread::init_custom_servers()
{
	QString lyrics_path = Util::sayonara_path("lyrics");
	QDir dir(lyrics_path);
	QStringList json_files = dir.entryList(QStringList{"*.json"}, QDir::Files);

	for(QString json_file : json_files)
	{
		json_file.prepend(lyrics_path + "/");
		QList<Server*> servers = Lyrics::ServerJsonReader::parse_json_file(json_file);
		for(Server* server : servers)
		{
			add_server(server);
		}
	}
}

void LookupThread::add_server(Server* server)
{
	if(!server) {
		return;
	}

	if(!server->can_fetch_directly() && !server->can_search()){
		return;
	}

	QString name = server->name();
	bool found = false;
	for(int i=0; i<m->server_list.size(); i++)
	{
		Server* s = m->server_list[i];
		if(s->name() == name)
		{
			delete s;

			m->server_list.removeAt(i);
			m->server_list.insert(i, server);

			found = true;
			break;
		}
	}

	if(!found)
	{
		m->server_list << server;
	}
}

QStringList LookupThread::servers() const
{
	QStringList lst;
	for(Server* server : Algorithm::AsConst(m->server_list))
	{
		lst << server->name();
	}

	return lst;
}

QString LookupThread::lyric_header() const
{
	return m->lyric_header;
}

QString LookupThread::lyric_data() const
{
	return m->final_wp;
}


QString calc_url(Server* server, const QString& url_template, QString artist, QString song)
{
	artist = Server::apply_replacements(artist, server->replacements());
	song =  Server::apply_replacements(song, server->replacements());

	QString url = url_template;
	url.replace("<SERVER>", server->address());
	url.replace("<FIRST_ARTIST_LETTER>", QString(artist[0]).trimmed());
	url.replace("<ARTIST>", artist.trimmed());
	url.replace("<TITLE>", song.trimmed());

	if(server->is_lowercase()){
		return url.toLower();
	}

	return url;
}

QString calc_search_url(Server* server, QString artist, QString song)
{
	return calc_url(server, server->search_url_template(), artist, song);
}

QString calc_server_url(Server* server, QString artist, QString song)
{
	return calc_url(server, server->direct_url_template(), artist, song);
}
