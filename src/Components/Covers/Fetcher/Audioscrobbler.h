/* Audioscrobbler.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "CoverFetcher.h"

namespace Cover::Fetcher
{
	class Audioscrobbler :
		public Cover::Fetcher::Base
	{
		private:
			[[nodiscard]] QString privateIdentifier() const override;

		public:
			[[nodiscard]] bool canFetchCoverDirectly() const override;
			[[nodiscard]] QStringList parseAddresses(const QByteArray& website) const override;
			[[nodiscard]] QString albumAddress(const QString& artist, const QString& album) const override;
			[[nodiscard]] QString artistAddress(const QString& artist) const override;
			[[nodiscard]] int estimatedSize() const override;
	};
}

#endif // LFMCOVERFETCHER_H
