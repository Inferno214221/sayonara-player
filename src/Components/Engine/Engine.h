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
			void sig_data(const QByteArray& data);
			void sig_spectrum_changed();
			void sig_level_changed();

			void sig_metadata_changed(const MetaData& md);
			void sig_duration_changed(const MetaData& md);
			void sig_bitrate_changed(const MetaData& md);
			void sig_cover_data(const QByteArray& data, const QString& mimetype);

			void sig_current_position_changed(MilliSeconds ms);
			void sig_buffering(int progress);
			void sig_track_finished();
			void sig_track_ready();
			void sig_error(const QString& error_message);

		public:
			explicit Engine(QObject* parent=nullptr);
			~Engine();

			void update_bitrate(Bitrate br, GstElement* src);
			void update_duration(GstElement* src);

			void set_track_ready(GstElement* src);
			void set_track_almost_finished(MilliSeconds time2go);
			void set_track_finished(GstElement* src);

			bool is_streamrecroder_recording() const;
			void set_streamrecorder_recording(bool b);

			void set_spectrum(const SpectrumList& vals);
			SpectrumList spectrum() const;

			void set_level(float left, float right);
			QPair<float, float> level() const;

			void set_broadcast_enabled(bool b);
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
			void update_cover(GstElement* src, const QByteArray& data, const QString& mimedata);

			bool change_track(const MetaData& md);

			void set_buffer_state(int progress, GstElement* src);
			void error(const QString& error, const QString& element_name);

		private:
			PipelinePtr init_pipeline(const QString& name);
			bool change_metadata(const MetaData& md);

			bool change_track_crossfading(const MetaData& md);
			bool change_track_gapless(const MetaData& md);
			bool change_track_immediatly(const MetaData& md);

			void set_current_position_ms(MilliSeconds pos_ms);

		private slots:
			void s_gapless_changed();
			void s_streamrecorder_active_changed();

			void cur_pos_ms_changed(MilliSeconds pos_ms);
	};
}

#endif /* GSTENGINE_H_ */
