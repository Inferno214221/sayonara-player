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

#include "Components/PlayManager/PlayManager.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/M3UParser.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Utils.h"

#include <QDir>
#include <QFile>
#include <QDateTime>

namespace SR = StreamRecorder;
namespace FileUtils = ::Util::File;

struct SR::StreamRecorder::Private
{
	QString recordingDestination;
	QString sessionPlaylistName;
	MetaDataList sessionCollector;
	MetaData currentTrack;
	QDate date;
	QTime time;

	int currentIndex;
	bool recording;

	Private() :
		currentIndex(1),
		recording(false) {}
};

SR::StreamRecorder::StreamRecorder(PlayManager* playManager, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<StreamRecorder::Private>();

	clear();

	connect(playManager, &PlayManager::sigPlaystateChanged, this, &StreamRecorder::playstateChanged);
}

SR::StreamRecorder::~StreamRecorder() = default;

void SR::StreamRecorder::clear()
{
	m->currentTrack.setTitle("");
	m->sessionCollector.clear();
	m->recordingDestination = "";
	m->sessionPlaylistName.clear();
	m->currentIndex = 1;
}

void SR::StreamRecorder::newSession()
{
	clear();

	m->date = QDate::currentDate();
	m->time = QTime::currentTime();
}

QString SR::StreamRecorder::changeTrack(const MetaData& track)
{
	const auto streamRecorderPath = GetSetting(Set::Engine_SR_Path);

	if(!m->recording)
	{
		return QString();
	}

	if(track.title() == m->currentTrack.title())
	{
		return m->recordingDestination;
	}

	const auto saved = save();
	if(saved)
	{
		m->currentIndex++;
	}

	if(!Util::File::isWWW(track.filepath()))
	{
		spLog(Log::Warning, this) << "Audio Source is not a stream";
		m->recordingDestination = "";
		m->sessionPlaylistName = "";
		m->recording = false;
		return QString();
	}

	m->currentTrack = track;
	m->currentTrack.setYear(Year(QDateTime::currentDateTime().date().year()));
	m->currentTrack.setTrackNumber(TrackNum(m->currentIndex));

	int i;
	auto targetPathTemplate = GetSetting(Set::Engine_SR_SessionPathTemplate);
	const auto useSessionPath = GetSetting(Set::Engine_SR_SessionPath);
	const auto valid = (Utils::validateTemplate(targetPathTemplate, &i) == Utils::ErrorCode::OK);

	if(!useSessionPath || !valid)
	{
		targetPathTemplate = Utils::targetPathTemplateDefault(useSessionPath);
	}

	const auto targetPath = Utils::fullTargetPath(streamRecorderPath,
	                                              targetPathTemplate,
	                                              m->currentTrack,
	                                              m->date,
	                                              m->time);
	if(targetPath.first.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot determine target path";
		m->recordingDestination.clear();
		m->sessionPlaylistName.clear();
		m->recording = false;
		return QString();
	}

	Util::File::createDirectories(Util::File::getParentDirectory(targetPath.first));

	m->recordingDestination = targetPath.first;
	m->sessionPlaylistName = targetPath.second;

	return m->recordingDestination;
}

bool SR::StreamRecorder::save()
{
	if(!FileUtils::exists(m->recordingDestination))
	{
		return false;
	}

	const auto fileInfo = QFileInfo(m->recordingDestination);
	if(fileInfo.size() < 4000)
	{
		return false;
	}

	spLog(Log::Info, this) << "Finalize file " << m->recordingDestination;

	m->currentTrack.setFilepath(m->recordingDestination);

	Tagging::Utils::setMetaDataOfFile(m->currentTrack);
	m->sessionCollector.push_back(m->currentTrack);

	M3UParser::saveM3UPlaylist(m->sessionPlaylistName, m->sessionCollector, true);

	return true;
}

QString SR::StreamRecorder::checkTargetPath(const QString& targetPath)
{
	if(!FileUtils::exists(targetPath))
	{
		Util::File::createDirectories(Util::File::getParentDirectory(targetPath));
	}

	const auto fileInfo = QFileInfo(targetPath);
	return (fileInfo.isWritable())
		? targetPath
		: QString();
}

void SR::StreamRecorder::record(bool b)
{
	if(b == m->recording)
	{
		return;
	}

	spLog(Log::Debug, this) << "Stream recorder: activate: " << b;

	if(b)
	{
		newSession();
	}

	else
	{
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
