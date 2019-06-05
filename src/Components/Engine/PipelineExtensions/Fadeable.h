/* CrossFader.h */

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

/* CrossFader.h */

#ifndef CROSSFADER_H
#define CROSSFADER_H

#include "Utils/Pimpl.h"
#include <QThread>


namespace PipelineExtensions
{
	class Fadeable;
	class CrossFadeableTimer :
		public QObject
	{
		Q_OBJECT
		PIMPL(CrossFadeableTimer)

		public:
			CrossFadeableTimer(Fadeable* fadeable);
			~CrossFadeableTimer();
			void start(MilliSeconds ms);
			void stop();

		private slots:
			void timed_out();
	};


	/**
	 * @brief The CrossFader class
	 * @ingroup EngineInterfaces
	 */
	class Fadeable
	{
		PIMPL(Fadeable)

		public:

			enum class FadeMode : unsigned char
			{
				NoFading=0,
				FadeIn,
				FadeOut
			};

			Fadeable();
			Fadeable(const Fadeable& other)=delete;

			virtual ~Fadeable();

		public:
			/**
			 * @brief get fading time in ms. This is useful to
			 * calculate the beginning of the next track
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


		private:
			bool init_fader(FadeMode mode);

		protected:
			virtual void stop()=0;
			virtual void play()=0;

			/**
			 * @brief Some additional stuff the parent class wants to do
			 * when fading out
			 */
			virtual void fade_out_handler()=0;

			/**
			 * @brief Some additional stuff the parent class wants to do
			 * when fading in
			 */
			virtual void fade_in_handler()=0;

			/**
			 * @brief get current volume of pipeline
			 * @return value between 0 and 1.0
			 */
			virtual double get_internal_volume() const=0;

			/**
			 * @brief set current volume of pipeline
			 * @param vol value between 0 and 1.0
			 */
			virtual void set_internal_volume(double vol)=0;

			/**
			 * @brief Stops the current fader process
			 */
			void abort_fader();


		private:
			friend class CrossFadeableTimer;
			void timed_out();
	};
}

#endif // CROSSFADER_H
