
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
#include "Utils/Language/Language.h"

#include <QAction>

#include <algorithm>

using namespace Library;

using HeaderType=ColumnHeader::HeaderType;

struct ColumnHeader::Private
{
	QAction*		action=nullptr;
	int 			preferred_size;
	int				default_size;

	SortOrder		sort_asc;
	SortOrder		sort_desc;
	HeaderType		type;

	bool 			switchable;
	bool			stretchable;

	Private(ColumnHeader* parent, HeaderType type, bool switchable, SortOrder sort_asc, SortOrder sort_desc, int sz) :
		preferred_size(sz),
		default_size(sz),
		sort_asc(sort_asc),
		sort_desc(sort_desc),
		type(type),
		switchable(switchable),
		stretchable(false)
	{
		action = new QAction(parent);
		action->setChecked(true);
		action->setCheckable(switchable);
	}
};

ColumnHeader::~ColumnHeader() = default;

ColumnHeader::ColumnHeader(HeaderType type, bool switchable, SortOrder sort_asc, SortOrder sort_desc, int preferred_size, bool stretchable)
{
	m = Pimpl::make<Private>(this, type, switchable, sort_asc, sort_desc, preferred_size);
	m->stretchable = stretchable;
}

bool ColumnHeader::stretchable() const
{
	return m->stretchable;
}

int ColumnHeader::default_size() const
{
	return m->default_size;
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

bool ColumnHeader::is_action_checked() const
{
	if(!m->switchable){
		return true;
	}

	return m->action->isChecked();
}

void ColumnHeader::retranslate()
{
	m->action->setText(this->title());
}

QString ColumnHeader::title() const
{
	switch(m->type)
	{
		case ColumnHeader::Sharp:
			return "#";
		case ColumnHeader::Artist:
			return Lang::get(Lang::Artist).toFirstUpper();
		case ColumnHeader::Album:
			return Lang::get(Lang::Album).toFirstUpper();
		case ColumnHeader::Title:
			return Lang::get(Lang::Title).toFirstUpper();
		case ColumnHeader::NumTracks:
			return Lang::get(Lang::NumTracks).toFirstUpper();
		case ColumnHeader::Duration:
			return Lang::get(Lang::Duration).toFirstUpper();
		case ColumnHeader::DurationShort:
			return Lang::get(Lang::DurationShort).toFirstUpper();
		case ColumnHeader::Year:
			return Lang::get(Lang::Year).toFirstUpper();
		case ColumnHeader::Rating:
			return Lang::get(Lang::Rating).toFirstUpper();
		case ColumnHeader::Bitrate:
			return Lang::get(Lang::Bitrate).toFirstUpper();
		case ColumnHeader::Filesize:
			return Lang::get(Lang::Filesize).toFirstUpper();
		case ColumnHeader::Discnumber:
			return Lang::get(Lang::Disc);
		default:
			return QString();
	}
}
