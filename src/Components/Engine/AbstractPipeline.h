/* GSTPipeline.h */

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

#ifndef GSTPIPELINE_H
#define GSTPIPELINE_H

#include "Utils/Pimpl.h"

#include <QObject>

#include <gst/gst.h>
#include <gst/gstbuffer.h>

namespace Engine
{
	class Base;
}

namespace Pipeline
{
	/**
	 * @brief The GSTFileMode enum
	 * @ingroup Engine
	 */
	enum class GSTFileMode : uint8_t
	{
		File,
		Http
	};

	/**
	 * @brief The AbstractPipeline class
	 * @ingroup Engine
	 */
	class Base :
		public QObject
	{
		Q_OBJECT
		PIMPL(Base)

		signals:
			void sig_duration_changed();

		protected:
			virtual bool create_elements()=0;
			virtual bool add_and_link_elements()=0;
			virtual bool configure_elements()=0;

			virtual MilliSeconds get_about_to_finish_time() const;
			void set_about_to_finish(bool b);

		signals:
			void sig_finished();
			void sig_about_to_finish(MilliSeconds ms);
			void sig_pos_changed_ms(MilliSeconds ms);
			void sig_data(Byte*, uint64_t);


		public slots:
			virtual void play();
			virtual void pause();
			virtual void stop();


		public:
			Base(QString name, Engine::Base* engine, QObject* parent=nullptr);
			virtual ~Base();

			virtual GstElement* get_source() const=0;
			virtual bool		init(GstState state=GST_STATE_READY);
			virtual GstElement* pipeline() const;
			virtual GstState	get_state();
			virtual void		refresh_position();

			virtual void			finished();
			virtual void			check_about_to_finish();
			virtual MilliSeconds	get_time_to_go() const;
			virtual void			set_data(uchar* data, uint64_t size);

			virtual bool			set_uri(gchar* uri);

			void					update_duration_ms(MilliSeconds duration_ms, GstElement* src);
			virtual MilliSeconds	get_duration_ms() const final ;
			virtual MilliSeconds	get_source_position_ms() const final;
			virtual MilliSeconds	get_pipeline_position_ms() const final;

			bool					has_element(GstElement* e) const;
	};
}

#endif // GSTPIPELINE_H
