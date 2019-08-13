/* PlaybackEngine.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "SoundOutReceiver.h"
#include <QObject>
#include <gst/gst.h>

#include <QImage>

class SpectrumReceiver;
class LevelReceiver;

namespace StreamRecorder
{
	class StreamRecorder;
}

namespace Engine
{
	class Extractor : public QObject
	{
		Q_OBJECT

		signals:
			void sig_finished();

		private:
			QByteArray mData;
			QString mMime;

		public:
			QImage mImage;
			Extractor(const QByteArray& data, const QString& mime);
			~Extractor();

		public slots:
			void start();
	};

	class Pipeline;
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
			void sig_data(const unsigned char* data, uint64_t n_bytes);

			void sig_metadata_changed(const MetaData& md);
			void sig_duration_changed(const MetaData& md);
			void sig_bitrate_changed(const MetaData& md);
			void sig_cover_changed(const QImage& img);

			void sig_current_position_changed(MilliSeconds ms);
			void sig_buffering(int progress);
			void sig_track_finished();
			void sig_track_ready();
			void sig_error(const QString& error_message);

		public:
			explicit Engine(QObject* parent=nullptr);
			~Engine();

			bool init();

			void update_bitrate(Bitrate br, GstElement* src);
			void update_duration(GstElement* src);

			void set_track_ready(GstElement* src);
			void set_track_almost_finished(MilliSeconds time2go);
			void set_track_finished(GstElement* src);

			bool is_streamrecroder_recording() const;
			void set_streamrecorder_recording(bool b);

			void set_spectrum(const SpectrumList& vals);
			void add_spectrum_receiver(SpectrumReceiver* receiver);

			void set_level(float left, float right);
			void add_level_receiver(LevelReceiver* receiver);

			void set_n_sound_receiver(int num_sound_receiver);

			void set_equalizer(int band, int value);

			MetaData current_track() const;


		public slots:
			void play();
			void stop();
			void pause();

			void jump_abs_ms(MilliSeconds pos_ms);
			void jump_rel_ms(MilliSeconds pos_ms);
			void jump_rel(double percent);
			void update_metadata(const MetaData& md, GstElement* src);
			void update_cover(const QByteArray& data, const QString& mimedata);

			bool change_track(const MetaData& md);

			void set_buffer_state(int progress, GstElement* src);
			void error(const QString& error);

		private:
			bool init_pipeline(Pipeline** pipeline, const QString& name);
			bool change_metadata(const MetaData& md);

			bool change_track_crossfading(const MetaData& md);
			bool change_track_gapless(const MetaData& md);
			bool change_track_immediatly(const MetaData& md);

			void set_current_position_ms(MilliSeconds pos_ms);

		private slots:
			void s_gapless_changed();
			void s_streamrecorder_active_changed();

			void cur_pos_ms_changed(MilliSeconds pos_ms);

			void worker_finished();
	};
}

#endif /* GSTENGINE_H_ */
