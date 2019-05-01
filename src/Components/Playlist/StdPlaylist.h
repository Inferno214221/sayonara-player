/* StdPlaylist.h */

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

#ifndef STDPLAYLIST_H
#define STDPLAYLIST_H

#include "AbstractPlaylist.h"

namespace Playlist
{
	/**
	 * @brief The StdPlaylist class
	 * @ingroup Playlists
	 */
	class Standard :
			public Base
	{
		Q_OBJECT
		PIMPL(Standard)

	private:
		int calc_shuffle_track();

	public:
		explicit Standard(int idx, const QString& name);
		virtual ~Standard();

		void play() override;
		void pause() override;
		void stop() override;
		void fwd() override;
		void bwd() override;
		void next() override;
		bool change_track(int idx) override;
		bool wake_up() override;

		int create_playlist(const MetaDataList& v_md) override;

		void metadata_deleted(const MetaDataList &v_md_deleted) override;
		void metadata_changed(const MetaDataList& old_md, const MetaDataList& new_md) override;
		void metadata_changed_single(const MetaData& metadata) override;
		void duration_changed(MilliSeconds duration) override;

		Playlist::Type type() const override;
		void set_changed(bool b) override;
	};
}

#endif // STDPLAYLISTR_H
