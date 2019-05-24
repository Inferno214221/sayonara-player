/* PlaybackPipeline.h */

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

#ifndef GSTPLAYBACKPIPELINE_H_
#define GSTPLAYBACKPIPELINE_H_

#include "Components/Engine/Playback/PlaybackEngine.h"
#include "ChangeablePipeline.h"
#include "Crossfader.h"
#include "DelayedPlayHandler.h"
#include "Utils/Pimpl.h"

namespace Pipeline
{
	/**
	 * @brief The PlaybackPipeline class
	 * @ingroup Engine
	 */
	class Playback :
			public QObject,
			public CrossFader,
			public Changeable,
			public DelayedPlayHandler
	{
		Q_OBJECT
		PIMPL(Playback)

	signals:
		void sig_about_to_finish(MilliSeconds ms);
		void sig_pos_changed_ms(MilliSeconds ms);
		void sig_data(Byte* data, uint64_t size);

	public:
		explicit Playback(Engine::Playback* engine, QObject *parent=nullptr);
		virtual ~Playback();

		bool init(GstState state=GST_STATE_NULL);
		bool set_uri(gchar* uri);

		void enable_broadcasting(bool b);
		void enable_streamrecorder(bool b);
		void set_streamrecorder_path(const QString& session_path);

		void enable_visualizer(bool b);

		// Crossfader
		void set_current_volume(double volume) override;
		double get_current_volume() const override;



		GstState	get_state() const;


		MilliSeconds get_time_to_go() const;

		void check_about_to_finish();
		void update_duration_ms(MilliSeconds duration_ms, GstElement* src);
		void refresh_position();

		MilliSeconds	get_duration_ms() const;
		MilliSeconds	get_position_ms() const;

		bool			has_element(GstElement* e) const;

		void set_data(Byte* data, uint64_t size);

	public slots:

		void play() override;	// Crossfader
		void stop() override;	// Crossfader
		void pause();

		void set_eq_band(int band_name, int val);

		NanoSeconds seek_rel(double percent, NanoSeconds ref_ns);
		NanoSeconds seek_abs(NanoSeconds ns );

	protected slots:
		void s_vol_changed();
		void s_show_visualizer_changed();
		void s_mute_changed();
		void s_speed_active_changed();
		void s_speed_changed();
		void s_sink_changed();

	private:
		bool			create_elements();
		bool			create_source(gchar* uri);
		void			remove_source();
		GstElement*		create_sink(const QString& name);

		bool			add_and_link_elements();
		bool			configure_elements();

		bool			init_streamrecorder();

		GstElement* pipeline() const override;	// Changeable


		MilliSeconds	get_about_to_finish_time() const;

		void			fade_in_handler() override;		// Crossfader
		void			fade_out_handler() override;	// Crossfader



	};
}

#endif
