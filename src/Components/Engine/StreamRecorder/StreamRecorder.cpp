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

#include "Components/Engine/PipelineExtensions/StreamRecordable.h"
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
#include <QDate>
#include <QTime>

#include <memory>

namespace StreamRecorder
{
	namespace
	{
		using StreamRecordablePtr = std::shared_ptr<PipelineExtensions::StreamRecordable>;

		Utils::TargetPath prepareTargetPath(const MetaData& track, const QDate& date, const QTime& time)
		{
			int i;
			auto targetPathTemplate = GetSetting(Set::Engine_SR_SessionPathTemplate);
			const auto useSessionPath = GetSetting(Set::Engine_SR_SessionPath);
			const auto valid = (Utils::validateTemplate(targetPathTemplate, &i) == Utils::ErrorCode::OK);

			if(!useSessionPath || !valid)
			{
				targetPathTemplate = Utils::targetPathTemplateDefault(useSessionPath);
			}

			const auto streamRecorderPath = GetSetting(Set::Engine_SR_Path);
			return Utils::fullTargetPath(streamRecorderPath,
			                             targetPathTemplate,
			                             track,
			                             date,
			                             time);
		}
	}

	struct StreamRecorder::Private
	{
		StreamRecordablePtr streamRecordable;
		QString recordingDestination;
		QString sessionPlaylistName;
		MetaDataList sessionCollector;
		MetaData currentTrack;
		QDate date;
		QTime time;

		int currentIndex {1};
		bool recording {false};

		explicit Private(StreamRecordablePtr streamRecordable) :
			streamRecordable {std::move(streamRecordable)} {}
	};

	StreamRecorder::StreamRecorder(PlayManager* playManager, StreamRecordablePtr streamRecordable,
	                               QObject* parent) :
		QObject(parent),
		m {Pimpl::make<StreamRecorder::Private>(std::move(streamRecordable))}
	{
		connect(playManager, &PlayManager::sigPlaystateChanged, this, &StreamRecorder::playstateChanged);
	}

	StreamRecorder::~StreamRecorder() = default;

	void StreamRecorder::startNewSession()
	{
		m->recording = true;
		m->currentTrack.setTitle({});
		m->sessionCollector.clear();
		m->recordingDestination.clear();
		m->sessionPlaylistName.clear();
		m->currentIndex = 1;
		m->date = QDate::currentDate();
		m->time = QTime::currentTime();
		m->streamRecordable->prepareForRecording();
	}

	void StreamRecorder::endSession()
	{
		save();
		m->recording = false;
		m->recordingDestination.clear();
		m->sessionPlaylistName.clear();
		m->streamRecordable->finishRecording();
	}

	bool StreamRecorder::save()
	{
		const auto fileInfo = QFileInfo(m->recordingDestination);
		if(!fileInfo.exists())
		{
			spLog(Log::Info, this) << "Skip file " << m->recordingDestination;
			return false;
		}

		spLog(Log::Info, this) << "Finalize file " << m->recordingDestination;

		m->currentTrack.setFilepath(m->recordingDestination);

		Tagging::Utils::setMetaDataOfFile(m->currentTrack);
		m->sessionCollector.push_back(m->currentTrack);

		M3UParser::saveM3UPlaylist(m->sessionPlaylistName, m->sessionCollector, true);

		return true;
	}

	void StreamRecorder::changeTrack(const MetaData& track)
	{
		if(!m->recording)
		{
			return;
		}

		if(track.title() == m->currentTrack.title())
		{
			m->streamRecordable->setRecordingPath(m->recordingDestination);
			return;
		}

		if(save())
		{
			m->currentIndex++;
		}

		if(!Util::File::isWWW(track.filepath()))
		{
			endSession();
			spLog(Log::Warning, this) << "Audio Source is not a stream. Clearing session.";

			return;
		}

		m->currentTrack = track;
		m->currentTrack.setYear(static_cast<Year>(QDateTime::currentDateTime().date().year()));
		m->currentTrack.setTrackNumber(static_cast<TrackNum>(m->currentIndex));

		const auto [audioPath, playlistPath] = prepareTargetPath(m->currentTrack, m->date, m->time);
		if(audioPath.isEmpty())
		{
			spLog(Log::Warning, this) << "Cannot determine target path";
			endSession();
			return;
		}

		Util::File::createDirectories(Util::File::getParentDirectory(audioPath));

		m->recordingDestination = audioPath;
		m->sessionPlaylistName = playlistPath;

		spLog(Log::Info, this) << "Change file: " << m->recordingDestination;

		m->streamRecordable->setRecordingPath(m->recordingDestination);
	}

	void StreamRecorder::record(const bool b)
	{
		if(b == m->recording)
		{
			return;
		}

		if(b)
		{
			startNewSession();
		}

		else
		{
			endSession();
		}
	}

	bool StreamRecorder::isRecording() const { return m->recording; }

	void StreamRecorder::playstateChanged(const PlayState state)
	{
		if(state == PlayState::Stopped)
		{
			record(false);
		}
	}
}
