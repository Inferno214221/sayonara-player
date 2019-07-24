/* CoverLookup.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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
#include "CoverUtils.h"

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

		Lookup(const Location& cl, int n_covers, QObject* parent);
		~Lookup() override;


		/**
		 * @brief Stop the Cover::FetchThread if running and
		 * retrieve the sig_finished signal
		 * If no Cover::FetchThread is running, nothing will happen
		 */
		void stop() override;

		/**
		 * @brief indicates if the Cover::FetchThread is running
		 * @return
		 */
		bool is_thread_running() const;

		/**
		 * @brief Set some custom data you can retrieve later
		 * @param data
		 */
		void set_user_data(void* data);

		/**
		 * @brief Fetch your custom data again
		 * @return
		 */
		void* user_data();

		/**
		 * @brief Get a copy of all pixmaps that where fetched
		 * @return
		 */
		QList<QPixmap> pixmaps() const;

		Cover::Source source() const;

	private:

		bool fetch_from_database();
		bool fetch_from_extractor();
		bool fetch_from_www();


		bool start_extractor(const Location& cl);
		/**
		 * @brief Starts a new CoverFetchThread
		 * @param cl CoverLocation object
		 */
		bool start_new_thread(const Location& cl);

		bool add_new_cover(const QPixmap& pm, bool save);

		void emit_finished(bool success);

	public slots:
		void start();

	private slots:
		/**
		 * @brief called when CoverFetchThread has found cover
		 * @param cl
		 */
		void cover_found(int idx);

		/**
		 * @brief called when CoverFetchThread has finished
		 */
		void thread_finished(bool);

		void extractor_finished();
	};


	/**
	 * @brief CoverLookupPtr
	 * @ingroup Covers
	 */
	using LookupPtr=std::shared_ptr<Lookup>;

}
#endif /* COVERLOOKUP_H_ */
