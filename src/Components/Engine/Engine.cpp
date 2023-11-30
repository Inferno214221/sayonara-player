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
#include "Interfaces/Engine/AudioDataReceiver.h"

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
#include <utility>

namespace Engine
{
	using PipelinePtr = std::shared_ptr<Pipeline>;

	struct PipelineCreationException :
		public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override;
	};

	const char* PipelineCreationException::what() const noexcept
	{
		return "Pipeline could not be created";
	}

	class EngineImpl :
		public Engine
	{
		public:
			EngineImpl(Util::FileSystemPtr fileSystem, Tagging::TagWriterPtr tagWriter, QObject* parent) :
				Engine(parent),
				m_fileSystem {std::move(fileSystem)},
				m_tagWriter {std::move(tagWriter)}
			{
				gst_init(nullptr, nullptr);

				m_pipeline = initPipeline("FirstPipeline");
				if(!m_pipeline)
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

				ListenSetting(Set::Engine_SR_Active, EngineImpl::streamrecorderActiveChanged);
				ListenSetting(Set::PL_Mode, EngineImpl::gaplessChanged);
				ListenSetting(Set::Engine_CrossFaderActive, EngineImpl::gaplessChanged);
			}

			~EngineImpl() override
			{
				if(isStreamRecorderRecording())
				{
					setStreamRecorderRecording(false);
				}

				if(m_streamRecorder)
				{
					m_streamRecorder->deleteLater();
					m_streamRecorder = nullptr;
				}
			}

			bool changeTrack(const MetaData& track) override
			{
				if(!m_pipeline)
				{
					return false;
				}

				const auto crossfaderActive = GetSetting(Set::Engine_CrossFaderActive);
				if(m_gaplessState != GaplessState::Stopped && crossfaderActive)
				{
					return changeTrackCrossfading(track);
				}

				if(m_gaplessState == GaplessState::AboutToFinish)
				{
					return changeTrackGapless(track);
				}

				return changeTrackImmediatly(track);
			}

			void play() override
			{
				if(m_gaplessState == GaplessState::AboutToFinish ||
				   m_gaplessState == GaplessState::TrackFetched)
				{
					return;
				}

				m_pipeline->play();

				if(isStreamRecorderRecording())
				{
					assert(m_streamRecorder);
					m_streamRecorder->updateMetadata(m_currentTrack);
				}

				changeGaplessState(GaplessState::Playing);
			}

			void pause() override { m_pipeline->pause(); }

			void stop() override
			{
				m_pipeline->stop();

				if(m_otherPipeline)
				{
					m_otherPipeline->stop();
				}

				if(isStreamRecorderRecording())
				{
					setStreamRecorderRecording(false);
				}

				changeGaplessState(GaplessState::Stopped);
				m_currentPositionMs = 0;
				emit sigBuffering(-1);
			}

			void jumpAbsMs(const MilliSeconds ms) override { m_pipeline->seekAbsoluteMs(ms); }

			void jumpRelMs(const MilliSeconds ms) override { m_pipeline->seekRelativeMs(ms); }

			void jumpRel(const double percent) override
			{
				m_pipeline->seekRelative(percent, m_currentTrack.durationMs());
			}

			void setTrackReady(GstElement* src) override
			{
				if(m_pipeline->hasElement(src))
				{
					emit sigTrackReady();
				}
			}

			void setTrackAlmostFinished(MilliSeconds time2go) override
			{
				if(sender() != m_pipeline.get())
				{
					return;
				}

				if(m_gaplessState == GaplessState::NoGapless ||
				   m_gaplessState == GaplessState::AboutToFinish)
				{
					return;
				}

				spLog(Log::Develop, this) << "About to finish: " <<
				                          int(m_gaplessState) << " (" << time2go << "ms)";

				changeGaplessState(GaplessState::AboutToFinish);

				const auto crossfade = GetSetting(Set::Engine_CrossFaderActive);
				if(crossfade)
				{
					m_pipeline->fadeOut();
				}

				emit sigTrackFinished();
			}

			void setTrackFinished(GstElement* src) override
			{
				if(m_pipeline->hasElement(src))
				{
					emit sigTrackFinished();
				}

				if(m_otherPipeline && m_otherPipeline->hasElement(src))
				{
					spLog(Log::Debug, this) << "Old track finished";

					m_otherPipeline->stop();
					changeGaplessState(GaplessState::Playing);
				}
			}

			void setEqualizer(const int band, const int val) override
			{
				m_pipeline->setEqualizerBand(band, val);
				if(m_otherPipeline)
				{
					m_otherPipeline->setEqualizerBand(band, val);
				}
			}

			[[nodiscard]] MetaData currentTrack() const override { return m_currentTrack; }

			void setBufferState(int progress, GstElement* src) override
			{
				if(!Util::File::isWWW(m_currentTrack.filepath()) ||
				   !m_pipeline->hasElement(src))
				{
					progress = -1;
				}

				emit sigBuffering(progress); // NOLINT(readability-misleading-indentation)
			}

			[[nodiscard]] bool isStreamRecorderRecording() const override
			{
				const auto streamRecorderActive = GetSetting(Set::Engine_SR_Active);
				return (streamRecorderActive && m_streamRecorder && m_streamRecorder->isRecording());
			}

			void setStreamRecorderRecording(const bool b) override
			{
				if(b)
				{
					if(!m_streamRecorder)
					{
						m_streamRecorder =
							new StreamRecorder::StreamRecorder(m_fileSystem, m_tagWriter, m_pipeline, this);
					}
				}

				if(m_streamRecorder)
				{
					if(b && !m_streamRecorder->isRecording())
					{
						m_streamRecorder->startNewSession(m_currentTrack);
					}

					else if(!b && m_streamRecorder->isRecording())
					{
						m_streamRecorder->endSession();
					}
				}
			}

			void updateCover(GstElement* src, const QByteArray& data, const QString& mimetype) override
			{
				if(m_pipeline->hasElement(src))
				{
					emit sigCoverDataAvailable(data, mimetype);
				}
			}

			void updateMetadata(const MetaData& track, GstElement* src) override
			{
				if(!m_pipeline->hasElement(src))
				{
					return;
				}

				if(!Util::File::isWWW(m_currentTrack.filepath()))
				{
					return;
				}

				m_currentTrack = track;

				setCurrentPositionMs(0);

				emit sigMetadataChanged(m_currentTrack);

				if(isStreamRecorderRecording())
				{
					m_streamRecorder->updateMetadata(track);
				}
			}

			void updateDuration(GstElement* src) override
			{
				if(!m_pipeline->hasElement(src))
				{
					return;
				}

				const auto durationMs = m_pipeline->duration();
				const auto difference = std::abs(durationMs - m_currentTrack.durationMs());
				if(durationMs < 1000 || difference < 1999 || // NOLINT(readability-magic-numbers)
				   durationMs > 1'500'000'000) // NOLINT(readability-magic-numbers)
				{
					return;
				}

				m_currentTrack.setDurationMs(durationMs);
				updateMetadata(m_currentTrack, src);

				emit sigDurationChanged(m_currentTrack);

				m_pipeline->checkPosition();
			}

			template<typename T>
			T bitrateDiff(T a, T b) { return std::max(a, b) - std::min(a, b); }

			void updateBitrate(Bitrate bitrate, GstElement* src) override
			{
				if((!m_pipeline->hasElement(src)) ||
				   (bitrate == 0) ||
				   (bitrateDiff(bitrate, m_currentTrack.bitrate()) < 1000)) // NOLINT(readability-magic-numbers)
				{
					return;
				}

				m_currentTrack.setBitrate(bitrate);

				emit sigBitrateChanged(m_currentTrack);
			}

			void setBroadcastEnabled(const bool b) override
			{
				m_pipeline->setBroadcastingEnabled(b);
				if(m_otherPipeline)
				{
					m_otherPipeline->setBroadcastingEnabled(b);
				}
			}

			void setSpectrum(const std::vector<float>& vals) override
			{
				m_spectrumValues = vals;
				emit sigSpectrumChanged();
			}

			[[nodiscard]]const std::vector<float>& spectrum() const override
			{
				return m_spectrumValues;
			}

			void setLevel(float left, float right) override
			{
				m_levelValues = {left, right};
				emit sigLevelChanged();
			}

			[[nodiscard]]QPair<float, float> level() const override
			{
				return m_levelValues;
			}

			void setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled) override
			{
				m_pipeline->setVisualizerEnabled(levelEnabled, spectrumEnabled);
				if(m_otherPipeline)
				{
					m_otherPipeline->setVisualizerEnabled(levelEnabled, spectrumEnabled);
				}
			}

			void error(const QString& error, const QString& elementName) override
			{
				QStringList msg {Lang::get(Lang::Error)};

				if(m_currentTrack.filepath().contains("soundcloud", Qt::CaseInsensitive))
				{
					msg << "Probably, Sayonara's Soundcloud limit of 15.000 "
					       "tracks per day is reached :( Sorry.";
				}

				if(error.trimmed().length() > 0)
				{
					msg << error;
				}

				if(elementName.contains("alsa"))
				{
					msg << tr("You should restart Sayonara now") + ".";
				}

				stop();

				emit sigError(msg.join("\n\n"));
			}

		private slots:

			void gaplessChanged()
			{
				const auto playlistMode = GetSetting(Set::PL_Mode);
				const auto gapless = (Playlist::Mode::isActiveAndEnabled(playlistMode.gapless()) ||
				                      GetSetting(Set::Engine_CrossFaderActive));

				if(gapless)
				{
					if(!m_otherPipeline)
					{
						m_otherPipeline = initPipeline("SecondPipeline");
					}

					changeGaplessState(GaplessState::Stopped);
				}

				else
				{
					changeGaplessState(GaplessState::NoGapless);
				}
			}

			void streamrecorderActiveChanged()
			{
				const auto isActive = GetSetting(Set::Engine_SR_Active);
				if(!isActive)
				{
					setStreamRecorderRecording(false);
				}
			}

			void currentPositionChanged(const MilliSeconds ms)
			{
				if(sender() == m_pipeline.get())
				{
					setCurrentPositionMs(ms);
				}
			}

		private: // NOLINT(readability-redundant-access-specifiers)
			enum class GaplessState :
				uint8_t
			{
				NoGapless = 0,        // no gapless enabled at all
				AboutToFinish,        // the phase when the new track is already displayed but not played yet
				TrackFetched,        // track is requested, but no yet there
				Playing,            // currently playing
				Stopped
			};

			void setCurrentPositionMs(const MilliSeconds ms)
			{
				if(std::abs(m_currentPositionMs - ms) >= Utils::getUpdateInterval())
				{
					m_currentPositionMs = ms;
					emit sigCurrentPositionChanged(ms);
				}
			}

			PipelinePtr initPipeline(const QString& name)
			{
				auto pipeline = std::make_shared<Pipeline>(name);
				if(!pipeline->init(this))
				{
					changeGaplessState(GaplessState::NoGapless);
					return nullptr;
				}

				connect(pipeline.get(), &Pipeline::sigAboutToFinishMs, this, &EngineImpl::setTrackAlmostFinished);
				connect(pipeline.get(), &Pipeline::sigPositionChangedMs, this, &EngineImpl::currentPositionChanged);
				connect(pipeline.get(), &Pipeline::sigDataAvailable, this, &EngineImpl::sigDataAvailable);

				return pipeline;
			}

			void swapPipelines()
			{
				m_otherPipeline->setVisualizerEnabled(
					m_pipeline->isLevelVisualizerEnabled(),
					m_pipeline->isSpectrumVisualizerEnabled());

				m_otherPipeline->setBroadcastingEnabled(
					m_pipeline->isBroadcastingEnabled());

				m_pipeline->setVisualizerEnabled(false, false);
				m_pipeline->setBroadcastingEnabled(false);

				std::swap(m_pipeline, m_otherPipeline);
			}

			void changeGaplessState(const GaplessState state)
			{
				const auto playlistMode = GetSetting(Set::PL_Mode);
				const auto gapless = Playlist::Mode::isActiveAndEnabled(playlistMode.gapless());
				const auto crossfader = GetSetting(Set::Engine_CrossFaderActive);

				m_gaplessState = state;

				if(!gapless && !crossfader)
				{
					m_gaplessState = GaplessState::NoGapless;
				}
			}

			bool changeMetadata(const MetaData& track)
			{
				m_currentTrack = track;
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
					m_currentTrack = MetaData();

					spLog(Log::Warning, this) << "uri = 0";
					return false;
				}

				const auto success = m_pipeline->prepare(uri);
				if(!success)
				{
					changeGaplessState(GaplessState::Stopped);
				}

				return success;
			}

			bool changeTrackCrossfading(const MetaData& track)
			{
				swapPipelines();

				m_otherPipeline->fadeOut();

				const auto success = changeMetadata(track);
				if(success)
				{
					m_pipeline->fadeIn();
					changeGaplessState(GaplessState::Playing);
				}

				return success;
			}

			bool changeTrackGapless(const MetaData& track)
			{
				swapPipelines();

				const auto success = changeMetadata(track);
				if(success)
				{
					const auto timeToGo = m_otherPipeline->timeToGo();
					m_pipeline->startDelayedPlayback(timeToGo);

					changeGaplessState(GaplessState::TrackFetched);

					spLog(Log::Develop, this) << "Will start playing in " << timeToGo << "msec";
				}

				return success;
			}

			bool changeTrackImmediatly(const MetaData& track)
			{
				if(m_otherPipeline)
				{
					m_otherPipeline->stop();
				}

				m_pipeline->stop();

				return changeMetadata(track);
			}

			Util::FileSystemPtr m_fileSystem;
			Tagging::TagWriterPtr m_tagWriter;
			MetaData m_currentTrack;

			PipelinePtr m_pipeline, m_otherPipeline;

			std::vector<float> m_spectrumValues;
			QPair<float, float> m_levelValues;

			StreamRecorder::StreamRecorder* m_streamRecorder {nullptr};

			MilliSeconds m_currentPositionMs {0};
			GaplessState m_gaplessState {GaplessState::Stopped};
	};

	Engine::Engine(QObject* parent) :
		QObject {parent} {}

	Engine::~Engine() noexcept = default;

	Engine* createEngine(const Util::FileSystemPtr& fileSystem, const Tagging::TagWriterPtr& tagWriter, QObject* parent)
	{
		return new EngineImpl(fileSystem, tagWriter, parent);
	}
} // namespace