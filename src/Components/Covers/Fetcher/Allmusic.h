/* AllMusicCoverFetcher.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara Indexyer
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

#ifndef ALLMUSICCOVERFETCHER_H
#define ALLMUSICCOVERFETCHER_H

#include "CoverFetcher.h"

namespace Cover::Fetcher
{
	class Allmusic :
		public Cover::Fetcher::Base
	{
		private:
			QString privateIdentifier() const override;

		public:
			bool canFetchCoverDirectly() const override;
			QStringList parseAddresses(const QByteArray& website) const override;

			QString artistAddress(const QString& artist) const override;
			QString albumAddress(const QString& artist, const QString& album) const override;
			QString fulltextSearchAddress(const QString& str) const override;

			int estimatedSize() const override;
	};
}

#endif // ALLMUSICCOVERFETCHER_H
