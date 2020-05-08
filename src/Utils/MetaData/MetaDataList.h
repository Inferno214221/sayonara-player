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

#include <deque>
#include <functional>

/**
 * @brief The MetaDataList class
 * @ingroup MetaDataHelper
 */

class MetaDataList :
		public std::deque<MetaData>
{
	using Parent=std::deque<MetaData>;

public:
	MetaDataList();
	explicit MetaDataList(const MetaData& md);

	MetaDataList(const MetaDataList&);
	MetaDataList(MetaDataList&& other) noexcept;

	MetaDataList& operator=(const MetaDataList& other);
	MetaDataList& operator=(MetaDataList&& other) noexcept;

	~MetaDataList();

	bool contains(const MetaData& md) const;
	MetaDataList& removeTrack(int idx);
	MetaDataList& removeTracks(const IndexSet& rows);
	MetaDataList& removeTracks(int first, int last);
	MetaDataList& removeTracks(std::function<bool (const MetaData&)> attr);

	MetaDataList& moveTracks(const IndexSet& indexes, int tgt_idx) noexcept;
	MetaDataList& copyTracks(const IndexSet& indexes, int tgt_idx);
	MetaDataList& insertTrack(const MetaData& md, int tgt_idx);
	MetaDataList& insertTracks(const MetaDataList& v_md, int tgt_idx);

	IdxList findTracks(Id id) const;
	IdxList findTracks(const QString& filepath) const;

	QStringList toStringList() const;

	MetaDataList& operator <<(const MetaDataList& v_md);
	MetaDataList& operator <<(const MetaData& md);
	MetaDataList& operator <<(MetaDataList&& v_md) noexcept;
	MetaDataList& operator <<(MetaData&& md) noexcept;

	const MetaData& operator[](int i) const;
	MetaData& operator[](int i);

	MetaDataList& append(const MetaDataList& v_md);
	MetaDataList& append(MetaDataList&& v_md) noexcept;
	MetaDataList& append(const MetaData& md);
	MetaDataList& append(MetaData&& md) noexcept;

	QList<UniqueId> unique_ids() const;

	bool contains(TrackID id) const;
	void removeDuplicates();
	MetaData takeAt(int idx);
	bool isEmpty() const;
	MetaDataList& appendUnique(const MetaDataList& other);

	const MetaData& first() const;
	const MetaData& last() const;

	int count() const;

	void sort(Library::SortOrder so);

	void reserve(size_t items);
	size_t capacity() const;
};


#endif // METADATALIST_H
