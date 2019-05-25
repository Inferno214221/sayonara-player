/* LyricLookupThread.cpp */

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
 * LyricLookupThread.cpp
 *
 *  Created on: May 21, 2011
 *      Author: Lucio Carreras
 */

#include "LyricLookup.h"
#include "LyricServer.h"

#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Utils.h"

#include <QStringList>
#include <QRegExp>
#include <QMap>

#include <algorithm>


struct LyricLookupThread::Private
{
	bool					has_error;
	QString					artist;
	QString					title;
	int						cur_server;
	QList<ServerTemplate>	server_list;
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



LyricLookupThread::LyricLookupThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<LyricLookupThread::Private>();

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

LyricLookupThread::~LyricLookupThread() {}

QString LyricLookupThread::convert_to_regex(const QString& str) const
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

QString LyricLookupThread::calc_server_url(QString artist, QString song)
{
	if(m->cur_server < 0 || m->cur_server >= m->server_list.size()){
		return "";
	}

	QMap<QString, QString> replacements = m->server_list[m->cur_server].replacements;

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

	QString url = m->server_list[m->cur_server].call_policy;
	url.replace("<SERVER>", m->server_list[m->cur_server].server_address);
	url.replace("<FIRST_ARTIST_LETTER>", QString(artist[0]).trimmed());
	url.replace("<ARTIST>", artist.trimmed());
	url.replace("<TITLE>", song.trimmed());

	if(m->server_list[m->cur_server].to_lower){
		return url.toLower();
	}

	return url;
}


void LyricLookupThread::run(const QString& artist, const QString& title, int server_idx)
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

	QString url = this->calc_server_url(m->artist, m->title);

	stop();
	m->current_awa = new AsyncWebAccess(this);
	connect(m->current_awa, &AsyncWebAccess::sig_finished, this, &LyricLookupThread::content_fetched);

	m->current_awa->run(url);
}


void LyricLookupThread::content_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	QString url = awa->url();
	m->current_awa = nullptr;

	m->lyric_header =
			"<b>" + m->artist + " - " +  m->title + " </b><br />" +
			m->server_list[m->cur_server].display_str + ": " + url;


	if(!awa->has_data() || awa->has_error())
	{
		m->final_wp = tr("Cannot fetch lyrics from %1").arg(awa->url());
		m->has_error = true;
		emit sig_finished();
		return;
	}

	m->final_wp = parse_webpage(awa->data(), m->server_list[m->cur_server]);

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

void LyricLookupThread::stop()
{
	if(m->current_awa){
		disconnect(m->current_awa, &AsyncWebAccess::sig_finished,
				   this, &LyricLookupThread::content_fetched);

		m->current_awa->stop();
	}
}

bool LyricLookupThread::has_error() const
{
	return m->has_error;
}

void LyricLookupThread::init_server_list()
{
	/*parse_xml();
	return;*/

	// motörhead
	// crosby, stills & nash
	// guns 'n' roses
	// AC/DC
	// the doors
	// the rolling stones
	// petr nalitch
	// eric burdon and the animals
	// Don't speak


	ServerTemplate wikia;
	wikia.display_str = "Wikia.com";
	wikia.server_address = QString("http://lyrics.wikia.com");
	wikia.addReplacement("'", "");
	wikia.addReplacement("ö", "o");
	wikia.addReplacement(" ", "_");
	wikia.addReplacement("!", "");
	wikia.addReplacement("&", "%26");
	wikia.call_policy = QString("<SERVER>/wiki/<ARTIST>:<TITLE>");
	wikia.start_end_tag.insert("<div class='lyricbox'>", "<!--");
	wikia.include_start_tag = false;
	wikia.include_end_tag = false;
	wikia.is_numeric = true;
	wikia.to_lower = false;
	wikia.error = "this page needs content";


	ServerTemplate oldieLyrics;
	oldieLyrics.display_str = "OldieLyrics.com";
	oldieLyrics.server_address = QString("http://www.oldielyrics.com/lyrics");
	oldieLyrics.addReplacement(" ", "_");
	oldieLyrics.addReplacement("(", "_");
	oldieLyrics.addReplacement(")", "_");
	oldieLyrics.addReplacement(".", "_");
	oldieLyrics.addReplacement("&", "_");
	oldieLyrics.addReplacement("'", "");
	oldieLyrics.addReplacement("__", "_");
	oldieLyrics.call_policy = QString("<SERVER>/<ARTIST>/<TITLE>.html");
	oldieLyrics.start_end_tag.insert("&#", "</div>");
	oldieLyrics.include_start_tag = true;
	oldieLyrics.include_end_tag = false;
	oldieLyrics.is_numeric = true;
	oldieLyrics.to_lower = true;
	oldieLyrics.error = QString("error 404");


	ServerTemplate lyricskeeper;
	lyricskeeper.display_str = "lyrics keeper";
	lyricskeeper.server_address = QString("http://lyrics-keeper.com");
	lyricskeeper.addReplacement("&", "");
	lyricskeeper.addReplacement(" ", "-");
	lyricskeeper.addReplacement("'", "-");
	lyricskeeper.addReplacement("--", "-");
	lyricskeeper.call_policy = QString("<SERVER>/en/<ARTIST>/<TITLE>.html");
	lyricskeeper.start_end_tag.insert("<div id=\"lyrics\">", "</div>");
	lyricskeeper.include_start_tag = false;
	lyricskeeper.include_end_tag = false;
	lyricskeeper.is_numeric = false;
	lyricskeeper.to_lower = true;
	lyricskeeper.error = QString("page cannot be found");


	ServerTemplate metrolyrics;
	metrolyrics.display_str = "MetroLyrics.com";
	metrolyrics.server_address = QString("http://www.metrolyrics.com");
	metrolyrics.addReplacement("&", "and");
	metrolyrics.addReplacement(" ", "-");
	metrolyrics.call_policy = QString("<SERVER>/<TITLE>-lyrics-<ARTIST>.html");
	//metrolyrics.start_end_tag.insert("<span class='line line-s' id='line_1'>", "</p>");
	metrolyrics.start_end_tag.insert ("<div id=\"lyrics-body-text\" class=\"js-lyric-text\">", "</div>");
	metrolyrics.include_start_tag = false;
	metrolyrics.include_end_tag = false;
	metrolyrics.is_numeric = false;
	metrolyrics.to_lower = true;
	metrolyrics.error = QString("404 page not found");


	ServerTemplate elyrics;
	elyrics.display_str = "eLyrics";
	elyrics.server_address= QString("http://www.elyrics.net/read");
	elyrics.addReplacement(" ", "-");
	elyrics.addReplacement("the ", "");
	elyrics.addReplacement("The ", "");
	elyrics.addReplacement("'", "_");
	elyrics.call_policy = QString("<SERVER>/<FIRST_ARTIST_LETTER>/<ARTIST>-lyrics/<TITLE>-lyrics.html");

	elyrics.start_end_tag.insert("lyrics</strong><br>", "</div>");
	elyrics.start_end_tag.insert("<div id='inlyr' style='font-size:14px;'>", "</div>");

	elyrics.include_start_tag = false;
	elyrics.include_end_tag = false;
	elyrics.is_numeric = false;
	elyrics.to_lower = true;
	elyrics.error = QString("Error 404");


	ServerTemplate golyr;
	golyr.display_str = "Golyr";
	golyr.server_address = "http://www.golyr.de";
	golyr.call_policy = "<SERVER>/<ARTIST>/songtext-<TITLE>";
	golyr.addReplacement("'", "-");
	golyr.addReplacement(" ", "-");
	golyr.addReplacement("(", "");
	golyr.addReplacement(")", "");
	golyr.start_end_tag.insert("<div id=\"lyrics\">", "</div> <div class=\"fads\"");
	golyr.include_end_tag = false;
	golyr.is_numeric = false;
	golyr.to_lower = false;
	golyr.error = "404 Not Found";

	ServerTemplate musixmatch;
	musixmatch.display_str = "Musixmatch";
	musixmatch.server_address = "https://www.musixmatch.com";
	musixmatch.call_policy = "<SERVER>/lyrics/<ARTIST>/<TITLE>";
	musixmatch.addReplacement(" ", "-");
	musixmatch.addReplacement("'", "");
	musixmatch.addReplacement("/", "-");
	musixmatch.addReplacement(".", "-");
	musixmatch.addReplacement("&", "-");
	musixmatch.addReplacement("--", "-");
	musixmatch.start_end_tag.insert("<div id=\"selectable-lyrics\"", "</span><span data-reactid");
	musixmatch.start_end_tag.insert("<p class=.*content", "</p>");
	musixmatch.start_end_tag.insert("\"body\":\"", "\",\"");

	musixmatch.include_end_tag = false;
	musixmatch.is_numeric = false;
	musixmatch.to_lower = false;
	musixmatch.error = "404 Not Found";

	m->server_list.push_back(wikia);
	m->server_list.push_back(musixmatch);
	m->server_list.push_back(metrolyrics);
	m->server_list.push_back(oldieLyrics);
	m->server_list.push_back(lyricskeeper);
	m->server_list.push_back(elyrics);
	m->server_list.push_back(golyr);


	/*sp_log(Log::Info) << "Servers: [";
	for(const ServerTemplate& t : m->server_list){
		t.print_json();
		sp_log(Log::Info) << ",";
	}
	sp_log(Log::Info) << "]";*/

}

QStringList LyricLookupThread::servers() const
{
	QStringList lst;
	for(const ServerTemplate& t : Util::AsConst(m->server_list))
	{
		lst << t.display_str;
	}

	return lst;
}

QString LyricLookupThread::lyric_header() const
{
	return m->lyric_header;
}

QString LyricLookupThread::lyric_data() const
{
	return m->final_wp;
}



QString LyricLookupThread::parse_webpage(const QByteArray& raw, const ServerTemplate& t) const
{
	QString dst(raw);

	for(auto it=t.start_end_tag.cbegin(); it != t.start_end_tag.cend(); it++)
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
		if(t.is_numeric) {
			QRegExp rx("&#(\\d+);|<br />|</span>|</p>");

			QStringList tmplist;
			int pos = 0;
			while ((pos = rx.indexIn(content, pos)) != -1) {
				QString str = rx.cap(1);

				pos += rx.matchedLength();
				if(str.size() == 0) {
					tmplist.push_back(word);
					word = "";
					tmplist.push_back("<br />");
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

		else{
			dst = content;
		}

		dst.replace("\n", "<br />");
		dst.replace("\\n", "<br />");

		if(dst.size() > 100){
			break;
		}
	}

	return dst;
}
