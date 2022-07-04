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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "PlaylistDBInterface.h"

#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Playlist/PlaylistMode.h"

#include "Utils/Pimpl.h"

#include <QObject>

class PlayManager;

namespace Playlist
{
	class Playlist :
		public QObject,
		public DBInterface
	{
		Q_OBJECT
		PIMPL(Playlist)

			friend class Handler;

		signals:
			void sigFindTrackRequested(const MetaData& track);
			void sigDeleteFilesRequested(const MetaDataList& tracks);

			void sigItemsChanged(int index);
			void sigTrackChanged(int oldIndex, int newIndex);
			void sigBusyChanged(bool b);
			void sigCurrentScannedFileChanged(const QString& currentFile);

		public:
			explicit Playlist(int playlistIndex, const QString& name, PlayManager* playManager);
			virtual ~Playlist() override;

			int createPlaylist(const MetaDataList& tracks);

			virtual int currentTrackIndex() const;
			bool currentTrack(MetaData& track) const;
			int currentTrackWithoutDisabled() const;

			int index() const;
			void setIndex(int idx);

			Mode mode() const;
			void setMode(const Mode& mode);

			MilliSeconds runningTime() const;
			int count() const;

			void enableAll();

			void play();
			void stop();
			void fwd();
			void bwd();
			void next();
			bool wakeUp();

			void setBusy(bool b);
			bool isBusy() const;

			void reverse();
			void randomize();

			const MetaData& track(int index) const;
			const MetaDataList& tracks() const override;

			void insertTracks(const MetaDataList& tracks, int targetRow);
			void appendTracks(const MetaDataList& tracks);
			void removeTracks(const IndexSet& indexes);
			void replaceTrack(int idx, const MetaData& track);
			void clear();

			IndexSet moveTracks(const IndexSet& indexes, int targetRow);
			IndexSet copyTracks(const IndexSet& indexes, int targetRow);

			void findTrack(int index);
			bool changeTrack(int index, MilliSeconds positionMs = 0);
			bool wasChanged() const override;

			void reloadFromDatabase();
			void deleteTracks(const IndexSet& indexes);

		public slots:
			void metadataDeleted();
			void metadataChanged();
			void currentMetadataChanged();
			void durationChanged();

		private slots:
			void settingPlaylistModeChanged();

		private:
			void setCurrentTrack(int index);
			void setChanged(bool b) override;
	};
}
#endif // PLAYLIST_H
