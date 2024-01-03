/* Filter.cpp */

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

#include "Filter.h"
#include "Utils/Language/Language.h"

namespace
{
	bool isFiltertextEqual(const QString& filtertext1, const QString& filtertext2, const int minimumSearchStringLength)
	{
		if((filtertext1.size() < minimumSearchStringLength) &&
		   (filtertext2.size() < minimumSearchStringLength))
		{
			return true;
		}

		if(filtertext1 == filtertext2)
		{
			return true;
		}

		return false;
	}

	QString wrapInPercent(QString str)
	{
		if(!str.startsWith('%'))
		{
			str.prepend('%');
		}

		if(!str.endsWith('%'))
		{
			str.append('%');
		}

		return str;
	}

	bool isValid(const ::Library::Filter& filter, const int minimumSearchStringLength)
	{
		if(filter.mode() == ::Library::Filter::Mode::Invalid)
		{
			return false;
		}

		if(filter.mode() == ::Library::Filter::Mode::InvalidGenre)
		{
			return true;
		}

		const auto filters = filter.filtertext(false);
		return (filters.join("").size() >= minimumSearchStringLength);
	}
}

namespace Library
{
	struct Filter::Private
	{
		QString filtertext;
		Filter::Mode mode {Filter::Mode::Fulltext};
	};

	Filter::Filter() :
		m {Pimpl::make<Private>()}
	{
		clear();
	}

	Filter::Filter(const Filter& other) :
		Filter::Filter()
	{
		*m = *(other.m);
	}

	Filter& Filter::operator=(const Filter& other)
	{
		*m = *(other.m);
		return *this;
	}

	Filter::~Filter() = default;

	int Filter::count() const
	{
		return m->filtertext.isEmpty()
		       ? 0
		       : m->filtertext.split(",").size();
	}

	QStringList Filter::filtertext(const bool withPercent) const
	{
		QStringList result;

		auto filterTexts = m->filtertext.split(",");
		for(auto& filterText: filterTexts)
		{
			if(withPercent)
			{
				filterText = wrapInPercent(std::move(filterText));
			}

			if(!filterText.isEmpty())
			{
				result << filterText;
			}
		}

		return result;
	}

	QStringList Filter::searchModeFiltertext(const bool withPercent, const SearchModeMask searchModeMask) const
	{
		QStringList result;
		const auto filterTexts = m->filtertext.split(",");

		for(const auto& filterText: filterTexts)
		{
			auto convertedFiltertext = convertSearchstring(filterText, searchModeMask);
			if(withPercent)
			{
				convertedFiltertext = wrapInPercent(std::move(convertedFiltertext));
			}

			if(!convertedFiltertext.isEmpty())
			{
				result << convertedFiltertext;
			}
		}

		return result;
	}

	void Filter::setFiltertext(const QString& str)
	{
		m->filtertext = str;
	}

	Filter::Mode Filter::mode() const
	{
		return m->mode;
	}

	void Filter::setMode(const Filter::Mode mode)
	{
		m->mode = mode;
	}

	bool Filter::cleared() const
	{
		return (m->filtertext.isEmpty() && (m->mode != Filter::Mode::InvalidGenre));
	}

	bool Filter::isEqual(const Filter& other, const int minimumSearchStringLength) const
	{
		const auto bothInvalid =
			!isValid(*this, minimumSearchStringLength) && !isValid(other, minimumSearchStringLength);

		if(bothInvalid)
		{
			return true;
		}

		return (m->mode == other.mode()) &&
		       isFiltertextEqual(m->filtertext, other.m->filtertext, minimumSearchStringLength);
	}

	QString Filter::filterModeName(const Mode mode)
	{
		switch(mode)
		{
			case Filter::Mode::Filename:
				return Lang::get(Lang::Filename);

			case Filter::Mode::Fulltext:
				return Lang::get(Lang::Artists) + ", " +
				       Lang::get(Lang::Albums) + ", " +
				       Lang::get(Lang::Tracks);

			case Filter::Mode::Genre:
			case Filter::Mode::InvalidGenre:
				return Lang::get(Lang::Genre);

			default:
				return {};
		}
	}

	void Filter::clear()
	{
		m->filtertext = QString();
		m->mode = Mode::Fulltext;
	}
}