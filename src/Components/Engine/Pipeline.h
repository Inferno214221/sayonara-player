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

#include "PipelineExtensions/Changeable.h"
#include "PipelineExtensions/Fadeable.h"
#include "PipelineExtensions/DelayedPlayable.h"
#include "PipelineExtensions/Broadcaster.h"
#include "Utils/Pimpl.h"

#include <QObject>

namespace Engine
{
	class Engine;

	/**
	 * @brief The PlaybackPipeline class
	 * @ingroup Engine
	 */
	class Pipeline :
		public QObject,
		public PipelineExtensions::Fadeable,
		public PipelineExtensions::Changeable,
		public PipelineExtensions::DelayedPlayable,
		public PipelineExtensions::BroadcastDataReceiver
	{
		Q_OBJECT
		PIMPL(Pipeline)

		signals:
			void sigAboutToFinishMs(MilliSeconds ms);
			void sigPositionChangedMs(MilliSeconds ms);
			void sigDataAvailable(const QByteArray& data);

		public:
			explicit Pipeline(const QString& name, QObject* parent=nullptr);
			~Pipeline() override;

			bool init(Engine* engine);
			bool prepare(const QString& uri);

			void setRawData(const QByteArray& data) override; // BroadcastDataReceiver
			void setInternalVolume(double volume) override; // Crossfader
			double internalVolume() const override;      // Crossfader

			bool hasElement(GstElement* e) const;
			GstState state() const;

			void checkPosition();
			void checkAboutToFinish();

			void enableVisualizer(bool b);
			void enableBroadcasting(bool b);

			void record(bool b);
			void setRecordingPath(const QString& session_path);

			MilliSeconds	durationMs() const;
			MilliSeconds	positionMs() const;
			MilliSeconds	timeToGo() const;


		public slots:
			void play() override;	// Crossfader
			void stop() override;	// Crossfader
			void pause();

			void setEqualizerBand(int band_name, int val);

			NanoSeconds seekRelative(double percent, NanoSeconds ns);
			NanoSeconds seekAbsolute(NanoSeconds ns );


		protected slots:
			void volumeChanged();
			void showVisualizerChanged();
			void muteChanged();
			void speedActiveChanged();
			void sppedChanged();
			void sinkChanged();


		private:
			bool			createElements();
			GstElement*		createSink(const QString& name);

			bool			addAndLinkElements();
			void			configureElements();

			MilliSeconds	getAboutToFinishTime() const;

			void			setPositionElement(GstElement* element);
			GstElement*		positionElement();

			void			getFadeInHandler() override;		// Crossfader
			void			getFadeOutHandler() override;	// Crossfader
	};
}

#endif
