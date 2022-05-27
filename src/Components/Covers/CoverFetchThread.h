/* CoverFetchThread.h */

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


/*
 * CoverFetchThread.h
 *
 *  Created on: Jun 28, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef COVERFETCHTHREAD_H_
#define COVERFETCHTHREAD_H_

#include "Utils/Pimpl.h"

#include <QObject>
#include <functional>

class QImage;

namespace Cover
{
	class Location;

	class WebCoverFetcher :
		public QObject
	{
		Q_OBJECT
		PIMPL(WebCoverFetcher)

		signals:
			void sigFinished();
			void sigCoverFound(int idx);

		public:
			WebCoverFetcher() = delete;
			WebCoverFetcher(QObject* parent, const Cover::Location& coverLocation, const int requestedCovers);
			virtual ~WebCoverFetcher();

			bool start();
			void stop();

			QString url(int idx) const;
			QPixmap pixmap(int idx) const;

		private slots:
			void imageFetched();
			void contentFetched();

		private:
			bool processNextSearchUrl();
			bool processNextImageUrl();
			bool startNextRequest();
	};
}

#endif /* COVERFETCHTHREAD_H_ */
