
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

#include "ColumnHeader.h"
#include "Utils/Language.h"

#include <QAction>

#include <algorithm>

using namespace Library;

using HeaderType=ColumnHeader::HeaderType;

struct ColumnHeader::Private
{
	QAction*		action=nullptr;
	int 			preferred_size;

	SortOrder		sort_asc;
	SortOrder		sort_desc;
	HeaderType		type;

	bool 			switchable;

	Private(ColumnHeader* parent, HeaderType type, bool switchable, SortOrder sort_asc, SortOrder sort_desc) :
		preferred_size(0),
		sort_asc(sort_asc),
		sort_desc(sort_desc),
		type(type),
		switchable(switchable)

	{
		action = new QAction(parent);
		action->setChecked(true);
		action->setCheckable(switchable);
	}
};

ColumnHeader::ColumnHeader(HeaderType type, bool switchable, SortOrder sort_asc, SortOrder sort_desc)
{
	m = Pimpl::make<Private>(this, type, switchable, sort_asc, sort_desc);
}

ColumnHeader::~ColumnHeader() {}

ColumnHeader::ColumnHeader(HeaderType type, bool switchable, SortOrder sort_asc, SortOrder sort_desc, int preferred_size) :
	ColumnHeader(type, switchable, sort_asc, sort_desc)
{
	m->preferred_size = preferred_size;
}

int ColumnHeader::preferred_size() const
{
	return m->preferred_size;
}

void ColumnHeader::set_preferred_size(int size)
{
	m->preferred_size = size;
}

SortOrder ColumnHeader::sortorder_asc() const
{
	return	m->sort_asc;
}

SortOrder ColumnHeader::sortorder_desc() const
{
	return m->sort_desc;
}

QAction* ColumnHeader::action()
{
	m->action->setText( this->title() );
	return m->action;
}

bool ColumnHeader::is_visible() const
{
	if(!m->switchable){
		return true;
	}

	return m->action->isChecked();
}

bool ColumnHeader::is_hidden() const
{
	return (!is_visible());
}

void ColumnHeader::retranslate()
{
	m->action->setText(this->title());
}

int ColumnHeaderList::visible_columns() const
{
	auto count = std::count_if(this->begin(), this->end(), [](ColumnHeaderPtr header){
		return header->is_visible();
	});

	return static_cast<int>(count);
}

int ColumnHeaderList::visible_column(int n) const
{
	if(n < 0 || n > this->size())
	{
		return -1;
	}

	for(int i=0; i<this->size(); i++)
	{
		ColumnHeaderPtr header = this->at(i);
		if(header->is_visible()){
			n--;
		}

		if(n < 0){
			return i;
		}
	}

	return -1;
}

QString ColumnHeader::title() const
{
	switch(m->type)
	{
		case ColumnHeader::Sharp:
			return "#";
		case ColumnHeader::Artist:
			return Lang::get(Lang::Artist);
		case ColumnHeader::Album:
			return Lang::get(Lang::Album);
		case ColumnHeader::Title:
			return Lang::get(Lang::Title);
		case ColumnHeader::NumTracks:
			return Lang::get(Lang::NumTracks);
		case ColumnHeader::Duration:
			return Lang::get(Lang::Duration);
		case ColumnHeader::DurationShort:
			return Lang::get(Lang::DurationShort);
		case ColumnHeader::Year:
			return Lang::get(Lang::Year);
		case ColumnHeader::Rating:
			return Lang::get(Lang::Rating);
		case ColumnHeader::Bitrate:
			return Lang::get(Lang::Bitrate);
		case ColumnHeader::Filesize:
			return Lang::get(Lang::Filesize);
		case ColumnHeader::Discnumber:
			return Lang::get(Lang::Disc);
		default:
			return QString();
	}
}
