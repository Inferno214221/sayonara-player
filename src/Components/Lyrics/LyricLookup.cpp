/* Lyrics::LookupThread.cpp */

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
 * Lyrics::LookupThread.cpp
 *
 *  Created on: May 21, 2011
 *      Author: Lucio Carreras
 */

#include "LyricLookup.h"
#include "LyricServer.h"

#include "Golyr.h"
#include "ELyrics.h"
#include "LyricsKeeper.h"
#include "MetroLyrics.h"
#include "Musixmatch.h"
#include "OldieLyrics.h"
#include "Wikia.h"
#include "Songtexte.h"

#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QRegExp>
#include <QMap>

namespace Algorithm=Util::Algorithm;
using Lyrics::Server;

struct Lyrics::LookupThread::Private
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


Lyrics::LookupThread::LookupThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Lyrics::LookupThread::Private>();

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

Lyrics::LookupThread::~LookupThread()=default;

QString Lyrics::LookupThread::convert_to_regex(const QString& str) const
{
	QString ret = str;

	const QList<QString> keys = m->regex_conversions.keys();
	for(const QString& key : keys)
	{
		ret.replace(key, m->regex_conversions.value(key));
	}

	ret.replace(" ", "\\s+");

	return ret;
}

QString Lyrics::LookupThread::calc_server_url(QString artist, QString song)
{
	if(m->cur_server < 0 || m->cur_server >= m->server_list.size()){
		return "";
	}

	Server* server = m->server_list[m->cur_server];
	QMap<QString, QString> replacements = server->replacements();

	for(int i=0; i<2; i++)
	{
		for(auto it=replacements.cbegin(); it != replacements.cend(); it++)
		{
			const QString key = it.key();
			while(artist.indexOf(key) >= 0){
				artist.replace(key, it.value());
			}

			while(song.indexOf(key) >= 0){
				song.replace(key, it.value());
			}
		}
	}

	QString url = server->call_policy();
	url.replace("<SERVER>", server->address());
	url.replace("<FIRST_ARTIST_LETTER>", QString(artist[0]).trimmed());
	url.replace("<ARTIST>", artist.trimmed());
	url.replace("<TITLE>", song.trimmed());

	if(server->is_lowercase()){
		return url.toLower();
	}

	return url;
}


void Lyrics::LookupThread::run(const QString& artist, const QString& title, int server_idx)
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

	Lyrics::Server* server = m->server_list[m->cur_server];
	if(server->can_fetch_directly())
	{
		QString url = calc_server_url(m->artist, m->title);
		call_website(url);
	}

	else if(server->can_search())
	{
		QString url = server->search_address(artist, title);
		if(!url.isEmpty())
		{
			start_search(url);
		}
	}
}


void Lyrics::LookupThread::start_search(const QString& url)
{
	sp_log(Log::Debug, this) << "Search Lyrics from " << url;

	AsyncWebAccess* awa = new AsyncWebAccess(this, QByteArray(), AsyncWebAccess::Behavior::AsBrowser);
	connect(awa, &AsyncWebAccess::sig_finished, this, &Lyrics::LookupThread::search_finished);
	awa->run(url);
}


void Lyrics::LookupThread::search_finished()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	Server* server = m->server_list[m->cur_server];
	QString original_url = awa->url();
	QByteArray data = awa->data();
	awa->deleteLater();

	QString url = server->parse_search_result(QString::fromLocal8Bit(data));
	if(!url.isEmpty())
	{
		call_website(url);
	}

	else
	{
		sp_log(Log::Debug, this) << "Search Lyrics not successful ";
		m->final_wp = tr("Cannot fetch lyrics from %1").arg(original_url);
		m->has_error = true;
		emit sig_finished();
	}
}


void Lyrics::LookupThread::call_website(const QString& url)
{
	stop();

	sp_log(Log::Debug, this) << "Fetch Lyrics from " << url;
	m->current_awa = new AsyncWebAccess(this);
	connect(m->current_awa, &AsyncWebAccess::sig_finished, this, &Lyrics::LookupThread::content_fetched);
	m->current_awa->run(url);
}


void Lyrics::LookupThread::content_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
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

	m->final_wp = parse_webpage(awa->data(), server);

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


void Lyrics::LookupThread::stop()
{
	if(m->current_awa)
	{
		disconnect(m->current_awa, &AsyncWebAccess::sig_finished,
				   this, &Lyrics::LookupThread::content_fetched);

		m->current_awa->stop();
	}
}

bool Lyrics::LookupThread::has_error() const
{
	return m->has_error;
}

void Lyrics::LookupThread::init_server_list()
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

	m->server_list.push_back(new Lyrics::Wikia());
	m->server_list.push_back(new Lyrics::Songtexte());
	m->server_list.push_back(new Lyrics::Musixmatch());
	m->server_list.push_back(new Lyrics::MetroLyrics());
	m->server_list.push_back(new Lyrics::OldieLyrics());
	m->server_list.push_back(new Lyrics::LyricsKeeper);
	m->server_list.push_back(new Lyrics::ELyrics());
	m->server_list.push_back(new Lyrics::Golyr());
}

QStringList Lyrics::LookupThread::servers() const
{
	QStringList lst;
	for(Lyrics::Server* server : Algorithm::AsConst(m->server_list))
	{
		lst << server->name();
	}

	return lst;
}

QString Lyrics::LookupThread::lyric_header() const
{
	return m->lyric_header;
}

QString Lyrics::LookupThread::lyric_data() const
{
	return m->final_wp;
}

QString Lyrics::LookupThread::parse_webpage(const QByteArray& raw, Lyrics::Server* server) const
{
	QString dst(raw);

	QMap<QString, QString> tag_map = server->start_end_tag();
	for(auto it=tag_map.begin(); it != tag_map.end(); it++)
	{
		QString content;

		QString start_tag = it.key();
		QString end_tag = it.value();

		start_tag = convert_to_regex(start_tag);
		if(start_tag.startsWith("<") && !start_tag.endsWith(">")){
			start_tag.append(".*>");
		}

		end_tag = convert_to_regex(end_tag);

		QRegExp regex;
		regex.setMinimal(true);
		regex.setPattern(start_tag + "(.+)" + end_tag);
		if(regex.indexIn(dst) != -1){
			content  = regex.cap(1);
		}

		if(content.isEmpty()){
			continue;
		}

		QRegExp re_script;
		re_script.setPattern("<script.+</script>");
		re_script.setMinimal(true);
		while(re_script.indexIn(content) != -1){
			content.replace(re_script, "");
		}

		QString word;
		if(server->is_numeric())
		{
			QRegExp rx("&#(\\d+);|<br />|</span>|</p>");

			QStringList tmplist;
			int pos = 0;
			while ((pos = rx.indexIn(content, pos)) != -1)
			{
				QString str = rx.cap(1);

				pos += rx.matchedLength();
				if(str.size() == 0)
				{
					tmplist.push_back(word);
					word = "";
					tmplist.push_back("<br>");
				}

				else{
					word.append(QChar(str.toInt()));
				}
			}

			dst = "";

			for(const QString& str : tmplist) {
				dst.append(str);
			}
		}

		else {
			dst = content;
		}

		dst.replace("\n", "<br />");
		dst.replace("\\n", "<br />");

		if(dst.size() > 100){
			break;
		}
	}

	return dst.trimmed();
}
