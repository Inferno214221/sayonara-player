/* MimeDataUtils.h */

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



#ifndef MimeDataUtils_H
#define MimeDataUtils_H

class QMimeData;
class MetaDataList;

class QStringList;
class QString;

namespace Gui
{
	class CustomMimeData;
	/**
	 * @brief MimeData Utils
	 * @ingroup MimeData
	 */
	namespace MimeData
	{
		MetaDataList metadata(const QMimeData* data);

		QStringList playlists(const QMimeData* data);

		void set_cover_url(QMimeData* data, const QString& url);
		QString cover_url(const QMimeData* data);

		CustomMimeData* custom_mimedata(QMimeData* data);
		const CustomMimeData* custom_mimedata(const QMimeData* data);

		bool is_player_drag(const QMimeData* data);
		bool is_inner_drag_drop(const QMimeData* data, int target_playlist_idx);
		bool is_drag_from_playlist(const QMimeData* data);
	}
}

#endif // MimeDataUtils_H
