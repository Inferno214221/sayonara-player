/* DynamicPlaybackHandler.h */
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
#ifndef SAYONARA_PLAYER_DYNAMICPLAYBACKHANDLER_H
#define SAYONARA_PLAYER_DYNAMICPLAYBACKHANDLER_H

#include "Utils/Pimpl.h"

#include <QObject>

class MetaData;
class PlayManager;
namespace Playlist
{
	class Accessor;
}

namespace Util
{
	class FileSystem;
}

namespace DynamicPlayback
{
	class SimilarArtistFetcherFactory;
	class ArtistMatch;
	class Handler :
		public QObject
	{
		Q_OBJECT
		PIMPL(Handler)

		public:
			Handler(PlayManager* playManager, Playlist::Accessor* playlistAccessor,
			        const std::shared_ptr<SimilarArtistFetcherFactory>& similarArtistFetcherFactory,
			        const std::shared_ptr<Util::FileSystem>& fileSystem, QObject* parent = nullptr);
			~Handler() override;

		private slots:
			void currentTrackChanged(const MetaData& track);
			void similarArtistsAvailable();
			void timeout();

		private: // NOLINT(*-redundant-access-specifiers)
			void processArtistMatch(const ArtistMatch& artistMatch);
	};
}
#endif //SAYONARA_PLAYER_DYNAMICPLAYBACKHANDLER_H
