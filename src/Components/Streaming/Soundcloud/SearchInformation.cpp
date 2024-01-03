/* SearchInformation.cpp */

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



#include "SearchInformation.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

#include <QString>
#include <QHash>

struct SC::SearchInformation::Private
{
	ArtistId artistId;
	AlbumId albumId;
	TrackID trackId;

	QString searchstring;
};

struct SC::SearchInformationList::Private
{
	QHash<QString, IdSet> artistIdMap;
	QHash<QString, IdSet> albumIdMap;
	QHash<QString, IdSet> trackIdMap;
};

SC::SearchInformation::SearchInformation(int artistId, int albumId, int trackId, const QString& search_string)
{
	m = Pimpl::make<Private>();
	m->artistId = artistId;
	m->albumId = albumId;
	m->trackId = trackId;
	m->searchstring = search_string;
}

SC::SearchInformation::~SearchInformation() = default;

QString SC::SearchInformation::searchstring() const
{
	return m->searchstring;
}

int SC::SearchInformation::artistId() const
{
	return m->artistId;
}

int SC::SearchInformation::albumId() const
{
	return m->albumId;
}

int SC::SearchInformation::trackId() const
{
	return m->trackId;
}

SC::SearchInformationList::SearchInformationList()
{
	m = Pimpl::make<Private>();
}

SC::SearchInformationList::~SearchInformationList() {}

static IntSet ids(const QString& search_string, const QHash<QString, IntSet>& idMap)
{
	IntSet ids;
	QHash<int, int> results;
	int iterations = 0;

	for(int idx = 0; idx < search_string.size() - 3; idx++)
	{
		QString part = search_string.mid(idx, 3);
		const IntSet& part_ids = idMap[part];

		if(part_ids.isEmpty())
		{
			break;
		}

		for(int part_id: part_ids)
		{
			if(results.contains(part_id))
			{
				results[part_id] = results[part_id] + 1;
			}

			else
			{
				if(iterations == 0)
				{
					results[part_id] = 1;
				}
			}
		}

		iterations++;
	}

	for(auto it = results.cbegin(); it != results.cend(); it++)
	{
		int result = it.key();

		if(it.value() == iterations)
		{
			ids.insert(result);
		}
	}

	return ids;
}

IntSet SC::SearchInformationList::artistIds(const QString& search_string) const
{
	return ids(search_string, m->artistIdMap);
}

IntSet SC::SearchInformationList::albumIds(const QString& search_string) const
{
	return ids(search_string, m->albumIdMap);
}

IntSet SC::SearchInformationList::trackIds(const QString& search_string) const
{
	return ids(search_string, m->trackIdMap);
}

SC::SearchInformationList& SC::SearchInformationList::operator<<(const SearchInformation& search_information)
{
	QString search_string = search_information.searchstring();
	for(int idx = 0; idx < search_string.size() - 5; idx++)
	{
		QString part = search_string.mid(idx, 3).toLower();

		m->albumIdMap[part].insert(search_information.albumId());
		m->artistIdMap[part].insert(search_information.artistId());
		m->trackIdMap[part].insert(search_information.trackId());
	}

	return *this;
}

bool SC::SearchInformationList::isEmpty() const
{
	return m->albumIdMap.isEmpty();
}

void SC::SearchInformationList::clear()
{
	m->albumIdMap.clear();
	m->artistIdMap.clear();
	m->trackIdMap.clear();
}

