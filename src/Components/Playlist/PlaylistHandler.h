/* Playlist.h */

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


/*
 * Playlist.h
 *
 *  Created on: Apr 6, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef SAYONARA_PLAYLISTHANDLER_H
#define SAYONARA_PLAYLISTHANDLER_H

#include "PlaylistDBInterface.h"
#include "PlaylistInterface.h"

#include "Utils/Pimpl.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Library/LibraryNamespaces.h"

#include <QObject>

class CustomPlaylist;
class PlayManager;

namespace Playlist
{
	class LocalPathPlaylistCreator;
	class Loader;
	class Handler :
		public QObject,
		public PlaylistCreator,
		public PlaylistAccessor
	{
		Q_OBJECT
		PIMPL(Handler)

		public:
			Handler(PlayManager* playManager, const std::shared_ptr<::Playlist::Loader>& playlistLoader);
			~Handler() override;

			void shutdown();

			[[nodiscard]] int count() const override;

			PlaylistPtr playlist(int playlistIndex) override;
			PlaylistPtr playlistById(int playlistId) override;

			[[nodiscard]] int activeIndex() const override;
			PlaylistPtr activePlaylist() override;

			[[nodiscard]] int currentIndex() const override;
			void setCurrentIndex(int playlistIndex) override;

			[[nodiscard]] QString requestNewPlaylistName(const QString& prefix = QString()) const override;

			int
			createPlaylist(const MetaDataList& tracks, const QString& name = QString(), bool temporary = true,
			               bool isLocked = false) override;

			int createPlaylist(const QStringList& paths, const QString& name = QString(), bool temporary = true,
			                   LocalPathPlaylistCreator* playlistFromPathCreator = nullptr) override;

			int createPlaylist(const CustomPlaylist& playlist) override;
			int createCommandLinePlaylist(const QStringList& pathList,
			                              LocalPathPlaylistCreator* playlistFromPathCreator) override;

			int createEmptyPlaylist(bool override = false) override;

		public slots: // NOLINT(readability-redundant-access-specifiers)
			void closePlaylist(int playlistIndex);

		private:
			int addNewPlaylist(const QString& name, bool editable);
			[[nodiscard]] int exists(const QString& name) const;

		private slots: // NOLINT(readability-redundant-access-specifiers)
			void trackChanged(int oldIndex, int newIndex);
			void previous();
			void next();
			void wakeUp();
			void playstateChanged(PlayState state);
			void wwwTrackFinished(const MetaData& track);
			void playlistRenamed(int id, const QString& oldNamde, const QString& newName);
			void playlistDeleted(int id);

		signals:
			void sigNewPlaylistAdded(int playlistIndex);
			void sigPlaylistNameChanged(int playlistIndex);
			void sigCurrentPlaylistChanged(int playlistIndex);
			void sigActivePlaylistChanged(int playlistIndex);
			void sigTrackDeletionRequested(const MetaDataList& tracks, Library::TrackDeletionMode deletion_mode);
			void sigPlaylistClosed(int playlistIndex);
	};
}

#endif /* SAYONARA_PLAYLISTHANDLER_H */
