/* AbstractSearchModel.cpp */

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

#include "SearchableModel.h"
#include "Utils/Settings/Settings.h"

#include <QString>

namespace
{
	std::pair<QString, QString> splitSearchstring(const QString& searchstring, const Library::SearchModeMask searchMode)
	{
		const auto splitted = searchstring.split(' ');
		const auto prefix = splitted.size() == 2
		                    ? splitted[0]
		                    : QString {};

		const auto pureSearchstring = splitted.size() == 2
		                              ? splitted[1]
		                              : searchstring;

		return {prefix, Library::convertSearchstring(pureSearchstring, searchMode)};
	}
}

struct SearchModel::Private
{
	QList<int> matches;
	int currentIndex {-1};
};

SearchModel::SearchModel() :
	m {Pimpl::make<Private>()} {}

SearchModel::~SearchModel() noexcept = default;

int SearchModel::initSearch(const QString& searchstring, int startIndex)
{
	m->matches.clear();
	m->currentIndex = -1;

	const auto count = itemCount();
	if(count == 0)
	{
		return 0;
	}

	const auto searchMode = static_cast<Library::SearchModeMask>(GetSetting(Set::Lib_SearchMode));
	const auto [prefix, pureSearchstring] = splitSearchstring(searchstring, searchMode);

	startIndex = std::max(startIndex, 0);
	for(auto i = 0; i < count; i++)
	{
		const auto row = (startIndex + i) % count;
		const auto data = Library::convertSearchstring(searchableString(row, prefix), searchMode);

		if(data.contains(searchstring))
		{
			m->matches << row;
		}
	}

	return m->matches.count();
}

int SearchModel::searchNext()
{
	if(m->matches.isEmpty())
	{
		return -1;
	}

	m->currentIndex = (m->currentIndex + 1) % m->matches.count();
	return m->matches[m->currentIndex];
}

int SearchModel::searchPrevious()
{
	if(m->matches.isEmpty())
	{
		return -1;
	}

	const auto index = m->currentIndex - 1;
	m->currentIndex = (index < 0) ? m->matches.count() - 1 : index;

	return m->matches[m->currentIndex];
}

SearchableTableModel::SearchableTableModel(QObject* parent) :
	QAbstractTableModel(parent) {}

SearchableTableModel::~SearchableTableModel() = default;
