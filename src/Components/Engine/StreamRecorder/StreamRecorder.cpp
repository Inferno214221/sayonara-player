/* StreamRecorder.cpp */

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

#include "StreamRecorder.h"
#include "Interfaces/PlayManager.h"
#include "Components/PlayManager/PlayManagerProvider.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StandardPaths.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"

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


SR::StreamRecorder::StreamRecorder(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<StreamRecorder::Private>();

	clear();

	auto* playManager = PlayManagerProvider::instance()->playManager();
	connect(playManager, &PlayManager::sigPlaystateChanged, this, &StreamRecorder::playstateChanged);
}

SR::StreamRecorder::~StreamRecorder() {}

void SR::StreamRecorder::clear()
{
	m->md.setTitle("");
	m->session_collector.clear();
	m->sr_recording_dst = "";
	m->session_playlist_name.clear();
	m->cur_idx = 1;
}

void SR::StreamRecorder::newSession()
{
	clear();

	m->date = QDate::currentDate();
	m->time = QTime::currentTime();
}


QString SR::StreamRecorder::changeTrack(const MetaData& md)
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

	if(!Util::File::isWWW(md.filepath()))
	{
		spLog(Log::Warning, this) << "Audio Source is not a stream";
		m->sr_recording_dst = "";
		m->session_playlist_name = "";
		m->recording = false;
		return "";
	}

	m->md = md;
	m->md.setYear(Year(QDateTime::currentDateTime().date().year()));
	m->md.setTrackNumber(TrackNum(m->cur_idx));

	int i;
	QString target_path_template = GetSetting(Set::Engine_SR_SessionPathTemplate);
	bool use_session_path = GetSetting(Set::Engine_SR_SessionPath);
	bool valid = (Utils::validateTemplate(target_path_template, &i) == Utils::ErrorCode::OK);

	if(!use_session_path || !valid)
	{
		target_path_template = Utils::targetPathTemplateDefault(use_session_path);
	}

	Utils::TargetPaths target_path = Utils::fullTargetPath(sr_path, target_path_template, m->md, m->date, m->time);
	if(target_path.first.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot determine target path";
		m->sr_recording_dst = "";
		m->session_playlist_name = "";
		m->recording = false;
		return "";
	}

	Util::File::createDirectories(Util::File::getParentDirectory(target_path.first));

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

	spLog(Log::Info, this) << "Finalize file " << m->sr_recording_dst;

	m->md.setFilepath(m->sr_recording_dst);

	Tagging::Utils::setMetaDataOfFile(m->md);
	m->session_collector.push_back(m->md);

	PlaylistParser::saveM3UPlaylist(m->session_playlist_name, m->session_collector, true);

	return true;
}


QString SR::StreamRecorder::checkTargetPath(const QString& target_path)
{
	if(!FileUtils::exists(target_path)) {
		Util::File::createDirectories(Util::File::getParentDirectory(target_path));
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

	spLog(Log::Debug, this) << "Stream recorder: activate: " << b;

	if(b) {
		newSession();
	}

	else {
		save();
		clear();
	}

	m->recording = b;
}


bool SR::StreamRecorder::isRecording() const
{
	return m->recording;
}


void SR::StreamRecorder::playstateChanged(PlayState state)
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
