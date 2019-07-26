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

#include "Golyr.h"
#include "ELyrics.h"
#include "LyricsKeeper.h"
#include "MetroLyrics.h"
#include "Musixmatch.h"
#include "OldieLyrics.h"
#include "Wikia.h"
#include "Songtexte.h"
#include "Genius.h"

#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QRegExp>
#include <QMap>

namespace Algorithm=Util::Algorithm;
using namespace Lyrics;

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

QString LookupThread::convert_to_regex(const QString& str) const
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
		QString url = server->search_address(artist, title);
		if(!url.isEmpty())
		{
			start_search(url);
		}
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

	m->server_list.push_back(new Wikia());
	m->server_list.push_back(new Musixmatch());
	m->server_list.push_back(new Songtexte());
	m->server_list.push_back(new Genius());
	m->server_list.push_back(new MetroLyrics());
	m->server_list.push_back(new OldieLyrics());
	m->server_list.push_back(new LyricsKeeper);
	m->server_list.push_back(new ELyrics());
	m->server_list.push_back(new Golyr());
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

QString LookupThread::parse_webpage(const QByteArray& raw, Server* server) const
{
	QString dst(raw);

	Server::StartEndTags tags = server->start_end_tag();
	for(const Server::StartEndTag& tag : tags)
	{
		QString start_tag = convert_to_regex(tag.first);
		if(start_tag.startsWith("<") && !start_tag.endsWith(">")){
			start_tag.append(".*>");
		}

		QString end_tag = convert_to_regex(tag.second);

		QString content;
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

		dst.replace("\n", "<br>");
		dst.replace("\\n", "<br>");
		dst.replace(QRegExp("<br\\s*/>"), "<br>");
		dst.replace("\\\"", "\"");

		QRegExp re_ptag("<p\\s.*>");
		re_ptag.setMinimal(true);
		dst.remove(re_ptag);
		dst.remove(QRegExp("</p>"));

		QRegExp re_comment("<!--.*-->");
		re_comment.setMinimal(true);
		dst.remove(re_comment);

		QRegExp re_linefeed("<br>\\s*<br>\\s*<br>");
		while(dst.contains(re_linefeed)) {
			dst.replace(re_linefeed, "<br><br>");
		}

		while(dst.startsWith("<br>")){
			dst = dst.right(dst.count() - 4);
		}

		int idx = dst.indexOf("<a");
		while(idx >= 0)
		{
			int idx2 = dst.indexOf("\">", idx);
			dst.remove(idx, idx2 - idx + 2);
			idx = dst.indexOf("<a");
		}

		if(dst.size() > 100){
			break;
		}
	}

	return dst.trimmed();
}



QString calc_server_url(Server* server, QString artist, QString song)
{
	artist = Server::apply_replacements(artist, server->replacements());
	song =  Server::apply_replacements(song, server->replacements());

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
