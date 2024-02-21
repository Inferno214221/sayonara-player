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

#ifndef SAYONARA_COMPONENTS_PLAYLIST
#define SAYONARA_COMPONENTS_PLAYLIST

#include "PlaylistDBInterface.h"
#include "PlaylistModifiers.h"

#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Playlist/PlaylistMode.h"

#include "Utils/Pimpl.h"

#include <QObject>
#include <functional>
#include <optional>

class PlayManager;
class MetaDataList;

namespace Util
{
	class FileSystem;
}

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
			void sigLockChanged();
			void sigItemsChanged(int index);
			void sigTrackChanged(int oldIndex, int newIndex);
			void sigBusyChanged(bool b);
			void sigCurrentScannedFileChanged(const QString& currentFile);

		public:
			Playlist(int playlistIndex, const QString& name, PlayManager* playManager,
			         const std::shared_ptr<Util::FileSystem>& fileSystem);
			~Playlist() override;

			int createPlaylist(const MetaDataList& tracks);

			[[nodiscard]] int currentTrackIndex() const;

			[[nodiscard]] int index() const;
			void setIndex(int idx);

			[[nodiscard]] Mode mode() const;
			void setMode(const Mode& mode);

			void play();
			void stop();
			void fwd();
			void bwd();
			void next();
			bool wakeUp();

			[[nodiscard]] int count() const;

			[[nodiscard]] bool isBusy() const;
			void setBusy(bool b);

			[[nodiscard]] const MetaDataList& tracks() const override;

			bool changeTrack(int index, MilliSeconds positionMs = 0);

			[[nodiscard]] bool wasChanged() const override;
			void resetChangedStatus();

			using Modificator = std::function<MetaDataList(MetaDataList)>;
			void modifyTracks(Modificator&& modificator, Reason reason, Operation operation);

		protected:
			void setChanged(bool b) override;
			void emitLockChanged() override;

		private slots:
			void metadataChanged();
			void metadataDeleted();
			void settingPlaylistModeChanged();
			void currentMetadataChanged();
			void durationChanged();

		private:
			void replaceTrack(int index, const MetaData& track);
			void setCurrentTrack(int index);
	};
}

#endif // SAYONARA_COMPONENTS_PLAYLIST
