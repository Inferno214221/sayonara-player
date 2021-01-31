/* Playlist.h */

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


/*
 * Playlist.h
 *
 *  Created on: Apr 6, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef PLAYLISTHANDLER_H_
#define PLAYLISTHANDLER_H_

#include "PlaylistDBInterface.h"
#include "Interfaces/PlaylistCreator.h"

#include "Components/PlayManager/PlayState.h"

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Library/LibraryNamespaces.h"

#include <QObject>

class CustomPlaylist;
class PlayManager;

namespace Playlist
{
	class Loader;
	/**
	 * @brief Global handler for playlists
	 * @ingroup Playlists
	 */
	class Handler :
		public QObject,
		public PlaylistCreator
	{
		Q_OBJECT
		PIMPL(Handler)

		public:
			Handler(PlayManager* playManager, std::shared_ptr<::Playlist::Loader> playlistLoader);
			~Handler();

			/**
			 * @brief Call this before the program stops.
			 * Singletons and Destructors don't work out so well
			 */
			void shutdown();

			/**
			 * @brief Returns number of playlists
			 * @return
			 */
			int count() const;

			/**
			 * @brief get specific playlist at given index
			 * @param playlistIndex playlist index
			 * @return read only pointer object to a playlist, may be nullptr
			 */
			PlaylistPtr playlist(int playlistIndex) override;
			PlaylistPtr playlistById(int playlistId) override;

			int activeIndex() const;
			PlaylistPtr activePlaylist();

			int currentIndex() const;
			void setCurrentIndex(int playlistIndex);

			/**
			 * @brief Request a new name for the playlist (usually New %1 is returned).
			 * If the prefix differs, instead of New, the prefix is chosen.
			 * E.g. "File system 2" for tracks added by the file manager
			 * @param The prefix is a localized "New" by default.
			 * @return playlist name
			 */
			QString requestNewPlaylistName(const QString& prefix = QString()) const override;

			/**
			 * @brief create a new playlist
			 * @param tracks track list
			 * @param name new playlist name. If no name given, current playlist will be overwritten
			 * @param temporary is the playlist temporary or persistent?
			 * @param type deprecated
			 * @return new playlist index
			 */
			int createPlaylist(const MetaDataList& tracks, const QString& name = QString(), bool temporary = true) override;

			/**
			 * @brief create a new playlist (overloaded)
			 * @param pathlist paths, may contain files or directories
			* @param name new playlist name. If no name given, current playlist will be overwritten
			 * @param temporary is the playlist temporary or persistent?
			 * @param type deprecated
			 * @return new playlist index
			 */
			int createPlaylist(const QStringList& pathList, const QString& name = QString(), bool temporary = true) override;
			int createCommandLinePlaylist(const QStringList& pathList) override;

			/**
			 * @brief create a new playlist (overloaded)
			 * @param customPlaylist a CustomPlaylist object fetched from database
			 * @return new playlist index
			 */
			int createPlaylist(const CustomPlaylist& customPlaylist) override;

			/**
			 * @brief create a new empty playlist
			 * @param name new playlist name. If no name given, current playlist will be overwritten
			 * @return new playlist index
			 */
			int createEmptyPlaylist(bool override = false) override;

			void deleteTracks(int playlistIndex, const IndexSet& rows, Library::TrackDeletionMode deletionMode);

			void applyPlaylistActionAfterDoubleClick();

		public slots:
			/**
			 * @brief close playlist
			 * @param playlistIndex playlist index
			 */
			void closePlaylist(int playlistIndex);

		private:
			int addNewPlaylist(const QString& name, bool editable);
			int exists(const QString& name) const;

		private slots:
			void trackChanged();
			void previous();
			void next();
			void wakeUp();
			void playstateChanged(PlayState state);
			void wwwTrackFinished(const MetaData& track);
			void playlistRenamed(int id, const QString& oldNamde, const QString& newName);
			void playlistDeleted(int id);

		signals:
			/**
			 * @brief emitted when new playlist has been added
			 * @param playlistIndex reference to new playlist
			 */
			void sigNewPlaylistAdded(int playlistIndex);

			/**
			 * @brief emitted when playlist name has changed
			 * @param playlistIndex index of playlist
			 */
			void sigPlaylistNameChanged(int playlistIndex);

			/**
			 * @brief emitted when tracks were added/removed or have changed
			 * @param playlistIndex playlist index
			 */
			void sigCurrentPlaylistChanged(int playlistIndex);
			void sigActivePlaylistChanged(int playlistIndex);

			/**
			 * @brief emitted when a track deletion was triggered over the Ui
			 * @param tracks which tracks should be deleted
			 * @param deletion_mode
			 */
			void sigTrackDeletionRequested(const MetaDataList& tracks, Library::TrackDeletionMode deletion_mode);

			void sigPlaylistClosed(int playlistIndex);
			void sigFindTrackRequested(TrackID trackId);
	};

	class HandlerProvider
	{
		PIMPL(HandlerProvider)
		SINGLETON(HandlerProvider)

		public:
			void init(Handler* handler);
			Handler* handler();
	};
}

#endif /* PLAYLISTHANDLER_H_ */
