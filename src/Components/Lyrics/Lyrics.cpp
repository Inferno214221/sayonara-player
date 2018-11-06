/* Lyrics.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "Lyrics.h"
#include "LyricLookup.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>

struct Lyrics::Private
{
	QStringList servers;
	MetaData md;
	QString artist;
	QString title;
	QString lyrics;
	QString lyric_header;
	QString lyric_tag_content;

	bool is_valid;

	Private()
	{
		is_valid = false;

		LyricLookupThread* lyric_thread = new LyricLookupThread();
		servers = lyric_thread->servers();
		delete lyric_thread;
	}

	void guess_artist_and_title();
};

Lyrics::Lyrics(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

Lyrics::~Lyrics() {}

bool Lyrics::fetch_lyrics(const QString& artist, const QString& title, int server_index)
{
	if(artist.isEmpty() || title.isEmpty()) {
		return false;
	}

	if(server_index < 0 || server_index >= m->servers.size()) {
		return false;
	}

	LyricLookupThread* lyric_thread = new LyricLookupThread(this);
	connect(lyric_thread, &LyricLookupThread::sig_finished, this, &Lyrics::lyrics_fetched);

	lyric_thread->run(artist, title, server_index);
	return true;
}

bool Lyrics::save_lyrics(const QString& plain_text)
{
	if(plain_text.isEmpty()){
		return false;
	}

	if(m->md.filepath().isEmpty()){
		return false;
	}

	bool success = Tagging::Utils::write_lyrics(m->md, plain_text);
	if(success){
		m->is_valid = true;
		m->lyric_tag_content = plain_text;
	}

	return success;
}

QStringList Lyrics::servers() const
{
	return m->servers;
}

void Lyrics::set_metadata(const MetaData& md)
{
	m->md = md;
	m->guess_artist_and_title();

	bool has_lyrics = Tagging::Utils::extract_lyrics(md, m->lyric_tag_content);
	if(!has_lyrics){
		sp_log(Log::Debug, this) << "Could not find lyrics in " << md.filepath();
	}

	else {
		sp_log(Log::Debug, this) << "Lyrics found in " << md.filepath();
	}
}

QString Lyrics::artist() const
{
	return m->artist;
}

QString Lyrics::title() const
{
	return m->title;
}

QString Lyrics::lyric_header() const
{
	return m->lyric_header;
}

QString Lyrics::local_lyric_header() const
{
	return "<b>" + artist() + " - " + title() + "</b>";
}

QString Lyrics::lyrics() const
{
	return m->lyrics.trimmed();
}

QString Lyrics::local_lyrics() const
{
	if(is_lyric_tag_available()){
		return m->lyric_tag_content.trimmed();
	}

	return QString();
}

bool Lyrics::is_lyric_valid() const
{
	return m->is_valid;
}

bool Lyrics::is_lyric_tag_available() const
{
	return (!m->lyric_tag_content.isEmpty());
}

bool Lyrics::is_lyric_tag_supported() const
{
	return Tagging::Utils::is_lyrics_supported(m->md.filepath());
}

void Lyrics::lyrics_fetched()
{
	LyricLookupThread* lyric_thread = static_cast<LyricLookupThread*>(sender());

	m->lyrics = lyric_thread->lyric_data();
	m->lyric_header = lyric_thread->lyric_header();
	m->is_valid = (!lyric_thread->has_error());

	lyric_thread->deleteLater();

	emit sig_lyrics_fetched();
}


void Lyrics::Private::guess_artist_and_title()
{
	bool guessed = false;

	if(	md.radio_mode() == RadioMode::Station &&
		md.artist().contains("://"))
	{
		if(md.title().contains("-")){
			QStringList lst = md.title().split("-");
			artist = lst.takeFirst().trimmed();
			title = lst.join("-").trimmed();
			guessed = true;
		}

		else if(md.title().contains(":")){
			QStringList lst = md.title().split(":");
			artist = lst.takeFirst().trimmed();
			title = lst.join(":").trimmed();
			guessed = true;
		}
	}

	if(guessed == false) {
		if(!md.artist().isEmpty()) {
			artist = md.artist();
			title = md.title();
		}

		else if(!md.album_artist().isEmpty()) {
			artist = md.album_artist();
			title = md.title();
		}

		else {
			artist = md.artist();
			title = md.title();
		}
	}
}

