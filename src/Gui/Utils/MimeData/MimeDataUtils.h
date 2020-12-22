/* MimeDataUtils.h */

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

#ifndef MimeDataUtils_H
#define MimeDataUtils_H

class QMimeData;
class MetaDataList;

class QStringList;
class QString;

namespace Gui
{
	class CustomMimeData;
	class AsyncDropHandler;

	namespace MimeData
	{
		/**
		 * @brief metadata
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		MetaDataList metadata(const QMimeData* data);
		bool hasMetadata(const QMimeData* data);

		/**
		 * @brief playlists
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		QStringList playlists(const QMimeData* data);

		/**
		 * @brief set_cover_url
		 * @ingroup MimeData
		 * @param data
		 * @param url
		 */
		void setCoverUrl(QMimeData* data, const QString& url);

		/**
		 * @brief cover_url
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		QString coverUrl(const QMimeData* data);

		/**
		 * @brief custom_mimedata
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		CustomMimeData* customMimedata(QMimeData* data);

		/**
		 * @brief custom_mimedata
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		const CustomMimeData* customMimedata(const QMimeData* data);

		/**
		 * @brief is_player_drag
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		bool isPlayerDrag(const QMimeData* data);

		/**
		 * @brief is_inner_drag_drop
		 * @ingroup MimeData
		 * @param data
		 * @param target_playlist_idx
		 * @return
		 */
		bool isInnerDragDrop(const QMimeData* data, int targetPlaylistIndex);

		/**
		 * @brief is_drag_from_playlist
		 * @ingroup MimeData
		 * @param data
		 * @return
		 */
		bool isDragFromPlaylist(const QMimeData* data);

		Gui::AsyncDropHandler* asyncDropHandler(const QMimeData* data);
	}
}

#endif // MimeDataUtils_H
