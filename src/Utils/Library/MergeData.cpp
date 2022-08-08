/* MergeData.cpp */

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

#include "MergeData.h"
#include "Utils/Set.h"

using Library::MergeData;

struct MergeData::Private
{
	Util::Set<Id> sourceIds;
	Id targetId;
	LibraryId libraryId;

	Private(const Util::Set<Id>& sourceIds, const Id targetId, const LibraryId libraryId) :
		sourceIds(sourceIds),
		targetId(targetId),
		libraryId(libraryId) {}
};

MergeData::MergeData(const Util::Set<Id>& sourceIds, const Id target_id, const LibraryId libraryId) :
	m {Pimpl::make<Private>(sourceIds, target_id, libraryId)} {}

MergeData::MergeData(const MergeData& other) :
	m {Pimpl::make<Private>(other.sourceIds(), other.targetId(), other.libraryId())} {}

MergeData::~MergeData() = default;

MergeData& MergeData::operator=(const MergeData& other)
{
	*m = *(other.m);
	return *this;
}

bool MergeData::isValid() const
{
	return ((targetId() >= 0) && (sourceIds().count() >= 2) && !(sourceIds().contains(-1)));
}

Util::Set<Id> MergeData::sourceIds() const { return m->sourceIds; }

Id MergeData::targetId() const { return m->targetId; }

LibraryId MergeData::libraryId() const { return m->libraryId; }
