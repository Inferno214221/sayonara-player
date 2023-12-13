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

#ifndef GSTPLAYBACKPIPELINE_H_
#define GSTPLAYBACKPIPELINE_H_

#include "PipelineExtensions/Broadcasting.h"
#include "PipelineExtensions/Changeable.h"
#include "PipelineExtensions/Fadeable.h"
#include "PipelineExtensions/DelayedPlayable.h"
#include "PipelineExtensions/Broadcasting.h"
#include "PipelineExtensions/PositionAccessible.h"
#include "PipelineExtensions/EqualizerAccesible.h"
#include "PipelineExtensions/StreamRecordable.h"
#include "Utils/Pimpl.h"

#include <QObject>

namespace Engine
{
	class Engine;

	class Pipeline :
		public QObject,
		public PipelineExtensions::Broadcastable,
		public PipelineExtensions::Fadeable,
		public PipelineExtensions::Changeable,
		public PipelineExtensions::DelayedPlayable,
		public PipelineExtensions::PositionAccessible,
		public PipelineExtensions::EqualizerAccessible,
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
			GstState state() const;

			void checkPosition();
			void checkAboutToFinish();

			void setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled);
			bool isLevelVisualizerEnabled() const;
			bool isSpectrumVisualizerEnabled() const;

			void setBroadcastingEnabled(bool b) override;
			[[nodiscard]] bool isBroadcastingEnabled() const override;

			void prepareForRecording() override;
			void finishRecording() override;
			void setRecordingPath(const QString& targetPath) override;

			MilliSeconds timeToGo() const;
			
		public slots:
			void play() override;    // Crossfader
			void stop() override;    // Crossfader
			void pause();

		private slots:
			void volumeChanged();
			void muteChanged();
			void speedActiveChanged();
			void sppedChanged();
			void sinkChanged();

		private:
			bool createElements();
			bool addAndLinkElements();
			void configureElements();

			void postProcessFadeIn() override;    // Crossfader
			void postProcessFadeOut() override;    // Crossfader
			void setInternalVolume(double volume) override;    // Crossfader
			double internalVolume() const override;            // Crossfader

			GstElement* positionElement() const override;
			GstElement* equalizerElement() const override;
	};
}

#endif
