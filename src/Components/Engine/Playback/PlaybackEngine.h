/* PlaybackEngine.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "Components/Engine/AbstractEngine.h"
#include "SoundOutReceiver.h"

class QTimer;
class QString;

class SpectrumReceiver;
class LevelReceiver;

namespace StreamRecorder
{
	class StreamRecorder;
}

namespace Pipeline
{
	class Playback;
}

namespace Engine
{
	/**
	 * @brief The PlaybackEngine class
	 * @ingroup Engine
	 */
	class Playback :
			public Base
	{
		Q_OBJECT
		PIMPL(Playback)

	signals:
		void sig_data(const unsigned char* data, uint64_t n_bytes);

	public:
		explicit Playback(QObject* parent=nullptr);
		~Playback();

		bool init() override;

		void update_bitrate(Bitrate br, GstElement* src) override;
		void update_duration(MilliSeconds duration_ms, GstElement* src) override;

		void set_track_ready(GstElement* src) override;
		void set_track_almost_finished(MilliSeconds time2go) override;
		void set_track_finished(GstElement* src) override;

		bool is_streamrecroder_recording() const;
		void set_streamrecorder_recording(bool b);

		int get_spectrum_bins() const;
		void set_spectrum(const SpectrumList& vals);
		void add_spectrum_receiver(SpectrumReceiver* receiver);

		void set_level(float left, float right);
		void add_level_receiver(LevelReceiver* receiver);

		void set_n_sound_receiver(int num_sound_receiver);

		void set_equalizer(int band, int value);


	public slots:
		void play() override;
		void stop() override;
		void pause() override;

		void jump_abs_ms(MilliSeconds pos_ms) override;
		void jump_rel_ms(MilliSeconds pos_ms) override;
		void jump_rel(double percent) override;
		void update_metadata(const MetaData& md, GstElement* src) override;
		void update_cover(const QImage& img, GstElement* src) override;

		bool change_track(const MetaData& md) override;

		void set_buffer_state(int progress, GstElement* src) override;

	private:
		bool init_pipeline(Pipeline::Playback** pipeline);

		bool change_uri(const QString& uri) override;
		bool change_metadata(const MetaData& md) override;

		bool change_track_crossfading(const MetaData& md);
		bool change_track_gapless(const MetaData& md);
		bool change_track_immediatly(const MetaData& md);


	private slots:
		void s_gapless_changed();
		void s_streamrecorder_active_changed();

		void cur_pos_ms_changed(MilliSeconds pos_ms);
	};
}

#endif /* GSTENGINE_H_ */
