/* Filter.cpp */

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

#include "Filter.h"
#include "Utils/Language/Language.h"

using Library::Filter;

struct Filter::Private
{
	QString					filtertext;
	Filter::Mode			mode;
	Library::SearchModeMask search_mode;
	bool					invalid_genre;

	Private() :
		invalid_genre(false)
	{}
};

Filter::Filter()
{
	m = Pimpl::make<Filter::Private>();
	clear();
}

Filter::Filter(const Filter& other)
{
	m = Pimpl::make<Filter::Private>();
	*m = *(other.m);
}

Filter& Filter::operator=(const Filter& other)
{
	*m = *(other.m);

	return *this;
}

Filter::~Filter() = default;

bool Filter::operator ==(const Filter& other)
{
	if(!this->isUseable() && !other.isUseable()) {
		return true;
	}

	bool same_filtertext = false;

	if(m->filtertext.size() < 3 && other.m->filtertext.size() < 3)
	{
		same_filtertext = true;
	}

	else if(m->filtertext.compare(other.m->filtertext, Qt::CaseInsensitive) == 0)
	{
		same_filtertext = true;
	}

	return
	(
		same_filtertext &&
		(m->mode == other.mode()) &&
		(m->invalid_genre == other.isInvalidGenre())
	);
}


QStringList Filter::filtertext(bool withPercent) const
{
	QStringList ret;
	const QStringList filterTexts = m->filtertext.split(",");

	for(QString filterText : filterTexts)
	{
		if(withPercent)
		{
			if(!filterText.startsWith('%')){
				filterText.prepend('%');
			}

			if(!filterText.endsWith('%')){
				filterText.append('%');
			}
		}

		if(!filterText.isEmpty())
		{
			ret << filterText;
		}
	}

	return ret;
}

QStringList Filter::searchModeFiltertext(bool withPercent) const
{
	QStringList ret;
	const QStringList filterTexts = m->filtertext.split(",");

	for(const QString& filterText : filterTexts)
	{
		QString convertedFiltertext = ::Library::Utils::convertSearchstring(filterText, m->search_mode);
		if(withPercent)
		{
			if(!convertedFiltertext.startsWith('%')){
				convertedFiltertext.prepend('%');
			}

			if(!convertedFiltertext.endsWith('%')){
				convertedFiltertext.append('%');
			}
		}

		if(!convertedFiltertext.isEmpty())
		{
			ret << convertedFiltertext;
		}
	}

	return ret;
}

void Filter::setFiltertext(const QString& str, ::Library::SearchModeMask search_mode)
{
	m->filtertext = str;
	m->search_mode = search_mode;
}

Filter::Mode Filter::mode() const
{
	return m->mode;
}

void Filter::setMode(Filter::Mode mode)
{
	m->mode = mode;
}

bool Filter::cleared() const
{
	return (m->filtertext.isEmpty() && !m->invalid_genre);
}

void Filter::setInvalidGenre(bool b)
{
	m->invalid_genre = b;
}

bool Filter::isInvalidGenre() const
{
	return m->invalid_genre;
}

bool Filter::isUseable() const
{
	if(m->mode == Filter::Mode::Invalid)
	{
		return false;
	}

	if(isInvalidGenre()){
		return true;
	}

	QStringList filters = filtertext(false);
	if(filters.join("").size() < 3)
	{
		return false;
	}

	return true;
}


QString Filter::text(Filter::Mode mode)
{
	switch(mode)
	{
		case Filter::Mode::Filename:
			return Lang::get(Lang::Filename);

		case Filter::Mode::Fulltext:
			return  Lang::get(Lang::Artists) + ", " +
					Lang::get(Lang::Albums) + ", " +
					Lang::get(Lang::Tracks);

		case Filter::Mode::Genre:
			return Lang::get(Lang::Genre);

		default:
			return QString();
	}
}

void Filter::clear()
{
	m->filtertext = QString();
	m->mode = Mode::Fulltext;
}


