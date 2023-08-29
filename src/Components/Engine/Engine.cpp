/* PlaybackEngine.cpp */

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

#include "Engine.h"
#include "Components/Engine/Callbacks.h"
#include "Components/Engine/Pipeline.h"
#include "Components/Engine/EngineUtils.h"

#include "StreamRecorder/StreamRecorder.h"

#include "Utils/FileSystem.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagWriter.h"

#include <QUrl>
#include <QList>

#include <algorithm>
#include <exception>

namespace Engine
{
	struct PipelineCreationException :
		public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override;
	};

	const char* PipelineCreationException::what() const noexcept
	{
		return "Pipeline could not be created";
	}

	struct Engine::Engine::Private
	{
		Util::FileSystemPtr fileSystem;
		Tagging::TagWriterPtr tagWriter;
		MetaData currentTrack;

		PipelinePtr pipeline, otherPipeline;

		std::vector<float> spectrumValues;
		QPair<float, float> levelValues;

		PlayManager* playManager;
		StreamRecorder::StreamRecorder* streamRecorder = nullptr;

		MilliSeconds currentPositionMs;
		GaplessState gaplessState;

		Private(Util::FileSystemPtr fileSystem, Tagging::TagWriterPtr tagWriter, PlayManager* playManager) :
			fileSystem(std::move(fileSystem)),
			tagWriter(std::move(tagWriter)),
			playManager(playManager),
			currentPositionMs(0),
			gaplessState(GaplessState::Stopped) {}

		void changeGaplessState(GaplessState state)
		{
			const auto playlsitMode = GetSetting(Set::PL_Mode);
			const auto gapless = Playlist::Mode::isActiveAndEnabled(playlsitMode.gapless());
			const auto crossfader = GetSetting(Set::Engine_CrossFaderActive);

			this->gaplessState = state;

			if(!gapless && !crossfader)
			{
				this->gaplessState = GaplessState::NoGapless;
			}
		}
	};

	Engine::Engine(const Util::FileSystemPtr& fileSystem, const Tagging::TagWriterPtr& tagWriter,
	               PlayManager* playManager, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(fileSystem, tagWriter, playManager)}
	{
		gst_init(nullptr, nullptr);

		m->pipeline = initPipeline("FirstPipeline");
		if(!m->pipeline)
		{
			throw PipelineCreationException();
		}

		const auto sink = GetSetting(Set::Engine_Sink);
		if(sink == "alsa")
		{
			Playlist::Mode plm = GetSetting(Set::PL_Mode);
			plm.setGapless(false, false);

			SetSetting(Set::Engine_CrossFaderActive, false);
			SetSetting(Set::PL_Mode, plm);
		}

		ListenSetting(Set::Engine_SR_Active, Engine::streamrecorderActiveChanged);
		ListenSetting(Set::PL_Mode, Engine::gaplessChanged);
		ListenSetting(Set::Engine_CrossFaderActive, Engine::gaplessChanged);
	}

	Engine::~Engine()
	{
		if(isStreamRecorderRecording())
		{
			setStreamRecorderRecording(false);
		}

		if(m->streamRecorder)
		{
			m->streamRecorder->deleteLater();
			m->streamRecorder = nullptr;
		}
	}

	PipelinePtr Engine::initPipeline(const QString& name)
	{
		auto pipeline = std::make_shared<Pipeline>(name);
		if(!pipeline->init(this))
		{
			m->changeGaplessState(GaplessState::NoGapless);
			return nullptr;
		}

		connect(pipeline.get(), &Pipeline::sigAboutToFinishMs, this, &Engine::setTrackAlmostFinished);
		connect(pipeline.get(), &Pipeline::sigPositionChangedMs, this, &Engine::currentPositionChanged);
		connect(pipeline.get(), &Pipeline::sigDataAvailable, this, &Engine::sigDataAvailable);

		return pipeline;
	}

	void Engine::Engine::swapPipelines()
	{
		m->otherPipeline->setVisualizerEnabled
			(
				m->pipeline->isLevelVisualizerEnabled(),
				m->pipeline->isSpectrumVisualizerEnabled()
			);

		m->otherPipeline->setBroadcastingEnabled
			(
				m->pipeline->isBroadcastingEnabled()
			);

		m->pipeline->setVisualizerEnabled(false, false);
		m->pipeline->setBroadcastingEnabled(false);

		std::swap(m->pipeline, m->otherPipeline);
	}

	bool Engine::changeTrackCrossfading(const MetaData& track)
	{
		swapPipelines();

		m->otherPipeline->fadeOut();

		const auto success = changeMetadata(track);
		if(success)
		{
			m->pipeline->fadeIn();
			m->changeGaplessState(GaplessState::Playing);
		}

		return success;
	}

	bool Engine::changeTrackGapless(const MetaData& track)
	{
		swapPipelines();

		const auto success = changeMetadata(track);
		if(success)
		{
			const auto timeToGo = m->otherPipeline->timeToGo();
			m->pipeline->playIn(timeToGo);

			m->changeGaplessState(GaplessState::TrackFetched);

			spLog(Log::Develop, this) << "Will start playing in " << timeToGo << "msec";
		}

		return success;
	}

	bool Engine::changeTrackImmediatly(const MetaData& track)
	{
		if(m->otherPipeline)
		{
			m->otherPipeline->stop();
		}

		m->pipeline->stop();

		return changeMetadata(track);
	}

	bool Engine::changeTrack(const MetaData& track)
	{
		if(!m->pipeline)
		{
			return false;
		}

		const auto crossfaderActive = GetSetting(Set::Engine_CrossFaderActive);
		if(m->gaplessState != GaplessState::Stopped && crossfaderActive)
		{
			return changeTrackCrossfading(track);
		}

		else if(m->gaplessState == GaplessState::AboutToFinish)
		{
			return changeTrackGapless(track);
		}

		return changeTrackImmediatly(track);
	}

	bool Engine::changeMetadata(const MetaData& track)
	{
		m->currentTrack = track;
		setCurrentPositionMs(0);

		const auto filepath = track.filepath();
		const auto isStream = Util::File::isWWW(filepath);
		auto uri = filepath;

		if(isStream)
		{
			uri = QUrl(filepath).toString();
		}

		else if(!filepath.contains("://"))
		{
			const auto url = QUrl::fromLocalFile(filepath);
			uri = url.toString();
		}

		if(uri.isEmpty())
		{
			m->currentTrack = MetaData();

			spLog(Log::Warning, this) << "uri = 0";
			return false;
		}

		const auto success = m->pipeline->prepare(uri);
		if(!success)
		{
			m->changeGaplessState(GaplessState::Stopped);
		}

		return success;
	}

	void Engine::play()
	{
		if(m->gaplessState == GaplessState::AboutToFinish ||
		   m->gaplessState == GaplessState::TrackFetched)
		{
			return;
		}

		m->pipeline->play();

		if(isStreamRecorderRecording())
		{
			setStreamRecorderRecording(true);
		}

		m->changeGaplessState(GaplessState::Playing);
	}

	void Engine::pause()
	{
		m->pipeline->pause();
	}

	void Engine::stop()
	{
		m->pipeline->stop();

		if(m->otherPipeline)
		{
			m->otherPipeline->stop();
		}

		if(isStreamRecorderRecording())
		{
			setStreamRecorderRecording(false);
		}

		m->changeGaplessState(GaplessState::Stopped);
		m->currentPositionMs = 0;
		emit sigBuffering(-1);
	}

	void Engine::jumpAbsMs(MilliSeconds pos_ms)
	{
		m->pipeline->seekAbsolute(pos_ms * GST_MSECOND);
	}

	void Engine::jumpRelMs(MilliSeconds ms)
	{
		MilliSeconds new_time_ms = m->pipeline->positionMs() + ms;
		m->pipeline->seekAbsolute(new_time_ms * GST_MSECOND);
	}

	void Engine::jumpRel(double percent)
	{
		m->pipeline->seekRelative(percent, m->currentTrack.durationMs() * GST_MSECOND);
	}

	void Engine::setCurrentPositionMs(MilliSeconds pos_ms)
	{
		if(std::abs(m->currentPositionMs - pos_ms) >= Utils::getUpdateInterval())
		{
			m->currentPositionMs = pos_ms;
			emit sigCurrentPositionChanged(pos_ms);
		}
	}

	void Engine::currentPositionChanged(MilliSeconds pos_ms)
	{
		if(sender() == m->pipeline.get())
		{
			this->setCurrentPositionMs(pos_ms);
		}
	}

	void Engine::setTrackReady(GstElement* src)
	{
		if(m->pipeline->hasElement(src))
		{
			emit sigTrackReady();
		}
	}

	void Engine::setTrackAlmostFinished(MilliSeconds time2go)
	{
		if(sender() != m->pipeline.get())
		{
			return;
		}

		if(m->gaplessState == GaplessState::NoGapless ||
		   m->gaplessState == GaplessState::AboutToFinish)
		{
			return;
		}

		spLog(Log::Develop, this) << "About to finish: " <<
		                          int(m->gaplessState) << " (" << time2go << "ms)";

		m->changeGaplessState(GaplessState::AboutToFinish);

		const auto crossfade = GetSetting(Set::Engine_CrossFaderActive);
		if(crossfade)
		{
			m->pipeline->fadeOut();
		}

		emit sigTrackFinished();
	}

	void Engine::setTrackFinished(GstElement* src)
	{
		if(m->pipeline->hasElement(src))
		{
			emit sigTrackFinished();
		}

		if(m->otherPipeline && m->otherPipeline->hasElement(src))
		{
			spLog(Log::Debug, this) << "Old track finished";

			m->otherPipeline->stop();
			m->changeGaplessState(GaplessState::Playing);
		}
	}

	void Engine::setEqualizer(int band, int val)
	{
		m->pipeline->setEqualizerBand(band, val);

		if(m->otherPipeline)
		{
			m->otherPipeline->setEqualizerBand(band, val);
		}
	}

	MetaData Engine::Engine::currentTrack() const
	{
		return m->currentTrack;
	}

	void Engine::setBufferState(int progress, GstElement* src)
	{
		if(!Util::File::isWWW(m->currentTrack.filepath()))
		{
			progress = -1;
		}

		else if(!m->pipeline->hasElement(src))
		{
			progress = -1;
		}

		emit sigBuffering(progress);
	}

	void Engine::gaplessChanged()
	{
		const auto playlistMode = GetSetting(Set::PL_Mode);
		const auto gapless = (Playlist::Mode::isActiveAndEnabled(playlistMode.gapless()) ||
		                      GetSetting(Set::Engine_CrossFaderActive));

		if(gapless)
		{
			if(!m->otherPipeline)
			{
				m->otherPipeline = initPipeline("SecondPipeline");
			}

			m->changeGaplessState(GaplessState::Stopped);
		}

		else
		{
			m->changeGaplessState(GaplessState::NoGapless);
		}
	}

	void Engine::streamrecorderActiveChanged()
	{
		const auto isActive = GetSetting(Set::Engine_SR_Active);
		if(!isActive)
		{
			setStreamRecorderRecording(false);
		}
	}

	bool Engine::isStreamRecorderRecording() const
	{
		const auto streamRecorderActive = GetSetting(Set::Engine_SR_Active);
		return (streamRecorderActive && m->streamRecorder && m->streamRecorder->isRecording());
	}

	void Engine::setStreamRecorderRecording(const bool b)
	{
		if(b)
		{
			if(!m->streamRecorder)
			{
				m->streamRecorder = new StreamRecorder::StreamRecorder(m->playManager,
				                                                       m->fileSystem,
				                                                       m->tagWriter,
				                                                       m->pipeline,
				                                                       this);
			}
		}

		if(m->streamRecorder)
		{
			m->streamRecorder->record(b);
			if(b)
			{
				m->streamRecorder->changeTrack(m->currentTrack);
			}
		}
	}

	void Engine::updateCover(GstElement* src, const QByteArray& data, const QString& mimetype)
	{
		if(m->pipeline->hasElement(src))
		{
			emit sigCoverDataAvailable(data, mimetype);
		}
	}

	void Engine::updateMetadata(const MetaData& track, GstElement* src)
	{
		if(!m->pipeline->hasElement(src))
		{
			return;
		}

		if(!Util::File::isWWW(m->currentTrack.filepath()))
		{
			return;
		}

		m->currentTrack = track;

		setCurrentPositionMs(0);

		emit sigMetadataChanged(m->currentTrack);

		if(isStreamRecorderRecording())
		{
			setStreamRecorderRecording(true);
		}
	}

	void Engine::updateDuration(GstElement* src)
	{
		if(!m->pipeline->hasElement(src))
		{
			return;
		}

		const auto durationMs = m->pipeline->durationMs();
		const auto difference = std::abs(durationMs - m->currentTrack.durationMs());
		if(durationMs < 1000 || difference < 1999 || durationMs > 1500000000)
		{
			return;
		}

		m->currentTrack.setDurationMs(durationMs);
		updateMetadata(m->currentTrack, src);

		emit sigDurationChanged(m->currentTrack);

		m->pipeline->checkPosition();
	}

	template<typename T>
	T bitrateDiff(T a, T b) { return std::max(a, b) - std::min(a, b); }

	void Engine::updateBitrate(Bitrate bitrate, GstElement* src)
	{
		if((!m->pipeline->hasElement(src)) ||
		   (bitrate == 0) ||
		   (bitrateDiff(bitrate, m->currentTrack.bitrate()) < 1000))
		{
			return;
		}

		m->currentTrack.setBitrate(bitrate);

		emit sigBitrateChanged(m->currentTrack);
	}

	void Engine::setBroadcastEnabled(bool b)
	{
		m->pipeline->setBroadcastingEnabled(b);
		if(m->otherPipeline)
		{
			m->otherPipeline->setBroadcastingEnabled(b);
		}
	}

	void Engine::setSpectrum(const std::vector<float>& vals)
	{
		m->spectrumValues = vals;
		emit sigSpectrumChanged();
	}

	const std::vector<float>& Engine::Engine::spectrum() const
	{
		return m->spectrumValues;
	}

	void Engine::setLevel(float left, float right)
	{
		m->levelValues = {left, right};
		emit sigLevelChanged();
	}

	QPair<float, float> Engine::Engine::level() const
	{
		return m->levelValues;
	}

	void Engine::Engine::setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled)
	{
		m->pipeline->setVisualizerEnabled(levelEnabled, spectrumEnabled);
		if(m->otherPipeline)
		{
			m->otherPipeline->setVisualizerEnabled(levelEnabled, spectrumEnabled);
		}
	}

	void Engine::error(const QString& error, const QString& element_name)
	{
		QStringList msg {Lang::get(Lang::Error)};

		if(m->currentTrack.filepath().contains("soundcloud", Qt::CaseInsensitive))
		{
			msg << "Probably, Sayonara's Soundcloud limit of 15.000 "
			       "tracks per day is reached :( Sorry.";
		}

		if(error.trimmed().length() > 0)
		{
			msg << error;
		}

		if(element_name.contains("alsa"))
		{
			msg << tr("You should restart Sayonara now") + ".";
		}

		stop();

		emit sigError(msg.join("\n\n"));
	}
} // namespace