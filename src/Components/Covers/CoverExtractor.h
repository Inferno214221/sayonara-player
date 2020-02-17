/* CoverExtractor.h */

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



#ifndef COVEREXTRACTOR_H
#define COVEREXTRACTOR_H

#include <QObject>
#include "CoverUtils.h"
#include "Utils/Pimpl.h"

class QPixmap;

namespace Cover
{
	class Location;
	class Extractor : public QObject
	{
		Q_OBJECT
		PIMPL(Extractor)

		signals:
			void sigFinished();

		public:
			Extractor(const Cover::Location& cl, QObject* parent);
			~Extractor();

			QPixmap pixmap() const;
			Cover::Source source() const;

		public slots:
			void start();
	};
}

#endif // COVEREXTRACTOR_H
