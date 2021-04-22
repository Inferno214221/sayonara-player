/* Lyrics.cpp */

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

#include "Lyrics.h"
#include "LyricLookup.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/TaggingLyrics.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>

using LyricsImpl=::Lyrics::Lyrics;

struct LyricsImpl::Private
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

		auto* lyric_thread = new ::Lyrics::LookupThread();
		servers = lyric_thread->servers();
		delete lyric_thread;
	}

	void guess_artist_and_title();
};

LyricsImpl::Lyrics(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

LyricsImpl::~Lyrics() {}

bool LyricsImpl::fetchLyrics(const QString& artist, const QString& title, int server_index)
{
	if(artist.isEmpty() || title.isEmpty()) {
		return false;
	}

	if(server_index < 0 || server_index >= m->servers.size()) {
		return false;
	}

	auto* lyric_thread = new ::Lyrics::LookupThread(this);
	connect(lyric_thread, &::Lyrics::LookupThread::sigFinished, this, &LyricsImpl::lyricsFetched);

	lyric_thread->run(artist, title, server_index);
	return true;
}

bool LyricsImpl::saveLyrics(const QString& plain_text)
{
	if(plain_text.isEmpty()){
		return false;
	}

	if(m->md.filepath().isEmpty()){
		return false;
	}

	bool success = Tagging::writeLyrics(m->md, plain_text);
	if(success){
		m->is_valid = true;
		m->lyric_tag_content = plain_text;
	}

	return success;
}

QStringList LyricsImpl::servers() const
{
	return m->servers;
}

void LyricsImpl::setMetadata(const MetaData& md)
{
	m->md = md;
	m->guess_artist_and_title();

	bool has_lyrics = Tagging::extractLyrics(md, m->lyric_tag_content);
	if(!has_lyrics){
		spLog(Log::Debug, this) << "Could not find lyrics in " << md.filepath();
	}

	else {
		spLog(Log::Debug, this) << "Lyrics found in " << md.filepath();
	}
}

QString LyricsImpl::artist() const
{
	return m->artist;
}

QString LyricsImpl::title() const
{
	return m->title;
}

QString LyricsImpl::lyricHeader() const
{
	return m->lyric_header;
}

QString LyricsImpl::localLyricHeader() const
{
	return "<b>" + artist() + " - " + title() + "</b>";
}

QString LyricsImpl::lyrics() const
{
	return m->lyrics.trimmed();
}

QString LyricsImpl::localLyrics() const
{
	if(isLyricTagAvailable()){
		return m->lyric_tag_content.trimmed();
	}

	return QString();
}

bool LyricsImpl::isLyricValid() const
{
	return m->is_valid;
}

bool LyricsImpl::isLyricTagAvailable() const
{
	return (!m->lyric_tag_content.isEmpty());
}

bool LyricsImpl::isLyricTagSupported() const
{
	return Tagging::isLyricsSupported(m->md.filepath());
}

void LyricsImpl::lyricsFetched()
{
	auto* lyric_thread = static_cast<::Lyrics::LookupThread*>(sender());

	m->lyrics = lyric_thread->lyricData();
	m->lyric_header = lyric_thread->lyricHeader();
	m->is_valid = (!lyric_thread->hasError());

	lyric_thread->deleteLater();

	emit sigLyricsFetched();
}

void LyricsImpl::Private::guess_artist_and_title()
{
	bool guessed = false;

	if(	md.radioMode() == RadioMode::Station &&
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

		else if(!md.albumArtist().isEmpty()) {
			artist = md.albumArtist();
			title = md.title();
		}

		else {
			artist = md.artist();
			title = md.title();
		}
	}
}

