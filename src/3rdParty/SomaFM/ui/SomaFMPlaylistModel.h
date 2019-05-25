/* SomaFMPlaylistModel.h */

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


/* SomaFMPlaylistModel.h */

#ifndef SOMAFMPLAYLISTMODEL_H
#define SOMAFMPLAYLISTMODEL_H

#include <QStringListModel>

#include "Utils/Pimpl.h"

class QMimeData;

namespace SomaFM
{
	class Station;
	class PlaylistModel :
			public QStringListModel
	{
		PIMPL(PlaylistModel)

	public:
		explicit PlaylistModel(QObject* parent=nullptr);
		~PlaylistModel();

		QMimeData* mimeData(const QModelIndexList& indexes) const override;

		void set_station(const SomaFM::Station& station);
	};
}

#endif // SOMAFMPLAYLISTMODEL_H
