/* PlaybackEngine.h */

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

#ifndef GSTPLAYBACKENGINE_H_
#define GSTPLAYBACKENGINE_H_

#include "Utils/Pimpl.h"
#include "Interfaces/Engine/AudioDataReceiverInterface.h"
#include <QObject>
#include <gst/gst.h>

#include <QImage>

namespace StreamRecorder
{
	class StreamRecorder;
}

namespace Engine
{
	class SpectrumReceiver;
	class LevelReceiver;

	class Pipeline;
	using PipelinePtr=std::shared_ptr<Pipeline>;
	/**
	 * @brief The PlaybackEngine class
	 * @ingroup Engine
	 */
	class Engine :
			public QObject
	{

		Q_OBJECT
		PIMPL(Engine)

	private:
		/**
		 * @brief The GaplessState enum
		 * @ingroup Engine
		 */
		enum class GaplessState : uint8_t
		{
			NoGapless=0,		// no gapless enabled at all
			AboutToFinish,		// the phase when the new track is already displayed but not played yet
			TrackFetched,		// track is requested, but no yet there
			Playing,			// currently playing
			Stopped
		};

		signals:
			void sigDataAvailable(const QByteArray& data);
			void sigSpectrumChanged();
			void sigLevelChanged();

			void sigMetadataChanged(const MetaData& md);
			void sigDurationChanged(const MetaData& md);
			void sigBitrateChanged(const MetaData& md);
			void sigCoverDataAvailable(const QByteArray& data, const QString& mimetype);

			void sigCurrentPositionChanged(MilliSeconds ms);
			void sigBuffering(int progress);
			void sigTrackFinished();
			void sigTrackReady();
			void sigError(const QString& error_message);

		public:
			explicit Engine(QObject* parent=nullptr);
			~Engine();

			void updateBitrate(Bitrate br, GstElement* src);
			void updateDuration(GstElement* src);

			void setTrackReady(GstElement* src);
			void setTrackAlmostFinished(MilliSeconds time2go);
			void setTrackFinished(GstElement* src);

			bool isStreamRecorderRecording() const;
			void setStreamRecorderRecording(bool b);

			void setSpectrum(const SpectrumList& vals);
			SpectrumList spectrum() const;

			void setLevel(float left, float right);
			QPair<float, float> level() const;

			void setVisualizerEnabled(bool b);
			void setBroadcastEnabled(bool b);
			void setEqualizer(int band, int value);

			MetaData currentTrack() const;

		public slots:
			void play();
			void stop();
			void pause();

			void jumpAbsMs(MilliSeconds pos_ms);
			void jumpRelMs(MilliSeconds pos_ms);
			void jumpRel(double percent);
			void updateMetadata(const MetaData& md, GstElement* src);
			void updateCover(GstElement* src, const QByteArray& data, const QString& mimedata);

			bool changeTrack(const MetaData& md);

			void setBufferState(int progress, GstElement* src);
			void error(const QString& error, const QString& elementName);

		private:
			PipelinePtr initPipeline(const QString& name);
			bool changeMetadata(const MetaData& md);

			bool changeTrackCrossfading(const MetaData& md);
			bool changeTrackGapless(const MetaData& md);
			bool changeTrackImmediatly(const MetaData& md);

			void setCurrentPositionMs(MilliSeconds pos_ms);

		private slots:
			void gaplessChanged();
			void streamrecorderActiveChanged();
			void currentPositionChanged(MilliSeconds pos_ms);
	};
}

#endif /* GSTENGINE_H_ */
