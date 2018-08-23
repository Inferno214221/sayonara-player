	/* MetaDataList.h */

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

#ifndef METADATALIST_H
#define METADATALIST_H

#include "MetaData.h"
#include "MetaDataFwd.h"
#include "Utils/typedefs.h"
#include "Utils/SetFwd.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

#include <vector>

/**
 * @brief The MetaDataList class
 * @ingroup MetaDataHelper
 */

class MetaDataList :
		public std::vector<MetaData>
{
	PIMPL(MetaDataList)

public:
	MetaDataList();
	explicit MetaDataList(const MetaData& md);

	MetaDataList(const MetaDataList&);
	MetaDataList(MetaDataList&& other);

	MetaDataList& operator=(const MetaDataList& other);
	MetaDataList& operator=(MetaDataList&& other);

	~MetaDataList();


	void set_current_track(int idx);
	int current_track() const;

	bool contains(const MetaData& md) const;
	MetaDataList& remove_track(int idx);
	MetaDataList& remove_tracks(const IndexSet& rows);
	MetaDataList& remove_tracks(int first, int last);

	MetaDataList& move_tracks(const IndexSet& indexes, int tgt_idx);
	MetaDataList& copy_tracks(const IndexSet& indexes, int tgt_idx);
	MetaDataList& insert_track(const MetaData& md, int tgt_idx);
	MetaDataList& insert_tracks(const MetaDataList& v_md, int tgt_idx);

	IdxList findTracks(Id id) const;
	IdxList findTracks(const QString& filepath) const;

	QStringList toStringList() const;

	MetaDataList& operator <<(const MetaDataList& v_md);
	MetaDataList& operator <<(const MetaData& md);

	MetaDataList& append(const MetaDataList& v_md);
	MetaDataList& append(const MetaData& md);

	bool contains(TrackID id) const;
	void remove_duplicates();
	MetaData take_at(int idx);
	bool isEmpty() const;
	MetaDataList& append_unique(const MetaDataList& other);

	const MetaData& first() const;
	const MetaData& last() const;

	int count() const;

	void sort(Library::SortOrder so);
};


#endif // METADATALIST_H
