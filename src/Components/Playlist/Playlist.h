/* Playlist.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "PlaylistStopBehavior.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Pimpl.h"

#include <QObject>

namespace Playlist
{
	/**
	 * @brief The Playlist class
	 * @ingroup Playlists
	 */
	class Playlist :
			public QObject,
			public DBInterface,
			protected StopBehavior
	{
		Q_OBJECT
		PIMPL(Playlist)

		friend class Handler;

		signals:
			void sig_items_changed(int idx);
			void sig_current_track_changed(int idx);
			void sig_stopped();
			void sig_find_track(TrackID track_id);

		public:
			explicit Playlist(int idx, Type type, const QString& name);
			~Playlist();

			int				create_playlist(const MetaDataList& v_md);
			int				current_track_index() const;
			bool			current_track(MetaData& metadata) const;
			int				index() const;
			void			set_index(int idx);
			Mode			mode() const;
			void			set_mode(const Mode& mode);
			MilliSeconds	running_time() const;
			int				count() const override;

			void			enable_all();

			void			play();
			void			stop();
			void			fwd();
			void			bwd();
			void			next();
			bool			wake_up();

		public:
			const MetaData& track(int idx) const override;
			const MetaDataList& tracks() const override;

			void insert_tracks(const MetaDataList& lst, int tgt);
			void append_tracks(const MetaDataList& lst);
			void remove_tracks(const IndexSet& indexes);
			void replace_track(int idx, const MetaData& metadata);
			void clear();

			IndexSet move_tracks(const IndexSet& indexes, int tgt);
			IndexSet copy_tracks(const IndexSet& indexes, int tgt);

			void find_track(int idx);

			bool change_track(int idx);

			bool was_changed() const override;
			bool is_storable() const override;

		public slots:
			void metadata_deleted();
			void metadata_changed();
			void metadata_changed_single();
			void duration_changed();

		private slots:
			void setting_playlist_mode_changed();

		private:
			int calc_shuffle_track();
			void set_changed(bool b) override;
	};
}
#endif // PLAYLIST_H
