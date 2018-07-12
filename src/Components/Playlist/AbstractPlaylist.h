/* Playlist.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

#include <QString>

class QStringList;
namespace SP
{
	template<typename T>
	class Set;
}

class MetaDataList;
class MetaData;

namespace Playlist
{
	/**
	 * @brief The Playlist class
	 * @ingroup Playlists
	 */
	class Base :
			public DBInterface,
			public SayonaraClass
	{
		Q_OBJECT
		PIMPL(Base)

		friend class Handler;

		signals:
			void sig_items_changed(int idx);
			void sig_current_track_changed(int idx);

		public:
			explicit Base(int idx, const QString& name=QString());
			virtual ~Base();

			QStringList		toStringList() const;

			IdxList			find_tracks(int id) const;
			IdxList			find_tracks(const QString& filepath) const;
			int				current_track_index() const;
			bool			current_track(MetaData& metadata) const;
			int				index() const;
			void			set_index(int idx);
			void			set_mode(const Playlist::Mode& mode);
			MilliSeconds	running_time() const;
			Playlist::Mode	mode() const;

			virtual
			Playlist::Type	type() const = 0;

			// from PlaylistDBInterface
			bool				is_empty() const override;
			int					count() const override;
			const MetaDataList&	playlist() const override;

			bool				was_changed() const override;
			bool				is_storable() const override;

			virtual int			create_playlist(const MetaDataList& v_md)=0;
			virtual void		replace_track(int idx, const MetaData& metadata);

			virtual void		play()=0;
			virtual void		pause()=0;
			virtual void		stop()=0;
			virtual void		fwd()=0;
			virtual void		bwd()=0;
			virtual void		next()=0;
			virtual bool		wake_up()=0;

		protected:
			void				set_storable(bool b);
			void				set_changed(bool b) override;


		public:
			const MetaData& operator[](int idx) const;

			MetaData& metadata(int idx);
			const MetaData& metadata(int idx) const;

			MetaDataList& metadata();
			const MetaDataList& metadata() const;

			virtual void clear();

			virtual IndexSet move_tracks(const IndexSet& indexes, int tgt);

			virtual IndexSet copy_tracks(const IndexSet& indexes, int tgt);

			virtual void remove_tracks(const IndexSet& indexes);

			virtual void insert_track(const MetaData& metadata, int tgt);
			virtual void insert_tracks(const MetaDataList& lst, int tgt);

			virtual void append_tracks(const MetaDataList& lst);

			virtual bool change_track(int idx);

		public slots:
			virtual void metadata_deleted(const MetaDataList& v_md_deleted)=0;
			virtual void metadata_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new)=0;
			virtual void metadata_changed_single(const MetaData& metadata)=0;
			virtual void duration_changed(MilliSeconds duration)=0;


		private slots:
			void _sl_playlist_mode_changed();
	};
}
#endif // PLAYLIST_H
