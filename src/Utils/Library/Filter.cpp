/* Filter.cpp */

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

#include "Filter.h"
#include "Utils/Language.h"

using Library::Filter;

struct Filter::Private
{
	QString					filtertext;
	Filter::Mode			mode;
	Library::SearchModeMask search_mode;
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

Filter::~Filter() {}

bool Filter::operator ==(const Filter& other)
{
	bool same_filtertext = false;

	if(m->filtertext.size() < 3 && other.m->filtertext.size() < 3)
	{
		same_filtertext = true;
	}

	else if(m->filtertext.compare(other.m->filtertext, Qt::CaseInsensitive) == 0)
	{
		same_filtertext = true;
	}

	return( same_filtertext && (m->mode == other.mode()) );
}


QStringList Filter::filtertext(bool with_percent) const
{
	QStringList ret;
	const QStringList tmp = m->filtertext.split(",");

	for(QString str : tmp)
	{
		if(with_percent)
		{
			if(!str.startsWith('%')){
				str.prepend('%');
			}

			if(!str.endsWith('%')){
				str.append('%');
			}
		}

		if(!str.isEmpty())
		{
			ret << str;
		}
	}

	return ret;
}

QStringList Filter::search_mode_filtertext(bool with_percent) const
{
	QStringList ret;
	const QStringList tmp = m->filtertext.split(",");

	for(const QString& tmp_str : tmp)
	{
		QString str = ::Library::Utils::convert_search_string(tmp_str, m->search_mode);
		if(with_percent)
		{
			if(!str.startsWith('%')){
				str.prepend('%');
			}

			if(!str.endsWith('%')){
				str.append('%');
			}
		}

		if(!str.isEmpty())
		{
			ret << str;
		}
	}

	return ret;
}

void Filter::set_filtertext(const QString& str, ::Library::SearchModeMask search_mode)
{
	m->filtertext = str;
	m->search_mode = search_mode;
}

Filter::Mode Filter::mode() const
{
	return m->mode;
}

void Filter::set_mode(Filter::Mode mode)
{
	m->mode = mode;
}

bool Filter::cleared() const
{
	return m->filtertext.isEmpty();
}

QString Filter::get_text(Filter::Mode mode)
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


