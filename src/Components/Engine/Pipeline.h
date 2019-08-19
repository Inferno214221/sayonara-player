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

#include "PipelineExtensions/Changeable.h"
#include "PipelineExtensions/Fadeable.h"
#include "PipelineExtensions/DelayedPlayable.h"
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
		public PipelineExtensions::DelayedPlayable
	{
		Q_OBJECT
		PIMPL(Pipeline)

		signals:
			void sig_about_to_finish(MilliSeconds ms);
			void sig_pos_changed_ms(MilliSeconds ms);
			void sig_data(Byte* data, uint64_t size);

		public:
			explicit Pipeline(const QString& name, QObject *parent=nullptr);
			~Pipeline() override;

			bool init(Engine* engine, GstState state=GST_STATE_NULL);
			bool prepare(const QString& uri);

			void set_data(Byte* data, uint64_t size);
			void set_internal_volume(double volume) override; // Crossfader
			double get_internal_volume() const override;      // Crossfader

			bool has_element(GstElement* e) const;
			GstState state() const;

			void check_position();
			void check_about_to_finish();

			void enable_visualizer(bool b);
			void enable_broadcasting(bool b);

			void record(bool b);
			void set_recording_path(const QString& session_path);

			MilliSeconds	duration_ms() const;
			MilliSeconds	position_ms() const;
			MilliSeconds	time_to_go() const;


		public slots:
			void play() override;	// Crossfader
			void stop() override;	// Crossfader
			void pause();

			void set_equalizer_band(int band_name, int val);

			NanoSeconds seek_rel(double percent, NanoSeconds ref_ns);
			NanoSeconds seek_abs(NanoSeconds ns );


		protected slots:
			void s_volume_changed();
			void s_show_visualizer_changed();
			void s_mute_changed();
			void s_speed_active_changed();
			void s_speed_changed();
			void s_sink_changed();


		private:
			bool			create_elements();
			bool			create_source(const QString& uri);
			void			remove_source();
			GstElement*		create_sink(const QString& name);

			bool			add_and_link_elements();
			void			configure_elements();

			MilliSeconds	get_about_to_finish_time() const;

			void			set_position_element(GstElement* element);
			GstElement*		position_element();

			GstElement*		pipeline() const override;	// Changeable

			void			fade_in_handler() override;		// Crossfader
			void			fade_out_handler() override;	// Crossfader
	};
}

#endif
