/* ConvertEngine.h */

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

#ifndef ConvertEngine_H
#define ConvertEngine_H

#include "Components/Engine/AbstractEngine.h"

namespace Pipeline
{
	class Convert;
}

namespace Engine
{
	class Convert :
			public Base
	{
		Q_OBJECT
		PIMPL(Convert)

	public:
		explicit Convert(QObject *parent=nullptr);
		~Convert();

		void set_track_finished(GstElement* src) override;
		bool init() override;

		void play() override;
		void stop() override;
		void pause() override;

		void jump_abs_ms(MilliSeconds pos_ms) override;
		void jump_rel_ms(MilliSeconds ms) override;
		void jump_rel(double percent) override;

		bool change_track(const MetaData&) override;
		bool change_track_by_filename(const QString&) override;


	private slots:
		void cur_pos_ms_changed(MilliSeconds ms);

	protected:
		// methods
		bool change_uri(char* uri) override;
		void configure_target(const MetaData& md);

		// Base interface
	public:
		void update_metadata(const MetaData& md, GstElement* src) override;
	};
}

#endif // ConvertEngine_H


