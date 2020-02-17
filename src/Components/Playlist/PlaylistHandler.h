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

#include "Components/PlayManager/PlayState.h"

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Library/LibraryNamespaces.h"

#include <QObject>

class CustomPlaylist;

namespace Playlist
{
	/**
	 * @brief Global handler for playlists
	 * @ingroup Playlists
	 */
	class Handler :
			public QObject
	{
		Q_OBJECT
		PIMPL(Handler)
		SINGLETON_QOBJECT(Handler)

		public:

			/**
			 * @brief The PlaylistIndex enum
			 */
			enum class PlaylistIndex : uint8_t
			{
				Current=0,
				Active
			};

		signals:
			/**
			 * @brief emitted when new playlist has been created
			 * @param pl Playlist, usually current one
			 */
			void sigPlaylistCreated(PlaylistPtr pl);

			/**
			 * @brief emitted when current track index has changed
			 * @param trackIndex index in playlist
			 * @param playlist_idx index of playlist
			 */
			void sigCurrentTrackChanged(int trackIndex, int playlist_idx);

			/**
			 * @brief emitted when new playlist has been added
			 * @param pl reference to new playlist
			 */
			void sigNewPlaylistAdded(PlaylistPtr pl);

			/**
			 * @brief emitted when playlist name has changed
			 * @param idx index of playlist
			 */
			void sigPlaylistNameChanged(int idx);

			/**
			 * @brief emitted when tracks were added/removed or have changed
			 * @param idx playlist index
			 */
			void sigCurrentPlaylistChanged(int idx);
			void sigActivePlaylistChanged(int idx);

			/**
			 * @brief emitted when a track deletion was triggered over the Ui
			 * @param tracks which tracks should be deleted
			 * @param deletion_mode
			 */
			void sigTrackDeletionRequested(const MetaDataList& tracks, Library::TrackDeletionMode deletion_mode);

			void sigFindTrackRequested(TrackID trackId);


		public:

			/**
			 * @brief Call this before the program stops.
			 * Singletons and Destructors don't work out so well
			 */
			void shutdown();

			/**
			 * @brief clears the current visible playlist
			 * @param playlistIndex playlist index
			 */
			void clearPlaylist(int playlistIndex);

			/**
			 * @brief insert tracks to active playlist after current playback position
			 * @param tracks list of tracks
			 */
			void playNext(const MetaDataList& tracks);
			void playNext(const QStringList& paths);

			/**
			 * @brief insert tracks into a playlist at a given index
			 * @param tracks track list
			 * @param idx track index within playlist
			 * @param playlistIndex playlist index
			 */
			void insertTracks(const MetaDataList& tracks, int idx, int playlistIndex);
			void insertTracks(const QStringList& paths, int idx, int playlistIndex);


			/**
			 * @brief append tracks at a given playlist index
			 * @param tracks track list
			 * @param playlistIndex playlist index
			 */
			void appendTracks(const MetaDataList& tracks, int playlistIndex);
			void appendTracks(const QStringList& paths, int playlistIndex);

			/**
			 * @brief move rows within playlist
			 * @param idx_list list of row indices to be moved
			 * @param tgt_idx target index where rows should be moved
			 * @param playlistIndex playlist index
			 */
			void moveRows(const IndexSet& indexes, int tgt_idx, int playlistIndex);


			/**
			 * @brief remove rows from playlist
			 * @param indexes list of row indices to be removed
			 * @param playlistIndex playlist index
			 */
			void removeRows(const IndexSet& indexes, int playlistIndex);


			/**
			 * @brief change the track in a given playlist
			 * @param idx track index
			 * @param playlistIndex playlist index
			 */
			void changeTrack(int trackIndex, int playlistIndex);


			/**
			 * @brief get active playlist index
			 * @return
			 */
			int	activeIndex() const;
			PlaylistConstPtr activePlaylist() const;


			int current_index() const;
			void set_current_index(int playlistIndex);

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
			PlaylistConstPtr playlist(int playlistIndex) const;


			/**
			 * @brief delete the given playlist from database
			 * @param playlistIndex playlist index
			 */
			void deletePlaylist(int playlistIndex);

			/**
			 * @brief close playlist
			 * @param playlistIndex playlist index
			 */
			int closePlaylist(int playlistIndex);


			/**
			 * @brief reload playlist from db
			 * @param playlistIndex playlist index
			 */
			void resetPlaylist(int playlistIndex);

			/**
			 * @brief Request a new name for the playlist (usually New %1 is returned).
			 * If the prefix differs, instead of New, the prefix is chosen.
			 * E.g. "File system 2" for tracks added by the file manager
			 * @param The prefix is a localized "New" by default.
			 * @return playlist name
			 */
			QString requestNewPlaylistName(const QString& prefix=QString()) const;


			/**
			 * @brief save playlist to database, overwrite old one
			 * @param playlistIndex playlist index
			 * @return SaveAnswer
			 */
			Util::SaveAsAnswer savePlaylist(int playlistIndex);


			/**
			 * @brief Save playlist under new name
			 * @param playlistIndex playlist index
			 * @param name new playlist name
			 * @param force_override override if name exists
			 * @return AlreadyThere if name exists and force_override is false
			 */
			Util::SaveAsAnswer savePlaylistAs(int playlistIndex, const QString& name, bool forceOverride);


			/**
			 * @brief rename playlist
			 * @param playlistIndex playlist index
			 * @param name new playlist name
			 * @return
			 */
			Util::SaveAsAnswer renamePlaylist(int playlistIndex, const QString& name);


			/**
			 * @brief save a playlist to file
			 * @param filename, if filename does not end with m3u, extension is appended automatically
			 * @param relative relative paths in m3u file
			 */
			void savePlaylistToFile(int playlistIndex, const QString& filename, bool relative);

			/**
			 * @brief create a new playlist
			 * @param tracks track list
			 * @param name new playlist name. If no name given, current playlist will be overwritten
			 * @param temporary is the playlist temporary or persistent?
			 * @param type deprecated
			 * @return new playlist index
			 */
			int createPlaylist(const MetaDataList& tracks, const QString& name=QString(), bool temporary=true, PlaylistType type=PlaylistType::Std);

			/**
			 * @brief create a new playlist (overloaded)
			 * @param pathlist paths, may contain files or directories
			* @param name new playlist name. If no name given, current playlist will be overwritten
			 * @param temporary is the playlist temporary or persistent?
			 * @param type deprecated
			 * @return new playlist index
			 */
			int createPlaylist(const QStringList& path_list, const QString& name=QString(), bool temporary=true, PlaylistType type=PlaylistType::Std);

			/**
			 * @brief create a new playlist (overloaded)
			 * @param dir directory path
			 * @param name new playlist name. If no name given, current playlist will be overwritten
			 * @param temporary is the playlist temporary or persistent?
			 * @param type deprecated
			 * @return new playlist index
			 */

			int createPlaylist(const QString& dir, const QString& name=QString(), bool temporary=true, PlaylistType type=PlaylistType::Std);


			/**
			 * @brief create a new playlist (overloaded)
			 * @param pl a CustomPlaylist object fetched from database
			 * @return new playlist index
			 */
			int createPlaylist(const CustomPlaylist& pl);


			/**
			 * @brief create a new empty playlist
			 * @param name new playlist name. If no name given, current playlist will be overwritten
			 * @return new playlist index
			 */
			int createEmptyPlaylist(bool overrideCurrent=false);
			int createEmptyPlaylist(const QString& name);


			void deleteTracks(int playlistIndex, const IndexSet& rows, Library::TrackDeletionMode deletion_mode);

		public slots:
			/**
			 * @brief load playlists of last session from database
			 * @return number of playlists fetched
			 */
			int	loadOldPlaylists();


		private slots:

			/**
			 * @brief play active playlist
			 */
			void played();

			/**
			 * @brief stop active playlist
			 */
			void stopped();

			/**
			 * @brief change track to previous track
			 */
			void previous();

			/**
			 * @brief change track to next track
			 */
			void next();

			void wakeUp();


			/**
			 * @brief PlayManager's playstate has changed
			 */
			void playstateChanged(PlayState state);

			void wwwTrackFinished(const MetaData& md);

			void currentTrackChanged(int index);
			void playlistStopped();

			void playlistRenamed(int id, const QString& oldName, const QString& newName);
			void playlistDeleted(int id);

			/**
			 * @brief Return of an async scanning operation when
			 * creating new playlists from paths
			 */
			void filescannerProgressChanged(const QString& current_file);
			void filesScanned();

		private:
			/**
			 * @brief adds a new playlist, creates it, if name is not in the list of playlists. If name already exists,
			 * @param name
			 * @param editable
			 * @param type
			 * @return index of new playlist
			 */
			int	addNewPlaylist(const QString& name, bool editable, PlaylistType type=PlaylistType::Std);

			/**
			 * @brief Create new playlist and return it
			 * @param type
			 * @param idx
			 * @param name
			 * @return
			 */
			PlaylistPtr newPlaylist(PlaylistType type, QString name);


			/**
			 * @brief Checks if playlist exists
			 * @param name playlist name, if empty, current playlist index is returned
			 * @return playlist index, -1 if playlist does not exist, current playlist if name is empty
			 */
			int exists(const QString& name) const;


			/**
			 * @brief get active playlist. If no playlists are available, create one.
			 * If there's no active index, the current index is used
			 * @return
			 */
			PlaylistPtr activePlaylist();

			/**
			 * @brief get playlist at a given index, return fallback if index is invalid
			 * @param playlistIndex playlist index
			 * @param fallback playlist returned when index is invalid
			 * @return
			 */
			PlaylistPtr playlist(int playlistIndex, PlaylistPtr fallback) const;


			/**
			 * @brief tells PlayManager that the current track has been changed,
			 * sets the current playlist index in settings, may insert the playlist into database nevermind if temporary
			 * @param pl paylist of interest. if nullptr, active playlist is taken
			 */
			void emitCurrentTrackChanged();

			/**
			 * @brief Set active playlist index, if playlist_index is invalid,
			 * @param playlistIndex playlist index
			 */
			void setActiveIndex(int playlist_index);

			/**
			 * @brief extracts metadata asynchronously
			 * @param index the playlist index
			 * @param paths list of paths
			 * @param target_row_idx where the found metadata should be inserted.
			 * If -1, the playlist is cleared before,
			 * If greater than number of tracks, the found metadata is appended
			 */
			void createFilescanner(int playlist_index, const QStringList& paths, int target_row_idx);
	};
}

#endif /* PLAYLISTHANDLER_H_ */
