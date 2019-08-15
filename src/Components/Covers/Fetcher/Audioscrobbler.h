/* Audioscrobbler.h */

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

#ifndef LFMCOVERFETCHER_H
#define LFMCOVERFETCHER_H

#include "../CoverFetcherInterface.h"

namespace Cover::Fetcher
{
    /**
     * @brief The LFMCoverFetcher class. See CoverFetcherInterface
     * @ingroup Covers
     */
	class Audioscrobbler :
            public Cover::Fetcher::Base
    {
		private:
			QString priv_identifier() const override;

		public:
			bool can_fetch_cover_directly() const override;
			QStringList parse_addresses(const QByteArray& website) const override;

			QString album_address(const QString& artist, const QString& album) const override;

			int estimated_size() const override;
    };
}

#endif // LFMCOVERFETCHER_H
