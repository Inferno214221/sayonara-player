/* CoverFetchThread.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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


/*
 * CoverFetchThread.h
 *
 *  Created on: Jun 28, 2011
 *      Author: Lucio Carreras
 */

#ifndef COVERFETCHTHREAD_H_
#define COVERFETCHTHREAD_H_

#include <QObject>
#include "Utils/Pimpl.h"

class QImage;
class AsyncWebAccess;

namespace Cover
{
	class Location;


	/**
	 * @brief The CoverFetchThread class, This is not a real QThread class, but behaves like one because of AsyncWebAccess
	 * @ingroup Covers
	 */
	class FetchThread :
			public QObject
	{
		Q_OBJECT
		PIMPL(FetchThread)

	signals:
		/**
		 * @brief emitted, when thread has finished
		 * @param b true, if couvers could be fetched. false else
		 */
		void sig_finished(bool b);

		/**
		 * @brief emitted, when covers has been found
		 * @param cl CoverLocation including the local cover path
		 */
		void sig_cover_found(int idx);


	public:
		FetchThread()=delete;
		FetchThread(QObject* parent, const Cover::Location& cl, const int n_covers);
		virtual ~FetchThread();

		/**
		 * @brief start fetching covers, if the url does not contain "google",
		 *   a direct link to an image is assumed and will be downloaded directly
		 * @return always true
		 */
		bool start();

		/**
		 * @brief fetch next cover
		 * @return false, if there are no more covers to fetch
		 */
		bool fetch_next_cover();

		/**
		 * @brief stops the current search
		 */
		void stop();

		QPixmap pixmap(int idx) const;


	private slots:
		/**
		 * @brief A single image has been fetched (reached when _n_covers was set to 1),
		 *   calls save_and_emit_image
		 * @param success indicates if image could be fetched successfully
		 */
		void single_image_fetched();

		/**
		 * @brief multi_image_fetched (reached when _n_covers was set to > 1),
		 *   calls save_and_emit_image for first image;
		 * @param success indicates if images could be fetched successfully
		 */
		void multi_image_fetched();

		/**
		 * @brief The website content has been fetched
		 * @param success indicates if content could be fetched
		 */
		void content_fetched();

	private:
		void emit_finished(bool success);
	};
}

#endif /* COVERFETCHTHREAD_H_ */
