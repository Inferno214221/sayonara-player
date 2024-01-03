/* LfmSimilarArtistFetcher.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_LFMSIMILARARTISTFETCHER_H
#define SAYONARA_PLAYER_LFMSIMILARARTISTFETCHER_H

#include "SimilarArtistFetcher.h"
#include "Utils/Pimpl.h"

namespace DynamicPlayback
{
	class LfmSimilarArtistFetcher :
		public SimilarArtistFetcher
	{
		Q_OBJECT
		PIMPL(LfmSimilarArtistFetcher)

		public:
			LfmSimilarArtistFetcher(const QString& artist,
			                        QObject* parent = nullptr);
			~LfmSimilarArtistFetcher() override;
			const ArtistMatch& similarArtists() const override;

		protected:
			void fetchSimilarArtists(const QString& artistName) override;

		private slots:
			void responseReceived(const QByteArray& data);
			void errorReceived(const QString& answer);
	};
}
#endif //SAYONARA_PLAYER_LFMSIMILARARTISTFETCHER_H
