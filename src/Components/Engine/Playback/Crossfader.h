/* CrossFader.h */

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

/* CrossFader.h */

#ifndef CROSSFADER_H
#define CROSSFADER_H

#include "Utils/Pimpl.h"

namespace Pipeline
{
	/**
	 * @brief The CrossFader class
	 * @ingroup EngineInterfaces
	 */
	class CrossFader
	{
		PIMPL(CrossFader)

	public:

		enum class FadeMode : unsigned char
		{
			NoFading=0,
			FadeIn,
			FadeOut
		};

		CrossFader();
		~CrossFader();

		/**
		 * @brief get current volume of pipeline
		 * @return value between 0 and 1.0
		 */
		virtual double get_current_volume() const=0;

		/**
		 * @brief set current volume of pipeline
		 * @param vol value between 0 and 1.0
		 */
		virtual void set_current_volume(double vol)=0;

		/**
		 * @brief get fading time in ms
		 * @return fading time in ms
		 */
		MilliSeconds get_fading_time_ms() const;

		/**
		 * @brief start to fade in
		 */
		void fade_in();

		/**
		 * @brief start to fade out
		 */
		void fade_out();

		bool is_fading_out() const;
		bool is_fading_int() const;

		/**
		 * @brief function is called periodically. This function should not be used from outside
		 * TODO
		 */
		void fader_timed_out();


	private:
		CrossFader(const CrossFader& other)=delete;

		void increase_volume();
		void decrease_volume();
		void init_fader();

	protected:
		void	    abort_fader();

		virtual void stop()=0;
		virtual void play()=0;
		virtual void fade_out_handler()=0;
		virtual void fade_in_handler()=0;
	};
}

#endif // CROSSFADER_H
