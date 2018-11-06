/* Engine.h */

/* Copyright (C) 2012  Lucio Carreras
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

#ifndef ENGINE_H_
#define ENGINE_H_

#include <QObject>

#include "Components/Engine/gstfwd.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

class QImage;
namespace Engine
{
	/**
	 * @brief The EngineName enum
	 * @ingroup Engine
	 */
	enum class Name : int8_t
	{
		Undefined=-1,
		EngineHandler=0,
		PlaybackEngine=1,
		ConvertEngine=2
	};

	/**
	 * @brief The Engine class
	 * @ingroup Engine
	 */
	class Base :
			public QObject,
			public SayonaraClass
	{
		Q_OBJECT
		PIMPL(Base)

	signals:
		void sig_md_changed(const MetaData& md);
		void sig_dur_changed(const MetaData& md);
		void sig_br_changed(const MetaData& md);
		void sig_cover_changed(const QImage& img);

		void sig_pos_changed_ms(MilliSeconds pos_ms);
		void sig_buffering(int progress);

		void sig_track_ready();
		void sig_track_almost_finished(MilliSeconds time2go);
		void sig_track_finished();

		void sig_error(const QString& message);


	public:
		explicit Base(Name name, QObject* parent=nullptr);
		virtual ~Base();

		virtual Name		name() const final;

		virtual bool		init()=0;

		virtual void		update_metadata(const MetaData& md, GstElement* src);
		virtual void		update_cover(const QImage& img, GstElement* src);
		virtual void		update_duration(MilliSeconds duration_ms, GstElement* src);
		virtual void		update_bitrate(Bitrate br, GstElement* src);

		virtual void		set_track_ready(GstElement* src);
		virtual void        set_track_almost_finished(MilliSeconds time2go);
		virtual void		set_track_finished(GstElement* src);

		virtual void		set_buffer_state(int percent, GstElement* src);

		virtual bool        change_track(const MetaData& md);
		virtual bool        change_track_by_filename(const QString& filepath);


	public slots:
		virtual void play()=0;
		virtual void stop();
		virtual void pause()=0;
		virtual void error(const QString& error);

		virtual void jump_abs_ms(MilliSeconds ms)=0;
		virtual void jump_rel_ms(MilliSeconds ms)=0;
		virtual void jump_rel(double ms)=0;


	protected:
		virtual bool        change_uri(char* uri)=0;

		// if the current track is changed, this routine should be called within the
		// change_track method. This method has to be called explicitly, because its
		// position within the change_track method is too specific
		virtual bool        change_metadata(const MetaData& md);

		const MetaData&		metadata() const;

		void				set_current_position_ms(MilliSeconds pos_ms);
		MilliSeconds		current_position_ms() const;
	};
}

#endif

