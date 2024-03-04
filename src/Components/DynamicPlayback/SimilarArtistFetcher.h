/* SimilarArtistFetcher.h */
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
#ifndef SAYONARA_PLAYER_SIMILARARTISTFETCHER_H
#define SAYONARA_PLAYER_SIMILARARTISTFETCHER_H

#include "Utils/Pimpl.h"
#include <QObject>

class QString;
namespace DynamicPlayback
{
	class ArtistMatch;
	class SimilarArtistFetcher :
		public QObject
	{
		Q_OBJECT
		PIMPL(SimilarArtistFetcher)

		signals:
			void sigFinished();

		public:
			explicit SimilarArtistFetcher(const QString& artistName, QObject* parent = nullptr);
			~SimilarArtistFetcher() override;

			[[nodiscard]] virtual const ArtistMatch& similarArtists() const = 0;

		protected:
			virtual void fetchSimilarArtists(const QString& artistName) = 0;

		public slots:
			void start();
	};

	class SimilarArtistFetcherFactory
	{
		public:
			virtual ~SimilarArtistFetcherFactory() = default;
			virtual SimilarArtistFetcher* create(const QString& artist) = 0;
	};

	using SimilarArtistFetcherFactoryPtr = std::shared_ptr<SimilarArtistFetcherFactory>;
}
#endif //SAYONARA_PLAYER_SIMILARARTISTFETCHER_H
