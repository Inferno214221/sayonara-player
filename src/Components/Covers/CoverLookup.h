/* CoverLookup.h */

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
 * CoverLookup.h
 *
 *  Created on: Apr 4, 2011
 *      Author: Lucio Carreras
 */

#ifndef COVERLOOKUP_H_
#define COVERLOOKUP_H_

#include "AbstractCoverLookup.h"
#include "Utils/Pimpl.h"

#include <QPixmap>
#include <QList>

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

		Lookup(QObject* parent=nullptr, int n_covers=1);
		~Lookup();

		/**
		 * @brief fetches cover for a CoverLocation.
		 *   1. Looks up CoverLocation::cover_path
		 *   2. Looks up CoverLocation::local_paths
		 *   3. Starts a CoverFetchThread
		 * @param cl CoverLocation of interest
		 * @return always true
		 */
		bool fetch_cover(const Location& cl, bool also_www=true);

		/**
		 * @brief not implemented
		 */
		void stop() override;

		bool is_thread_running() const;

		void set_user_data(void* data);
		void* take_user_data();

		QList<QPixmap> pixmaps() const;
		QList<QPixmap> take_pixmaps();


	private:
		/**
		 * @brief Starts a new CoverFetchThread
		 * @param cl CoverLocation object
		 */
		bool start_new_thread(const Location& cl);

		bool add_new_cover(const QPixmap& pm, const QString& hash);
		bool add_new_cover(const QPixmap& pm);


	private slots:
		/**
		 * @brief called when CoverFetchThread has found cover
		 * @param cl
		 */
		void cover_found(int idx);

		/**
		 * @brief called when CoverFetchThread has finished
		 */
		void finished(bool);
	};

	/**
	 * @brief CoverLookupPtr
	 * @ingroup Covers
	 */
	using LookupPtr=std::shared_ptr<Lookup>;

}
#endif /* COVERLOOKUP_H_ */
