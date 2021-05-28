/* CoverFetchManager.h */

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

#ifndef COVERFETCHMANAGER_H
#define COVERFETCHMANAGER_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QList>
#include <QObject>

namespace Cover
{
	namespace Fetcher
	{
		class Base;
		class Url;

		using CoverFetcherPtr = std::shared_ptr<Cover::Fetcher::Base>;
		using CoverFetcherList = QList<CoverFetcherPtr>;

		/**
		 * @brief Retrieve Download Urls for Cover Searcher.
		 * CoverFetcherInterface can be registered, so for
		 * example a last.fm cover fetcher via the register_cover_fetcher
		 * method. A specific CoverFetcherInterface may be retrieved by using
		 * the get_coverfetcher method.
		 * @ingroup Covers
		 */
		class Manager :
			public QObject
		{
			Q_OBJECT
			SINGLETON(Manager)
			PIMPL(Manager)

			public:
				/**
				 * @brief Register a cover fetcher. Per default
				 * there is one for Discogs, last.fm and Google
				 * @param fetcher an instance of a CoverFetcherInterface
				 */
				void registerCoverFetcher(CoverFetcherPtr fetcher);

				/**
				 * @brief get urls for a artist search query
				 * @param artist name
				 * @return list of urls
				 */
				QList<Url> artistAddresses(const QString& artist) const;

				/**
				 * @brief get urls for a album search query
				 * @param artist artist name
				 * @param album album name
				 * @return list of urls
				 */
				QList<Url> albumAddresses(const QString& artist, const QString& album) const;

				/**
				 * @brief get urls for a fuzzy query
				 * @param str query string
				 * @return list of urls
				 */
				QList<Url> searchAddresses(const QString& str) const;

				QList<Url> radioSearchAddresses(const QString& stationName, const QString& radioUrl) const;

				/**
				 * @brief get urls for a fuzzy query
				 * @param str query string
				 * @return list of urls
				 */
				QList<Url> searchAddresses(const QString& str,
				                           const QString& coverFetcherIdentifier) const;

				/**
				 * @brief get a CoverFetcherInterface by a specific url
				 * @param url the url retrieved from artist_addresses(), album_addresses(),
				 * search_addresses() or direct_fetch_url()
				 * @return null, if there's no suitable CoverFetcherInterface registered
				 */
				CoverFetcherPtr coverfetcher(const Url& url) const;

				/**
				 * @brief fetches all available cover fetcher
				 * @return
				 */
				CoverFetcherList coverfetchers() const;

				bool isActive(const CoverFetcherPtr fetcher) const;
				bool isActive(const QString& identifier) const;

				/**
				 * @brief If the LibraryItem has a reference to
				 * a cover download url an appropriate Url object
				 * can be retrieved here. The corresponding
				 * Cover::Fetcher is Cover::Fetcher::DirectFetcher
				 * @param url the direct download url
				 * @return
				 */
				Url directFetcherUrl(const QString& url) const;

				Url websiteFetcherUrl(const QString& url) const;

				static bool isSearchstringWebsite(const QString& searchstring);

			private slots:
				void serversChanged();
		};

	}
}
#endif // COVERFETCHMANAGER_H
