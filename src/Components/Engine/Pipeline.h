/* PlaybackPipeline.h */

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

#ifndef SAYONARA_PLAYER_PIPELINE_H
#define SAYONARA_PLAYER_PIPELINE_H

#include "PipelineExtensions/Broadcasting.h"
#include "PipelineExtensions/PipelineInterfaces.h"
#include "PipelineExtensions/Changeable.h"
#include "PipelineExtensions/StreamRecordable.h"

#include "Utils/Pimpl.h"

#include <QObject>

namespace Engine
{
	class Engine;

	class Pipeline :
		public QObject,
		public PipelineExtensions::Broadcastable,
		public PipelineExtensions::PlaystateController,
		public PipelineExtensions::VolumeController,
		public PipelineExtensions::Changeable,
		public PipelineExtensions::StreamRecordable
	{
		Q_OBJECT
		PIMPL(Pipeline)

		signals:
			void sigAboutToFinishMs(MilliSeconds ms);
			void sigPositionChangedMs(MilliSeconds ms);
			void sigDataAvailable(const QByteArray& data);

		public:
			explicit Pipeline(const QString& name, QObject* parent = nullptr);
			~Pipeline() override;

			bool init(Engine* engine);
			bool prepare(const QString& uri);

			bool hasElement(GstElement* e) const;
			[[nodiscard]] GstState state() const;

			void checkPosition();
			void checkAboutToFinish();

			void setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled);
			[[nodiscard]] bool isLevelVisualizerEnabled() const;
			[[nodiscard]] bool isSpectrumVisualizerEnabled() const;

			void setBroadcastingEnabled(bool b) override;
			[[nodiscard]] bool isBroadcastingEnabled() const override;

			void prepareForRecording() override;
			void finishRecording() override;
			void setRecordingPath(const QString& targetPath) override;

			void fadeIn();
			void fadeOut();

			void startDelayedPlayback(MilliSeconds ms);

			void seekRelative(double percent, MilliSeconds duration);
			void seekAbsoluteMs(MilliSeconds ms);
			void seekRelativeMs(MilliSeconds ms);
			[[nodiscard]] MilliSeconds duration() const;
			[[nodiscard]] MilliSeconds timeToGo() const;

			void setEqualizerBand(int band, int value);

		public slots: // NOLINT(readability-redundant-access-specifiers)
			void play() override;
			void stop() override;
			void pause() override;

		private slots:
			void volumeChanged();
			void muteChanged();
			void speedActiveChanged();
			void sppedChanged();
			void sinkChanged();

		private: // NOLINT(readability-redundant-access-specifiers)
			bool createElements();
			bool addAndLinkElements();
			void configureElements();

			void setVolume(double volume) override;
			[[nodiscard]] double volume() const override;
	};
} // SAYONARA_PLAYER_PIPELINE_H

#endif
