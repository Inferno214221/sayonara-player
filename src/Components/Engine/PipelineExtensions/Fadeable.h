/* CrossFader.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

	/**
	 * @brief The CrossFadeableTimer class
	 * @ingroup EngineInterfaces
	 */
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
			void timedOut();
	};

	/**
	 * @brief The CrossFader class
	 * @ingroup EngineInterfaces
	 */
	class Fadeable
	{
		PIMPL(Fadeable)

		public:

			enum class FadeMode :
				unsigned char
			{
				NoFading = 0,
				FadeIn,
				FadeOut
			};

			Fadeable();
			Fadeable(const Fadeable& other) = delete;

			virtual ~Fadeable();

			/**
			 * @brief get fading time in ms. This is useful to
			 * calculate the beginning of the next track
			 * @return fading time in ms
			 */
			MilliSeconds fadingTimeMs() const;

			/**
			 * @brief start to fade in
			 */
			void fadeIn();

			/**
			 * @brief start to fade out
			 */
			void fadeOut();

		private:
			bool initFader(FadeMode mode);

		protected:
			virtual void stop() = 0;
			virtual void play() = 0;

			/**
			 * @brief Some additional stuff the implementation class wants to do
			 * when fading out
			 */
			virtual void postProcessFadeOut() = 0;

			/**
			 * @brief Some additional stuff the implementation class wants to do
			 * when fading in
			 */
			virtual void postProcessFadeIn() = 0;

			/**
			 * @brief get current volume of pipeline
			 * @return value between 0 and 1.0
			 */
			virtual double volume() const = 0;

			/**
			 * @brief set current volume of pipeline
			 * @param vol value between 0 and 1.0
			 */
			virtual void setVolume(double vol) = 0;

			/**
			 * @brief Stops the current fader process
			 */
			void abortFader();

		private:
			friend class CrossFadeableTimer;
			void timedOut();
	};
}

#endif // CROSSFADER_H
