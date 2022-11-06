/* MetaDataList.h */

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

#ifndef METADATALIST_H
#define METADATALIST_H

#include "MetaData.h"
#include "Utils/Library/Sortorder.h"

#include <QList>

#include <deque>
#include <functional>

class MetaDataList :
	public std::deque<MetaData>
{
		using Parent = std::deque<MetaData>;

	public:
		MetaDataList();
		explicit MetaDataList(const MetaData& md);

		MetaDataList(const MetaDataList&);
		MetaDataList(MetaDataList&& other) noexcept;

		MetaDataList& operator=(const MetaDataList& other);
		MetaDataList& operator=(MetaDataList&& other) noexcept;

		~MetaDataList();

		void removeTracks(const IndexSet& rows);
		void removeTracks(int first, int last);
		void removeTracks(std::function<bool(const MetaData&)>&& attr);

		void moveTracks(const IndexSet& indexes, int targetIndex) noexcept;
		void copyTracks(const IndexSet& indexes, int targetIndex);
		void insertTracks(const MetaDataList& tracks, int targetIndex);
		void appendUnique(const MetaDataList& newTrack);

		MetaDataList& operator<<(const MetaDataList& tracks);
		MetaDataList& operator<<(const MetaData& track);
		MetaDataList& operator<<(MetaDataList&& tracks) noexcept;
		MetaDataList& operator<<(MetaData&& track) noexcept;

		const MetaData& operator[](int i) const;
		MetaData& operator[](int i);

		[[nodiscard]] int count() const;
		[[nodiscard]] bool isEmpty() const;
};

namespace Util
{
	QList<TrackID> trackIds(const MetaDataList& tracks);
	QList<UniqueId> uniqueIds(const MetaDataList& tracks);
}

#endif // METADATALIST_H
