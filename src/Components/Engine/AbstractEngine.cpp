/* Engine.cpp */

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

#include "AbstractEngine.h"

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"

#include <QUrl>

#include <gst/gst.h>
#include <cstdlib>

using Engine::Base;
using Engine::Name;

struct Base::Private
{
	MetaData		md;
	QString			uri;
	MilliSeconds	cur_pos_ms;
	Name			name;

	Private(Name name) :
		cur_pos_ms(0),
		name(name)
	{}
};

Base::Base(Name name, QObject *parent) :
	QObject(parent),
	SayonaraClass()
{
	m = Pimpl::make<Private>(name);
}

Base::~Base() {}

Engine::Name Base::name() const
{
	return m->name;
}


bool Base::change_track(const MetaData& md)
{
	m->cur_pos_ms = 0;

	return change_metadata(md);
}

bool Base::change_track_by_filename(const QString& filepath)
{
	MetaData md(filepath);

	bool success = true;

	bool got_md = Tagging::Utils::getMetaDataOfFile(md);
	if( !got_md ) {
		stop();
		success = false;
	}

	else{
		success = change_track(md);
	}

	return success;
}


bool Base::change_metadata(const MetaData& md)
{
	QString filepath = md.filepath();
	bool playing_stream = Util::File::is_www(filepath);

	// stream, but don't want to record
	// stream is already uri
	if (playing_stream)
	{
		m->uri = QUrl(filepath).toString();
	}

	// no stream (not quite right because of mms, rtsp or other streams
	// normal filepath -> no uri
	else if (!filepath.contains("://"))
	{
		QUrl url = QUrl::fromLocalFile(md.filepath());
		m->uri = url.toString();
	}

	else {
		m->uri = md.filepath();
	}

	if(m->uri.isEmpty())
	{
		m->md = MetaData();

		sp_log(Log::Warning, this) << "uri = 0";
		return false;
	}

	m->md = md;

	return change_uri(m->uri);
}


void Base::set_track_ready(GstElement* src)
{
	Q_UNUSED(src)
	emit sig_track_ready();
}

void Base::set_track_almost_finished(MilliSeconds time2go)
{
	emit sig_track_almost_finished(time2go);
}

void Base::set_track_finished(GstElement* src)
{
	Q_UNUSED(src)
	emit sig_track_finished();
}

void Base::update_metadata(const MetaData& md, GstElement* src)
{
	Q_UNUSED(src)

	m->md = md;
	emit sig_md_changed(m->md);
}

void Base::update_cover(const QImage& img, GstElement* src)
{
	Q_UNUSED(src)
	emit sig_cover_changed(img);
}

void Base::update_duration(MilliSeconds duration_ms, GstElement* src)
{
	Seconds duration_s = (duration_ms / 1000);
	Seconds md_duration_s = (metadata().length_ms / 1000);

	if(duration_s == 0 || duration_s > 1500000){
		return;
	}

	if(duration_s == md_duration_s) {
		return;
	}

	m->md.length_ms = duration_ms;
	update_metadata(m->md, src);

	emit sig_dur_changed(m->md);
}


void Base::update_bitrate(Bitrate br, GstElement* src)
{
	if( br <= 0) {
		return;
	}

	if( std::abs((int) br - (int) m->md.bitrate) <= 1000) { // (br / 1000) == (m->md.bitrate / 1000)
		return;
	}

	m->md.bitrate = br;
	update_metadata(m->md, src);

	emit sig_br_changed(m->md);
}


void Base::stop()
{
	m->cur_pos_ms = 0;

	emit sig_buffering(-1);
}

const MetaData& Base::metadata() const
{
	return m->md;
}

void Base::set_current_position_ms(MilliSeconds pos_ms)
{
	if ( m->cur_pos_ms / 100 == (pos_ms / 100)){
		return;
	}

	m->cur_pos_ms = pos_ms;

	emit sig_pos_changed_ms(pos_ms);
}


MilliSeconds Base::current_position_ms() const
{
	return m->cur_pos_ms;
}


void Base::set_buffer_state(int progress, GstElement* src)
{
	Q_UNUSED(src)
	emit sig_buffering(progress);
}


void Base::error(const QString& error)
{
	QString msg("Cannot play track");

	if(m->md.filepath().contains("soundcloud", Qt::CaseInsensitive))
	{
		msg += QString("\n\n") +
			   "Probably, Sayonara's Soundcloud limit of 15.000 "
			   "tracks per day is reached :( Sorry.";
	}

	if(error.trimmed().length() > 0){
		msg += QString("\n\n") + error;
	}

	emit sig_error(msg);
	stop();
}
