/* MetaDataList.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "MetaDataList.h"

#include "Utils/Algorithm.h"
#include "Utils/Set.h"
#include "Utils/globals.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>

namespace
{
	int ensureBetween(const int index, const int minimum, const int maximum)
	{
		return std::max(std::min(index, maximum), minimum);
	}
}

MetaDataList::MetaDataList() = default;

MetaDataList::MetaDataList(const MetaData& track) :
	MetaDataList()
{
	push_back(track);
}

MetaDataList::MetaDataList(const MetaDataList& other) = default;

MetaDataList::MetaDataList(MetaDataList&& other) noexcept = default;

MetaDataList::~MetaDataList() = default;

MetaDataList& MetaDataList::operator=(const MetaDataList& other)
{
	clear();
	std::copy(other.begin(), other.end(), std::back_inserter(*this));
	return *this;
}

MetaDataList& MetaDataList::operator=(MetaDataList&& other) noexcept
{
	clear();
	std::move(other.begin(), other.end(), std::back_inserter(*this));
	return *this;
}

void MetaDataList::insertTracks(const MetaDataList& tracks, const int targetIndex)
{
	if(targetIndex >= count())
	{
		std::copy(tracks.begin(), tracks.end(), std::back_inserter(*this));
	}

	else
	{
		const auto it = begin() + std::max(targetIndex, 0);
		std::copy(tracks.begin(), tracks.end(), std::inserter(*this, it));
	}
}

void MetaDataList::copyTracks(const IndexSet& indexes, int targetIndex)
{
	if(isEmpty())
	{
		return;
	}

	targetIndex = ensureBetween(targetIndex, 0, count() - 1);

	MetaDataList tracks;
	for(const auto index: indexes)
	{
		if(Util::between(index, *this))
		{
			tracks << at(static_cast<size_t>(index));
		}
	}

	insertTracks(tracks, targetIndex);
}

void MetaDataList::moveTracks(const IndexSet& indexes, int targetIndex) noexcept
{
	if(isEmpty())
	{
		return;
	}

	targetIndex = ensureBetween(targetIndex, 0, count() - 1);

	MetaDataList tracksToMove;
	MetaDataList tracksBeforeTgt;
	MetaDataList tracksAfterTgt;

	auto i = 0;
	for(auto it = begin(); it != end(); it++, i++)
	{
		auto& track = *it;

		if(indexes.contains(i))
		{
			tracksToMove << std::move(track);
		}

		else if(i < targetIndex)
		{
			tracksBeforeTgt << std::move(track);
		}

		else
		{
			tracksAfterTgt << std::move(track);
		}
	}

	auto it = begin();
	std::move(tracksBeforeTgt.begin(), tracksBeforeTgt.end(), it);
	it += tracksBeforeTgt.count();

	std::move(tracksToMove.begin(), tracksToMove.end(), it);
	it += tracksToMove.count();

	std::move(tracksAfterTgt.begin(), tracksAfterTgt.end(), it);
}

void MetaDataList::removeTracks(int first, int last)
{
	if(!isEmpty())
	{
		first = std::max(first, 0);
		last = std::min(count() - 1, last);

		erase(begin() + first, begin() + last + 1);
	}
}

void MetaDataList::removeTracks(std::function<bool(const MetaData&)>&& attr)
{
	if(!isEmpty())
	{
		erase(std::remove_if(begin(), end(), attr), end());
	}
}

void MetaDataList::removeTracks(const IndexSet& indexes)
{
	for(auto it = indexes.rbegin(); it != indexes.rend(); it++)
	{
		if(Util::between(*it, *this))
		{
			erase(begin() + *it);
		}
	}
}

MetaDataList& MetaDataList::operator<<(const MetaDataList& tracks)
{
	std::copy(tracks.begin(), tracks.end(), std::back_inserter(*this));
	return *this;
}

MetaDataList& MetaDataList::operator<<(MetaDataList&& tracks) noexcept
{
	std::move(tracks.begin(), tracks.end(), std::back_inserter(*this));
	return *this;
}

MetaDataList& MetaDataList::operator<<(const MetaData& track)
{
	push_back(track);
	return *this;
}

MetaDataList& MetaDataList::operator<<(MetaData&& track) noexcept
{
	push_back(std::move(track));
	return *this;
}

void MetaDataList::appendUnique(const MetaDataList& other)
{
	auto filepaths = std::set<uint> {};
	std::for_each(begin(), end(), [&filepaths](const auto& track) {
		filepaths.insert(track.filepathHash());
	});

	for(const auto& track: other)
	{
		const auto hash = track.filepathHash();
		if(filepaths.find(hash) == filepaths.end())
		{
			filepaths.insert(hash);
			push_back(track);
		}
	}
}

int MetaDataList::count() const { return static_cast<int>(Parent::size()); }

bool MetaDataList::isEmpty() const { return empty(); }

const MetaData& MetaDataList::operator[](int i) const { return at(i); }

MetaData& MetaDataList::operator[](int i) { return at(i); }

/*** free functions ***/

namespace Util
{
	QList<TrackID> trackIds(const MetaDataList& tracks)
	{
		QList<TrackID> trackIds;
		Util::Algorithm::transform(tracks, trackIds, [](const auto& track) {
			return track.id();
		});

		return trackIds;
	}

	QList<UniqueId> uniqueIds(const MetaDataList& tracks)
	{
		auto result = QList<UniqueId> {};
		Util::Algorithm::transform(tracks, result, [](const auto& track) {
			return track.uniqueId();
		});

		return result;
	}
}
