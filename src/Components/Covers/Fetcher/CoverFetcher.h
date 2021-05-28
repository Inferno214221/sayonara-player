/* CoverFetcher.h */

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

#ifndef ABSTRACTCOVERFETCHER_H
#define ABSTRACTCOVERFETCHER_H

class QString;
class QByteArray;
class QStringList;

namespace Cover::Fetcher
{
	/**
	 * @brief The CoverFetcherInterface interface
	 * @ingroup Covers
	 */
	class Base
	{
		private:
			virtual QString privateIdentifier() const = 0;

		public:
			Base();
			virtual ~Base();

			/**
			 * @brief Can the cover be fetched from the adress without starting a two-stage query?
			 * @return
			 */
			virtual bool canFetchCoverDirectly() const = 0;

			/**
			 * @brief  Get addresses from the downloaded website.
			 * If can_fetch_cover_directly returns true, this method is not called
			 * @param website website data
			 * @return
			 */
			virtual QStringList parseAddresses(const QByteArray& website) const = 0;

			/**
			 * @brief get name of CoverFetcherInterface like e.g. Discogs
			 * @return
			 */
			virtual QString identifier() const final;

			/**
			 * @brief Get the artist search url.
			 * This is called if is_artist_supported returns true.
			 * @param artist artist name
			 * @return
			 */
			virtual QString artistAddress(const QString& artist) const;

			/**
			 * @brief Get the album search url.
			 * This is called if is_album_supported returns true.
			 * @param artist artist name
			 * @param album album name
			 * @return
			 */
			virtual QString albumAddress(const QString& artist, const QString& album) const;

			/**
			 * @brief Get a custom search address
			 * This is called if is_search_supported returns true
			 * @param str search string
			 * @return
			 */
			virtual QString fulltextSearchAddress(const QString& str) const;

			virtual QString radioSearchAddress(const QString& stationName, const QString& stationUrl) const;

			/**
			 * @brief get_estimated_size. Rough image size of the CoverFetchInterface
			 * @return e.g. 300px
			 */
			virtual int estimatedSize() const = 0;

			virtual bool isWebserviceFetcher() const;

		protected:
			QString searchStringFromRadioStation(const QString& stationName, const QString& stationUrl) const;
	};

} // Cover::Fetcher

#endif // ABSTRACTCOVERFETCHER_H
