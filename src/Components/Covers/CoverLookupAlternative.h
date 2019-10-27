/* CoverLookupAlternative.h */

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

#ifndef COVERLOOKUPALTERNATIVE_H
#define COVERLOOKUPALTERNATIVE_H

#include "AbstractCoverLookup.h"
#include "Utils/Pimpl.h"

namespace Cover
{
	class Location;

	/**
	 * @brief The CoverLookupAlternative class
	 * @ingroup Covers
	 */
	class AlternativeLookup :
			public LookupBase
	{
		Q_OBJECT
		PIMPL(AlternativeLookup)

	signals:
		void sig_cover_changed(Cover::Location& cl);
		void sig_coverfetchers_changed();

	private:
		void go(const Cover::Location& cl);

	public:
		AlternativeLookup(const Cover::Location& cl, int n_covers, bool silent, QObject* parent);
		~AlternativeLookup();

		void start();
		void start(const QString& cover_fetcher_identifier);

		void start_text_search(const QString& search_term);
		void start_text_search(const QString& search_term, const QString& cover_fetcher_identifier);

		void stop() override;
		void reset();

		bool save(const QPixmap& pm, bool save_to_library);
		bool is_running() const;

		/**
		 * @brief silent results that the cover is not stored
		 * productively. The AlternativeCoverFetcher will
		 * save the cover to a temporary path which can be re-
		 * trieved by Cover::Location::alternative_path()
		 */
		bool is_silent() const;

		enum SearchMode
		{
			Fulltext,
			Default
		};

		QStringList active_coverfetchers(SearchMode mode) const;

		private slots:
			void started();
			void finished(bool success);
			void cover_found(const QPixmap& pm);
			void coverfetchers_changed();
	};
}

#endif // COVERLOOKUPALTERNATIVE_H
