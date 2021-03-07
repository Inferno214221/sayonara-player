/* CoverLookup.h */

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
 * CoverLookup.h
 *
 *  Created on: Apr 4, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef SAYONARA_COVER_LOOKUP_H
#define SAYONARA_COVER_LOOKUP_H

#include "AbstractCoverLookup.h"
#include "Utils/Pimpl.h"
#include "Utils/CoverUtils.h"

#include <QList>
#include <any>

class QPixmap;
namespace Cover
{
	class Location;

	/**
	 * @brief The CoverLookup class
	 * @ingroup Covers
	 */
	class Lookup :
		public LookupBase
	{
		Q_OBJECT
		PIMPL(Lookup)

		public:
			Lookup(const Location& coverLocation, int n_covers, QObject* parent);
			~Lookup() override;

			QList<QPixmap> pixmaps() const;
			Util::Covers::Source source() const;

			template<typename T>
			void setUserData(const T t)
			{
				mUserData = t;
			}

			template<typename T>
			auto userData() const -> T
			{
				try
				{
					return std::any_cast<T>(mUserData);
				}
				catch(...)
				{
					return T {};
				}
			}

		public slots:
			void start();
			void stop() override;

		private slots:
			void coverFound(int idx);
			void threadFinished();
			void extractorFinished();

		private:
			bool fetchFromDatabase();
			void fetchFromExtractor();
			bool fetchFromWWW();

			bool startNewThread(const Location& coverLocation);
			bool addNewCover(const QPixmap& pixmap, bool save);

			void done(bool success);

			std::any mUserData;
	};
}
#endif /* SAYONARA_COVER_LOOKUP_H */
