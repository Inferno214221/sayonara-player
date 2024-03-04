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

class WebClientFactory;
namespace DynamicPlayback
{
	class LfmSimilarArtistFetcher :
		public SimilarArtistFetcher
	{
		Q_OBJECT
		PIMPL(LfmSimilarArtistFetcher)

		public:
			LfmSimilarArtistFetcher(const QString& artist, const std::shared_ptr<WebClientFactory>& webClientFactory,
			                        QObject* parent = nullptr);
			~LfmSimilarArtistFetcher() override;
			[[nodiscard]] const ArtistMatch& similarArtists() const override;

		protected:
			void fetchSimilarArtists(const QString& artistName) override;

		private slots:
			void webClientFinished();
	};

	class LfmSimilarArtistFetcherFactory :
		public SimilarArtistFetcherFactory
	{
		public:
			~LfmSimilarArtistFetcherFactory() override;
			SimilarArtistFetcher* create(const QString& artist) override;

	};
}
#endif //SAYONARA_PLAYER_LFMSIMILARARTISTFETCHER_H
