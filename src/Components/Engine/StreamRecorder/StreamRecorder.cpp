/* StreamRecorder.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "StreamRecorder.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"

#include "Components/PlayManager/PlayManager.h"

#include <QDir>
#include <QFile>
#include <QDateTime>

namespace SR=StreamRecorder;
namespace FileUtils=::Util::File;

struct SR::StreamRecorder::Private
{
	QString			sr_recording_dst;				// recording destination
	QString			session_playlist_name;			// playlist name
	MetaDataList	session_collector;				// gather all tracks of a session
	MetaData		md;                             // current track
	QDate           date;
	QTime           time;

	int				cur_idx;						// index of track (used for filename)
	bool            recording;						// is in a session currently

	Private() :
		cur_idx(1),
		recording(false)
	{}
};


SR::StreamRecorder::StreamRecorder(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<StreamRecorder::Private>();

	clear();

	QDir d(Util::sayonara_path());

	// delete old stream ripper files
	QStringList lst = d.entryList(Util::soundfile_extensions(), (QDir::Files | QDir::NoDotAndDotDot));

	for( const QString& str : lst)
	{
		QString path = d.absoluteFilePath(str);
		QFile f(path);
		f.remove();

		sp_log(Log::Info, this) << "Remove " << path;
	}

	auto* play_manager = PlayManager::instance();
	connect(play_manager, &PlayManager::sig_playstate_changed, this, &StreamRecorder::playstate_changed);
}

SR::StreamRecorder::~StreamRecorder() = default;

void SR::StreamRecorder::clear()
{
	m->md.set_title("");
	m->session_collector.clear();
	m->sr_recording_dst = "";
	m->session_playlist_name.clear();
	m->cur_idx = 1;
}

void SR::StreamRecorder::new_session()
{
	clear();

	m->date = QDate::currentDate();
	m->time = QTime::currentTime();
}


QString SR::StreamRecorder::change_track(const MetaData& md)
{
	QString sr_path = GetSetting(Set::Engine_SR_Path);

	if(!m->recording){
		return "";
	}

	if(md.title() == m->md.title()) {
		return m->sr_recording_dst;
	}

	bool saved = save();
	if(saved)
	{
		m->cur_idx++;
	}

	if(!Util::File::is_www(md.filepath()))
	{
		sp_log(Log::Warning, this) << "Audio Source is not a stream";
		m->sr_recording_dst = "";
		m->session_playlist_name = "";
		m->recording = false;
		return "";
	}

	m->md = md;
	m->md.set_year(Year(QDateTime::currentDateTime().date().year()));
	m->md.set_track_number(TrackNum(m->cur_idx));

	int i;
	QString target_path_template = GetSetting(Set::Engine_SR_SessionPathTemplate);
	bool use_session_path = GetSetting(Set::Engine_SR_SessionPath);
	bool valid = (Utils::validate_template(target_path_template, &i) == Utils::ErrorCode::OK);

	if(!use_session_path || !valid)
	{
		target_path_template = Utils::target_path_template_default(use_session_path);
	}

	Utils::TargetPaths target_path = Utils::full_target_path(sr_path, target_path_template, m->md, m->date, m->time);
	if(target_path.first.isEmpty())
	{
		sp_log(Log::Warning, this) << "Cannot determine target path";
		m->sr_recording_dst = "";
		m->session_playlist_name = "";
		m->recording = false;
		return "";
	}

	Util::File::create_directories(Util::File::get_parent_directory(target_path.first));

	m->sr_recording_dst = target_path.first;
	m->session_playlist_name = target_path.second;

	return m->sr_recording_dst;
}


bool SR::StreamRecorder::save()
{
	if(!FileUtils::exists(m->sr_recording_dst)) {
		return false;
	}

	QFileInfo file_info(m->sr_recording_dst);
	if(file_info.size() < 4000) {
		return false;
	}

	sp_log(Log::Info, this) << "Finalize file " << m->sr_recording_dst;

	m->md.set_filepath(m->sr_recording_dst);

	Tagging::Utils::setMetaDataOfFile(m->md);
	m->session_collector.push_back(m->md);

	PlaylistParser::save_m3u_playlist(m->session_playlist_name, m->session_collector, true);

	return true;
}


QString SR::StreamRecorder::check_target_path(const QString& target_path)
{
	if(!FileUtils::exists(target_path)) {
		Util::File::create_directories(Util::File::get_parent_directory(target_path));
	}

	QFileInfo fi(target_path);

	if(!fi.isWritable()){
		return "";
	}

	return target_path;
}

void SR::StreamRecorder::record(bool b)
{
	if(b == m->recording) {
		return;
	}

	sp_log(Log::Debug, this) << "Stream recorder: activate: " << b;

	if(b) {
		new_session();
	}

	else {
		save();
		clear();
	}

	m->recording = b;
}


bool SR::StreamRecorder::is_recording() const
{
	return m->recording;
}


void SR::StreamRecorder::playstate_changed(PlayState state)
{
	if(state == PlayState::Stopped)
	{
		if(m->recording)
		{
			save();
			clear();
		}
	}
}
