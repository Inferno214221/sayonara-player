/* LibraryItem.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/Utils.h"

#include <QString>

#include <utility>

struct CustomField::Private
{
	QString display_name;
	QString value;
	QString id;

	Private(const QString& id, const QString& display_name, const QString& value) :
		display_name(display_name),
		value(value),
		id(id)
	{}

	Private(const Private& other) :
		CASSIGN(display_name),
		CASSIGN(value),
		CASSIGN(id)
	{}

	Private(Private&& other) noexcept :
		CMOVE(display_name),
		CMOVE(value),
		CMOVE(id)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(display_name);
		ASSIGN(value);
		ASSIGN(id);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(display_name);
		MOVE(value);
		MOVE(id);

		return *this;
	}
};


CustomField::CustomField(const QString& id, const QString& display_name, const QString& value)
{
	m = Pimpl::make<Private>(id, display_name, value);
}

CustomField::CustomField(const CustomField &other)
{
	m = Pimpl::make<Private>(*(other.m));
}

CustomField::CustomField(CustomField&& other) noexcept
{
	m = Pimpl::make<Private>
	(
		std::move(*(other.m))
	);
}

CustomField& CustomField::operator=(const CustomField& other)
{
	(*m) = *(other.m);
	return *this;
}

CustomField& CustomField::operator=(CustomField&& other) noexcept
{
	(*m) = std::move(*(other.m));
	return *this;
}

CustomField::~CustomField() {}

QString CustomField::get_id() const
{
	return m->id;
}

QString CustomField::get_display_name() const
{
	return m->display_name;
}

QString CustomField::get_value() const
{
	return m->value;
}

static UniqueId static_unique_id=1;

struct LibraryItem::Private
{
	CustomFieldList		additional_data;
	QStringList			cover_download_urls;
	UniqueId			unique_id;
	DbId				db_id;

	Private() :
		db_id(0)
	{
		unique_id = (++static_unique_id);
	}

	Private(const Private& other) :
		CASSIGN(additional_data),
		CASSIGN(cover_download_urls),
		CASSIGN(db_id)
	{
		unique_id = (++static_unique_id);

	}

	Private(Private&& other) noexcept :
		CMOVE(additional_data),
		CMOVE(cover_download_urls),
		CMOVE(unique_id),
		CMOVE(db_id)
	{
		(void) unique_id;
	}

	Private& operator=(const Private& other)
	{
		ASSIGN(additional_data);
		ASSIGN(cover_download_urls);
		ASSIGN(db_id);

		unique_id = (++static_unique_id);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(additional_data);
		MOVE(cover_download_urls);
		MOVE(unique_id);
		MOVE(db_id);

		return *this;
	}
};

LibraryItem::LibraryItem()
{
	m = Pimpl::make<Private>();
}

LibraryItem::LibraryItem(const LibraryItem& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

LibraryItem::LibraryItem(LibraryItem&& other) noexcept
{
	m = Pimpl::make<Private>(
		std::move(*(other.m))
	);
}

LibraryItem& LibraryItem::operator=(const LibraryItem& other)
{
	(*m) = *(other.m);
	return *this;
}

LibraryItem& LibraryItem::operator=(LibraryItem&& other) noexcept
{
	(*m) = std::move(*(other.m));
	return *this;
}

LibraryItem::~LibraryItem() {}

void LibraryItem::add_custom_field(const CustomField& field)
{
	m->additional_data.push_back(field);
}

void LibraryItem::add_custom_field(const QString& id, const QString& display_name, const QString& value)
{
	m->additional_data.push_back(CustomField(id, display_name, value));
}

void LibraryItem::replace_custom_field(const QString& id, const QString& display_name, const QString& value)
{
	for(int i=m->additional_data.size()-1; i>=0; i--)
	{
		CustomField field = m->additional_data.at(i);

		if(field.get_id() == id){
			m->additional_data.removeAt(i);
		}
	}

	this->add_custom_field(id, display_name, value);
}

const CustomFieldList& LibraryItem::get_custom_fields() const
{
	return m->additional_data;
}


QString LibraryItem::get_custom_field(const QString& id) const
{
	for(const CustomField& field : m->additional_data)
	{
		if(field.get_id().compare(id, Qt::CaseInsensitive) == 0){
			return field.get_value();
		}
	}

	return "";
}


QString LibraryItem::get_custom_field(int idx) const
{
	if(idx < 0 || idx >= int(m->additional_data.size())){
		return "";
	}

	return m->additional_data[idx].get_value();
}

QStringList LibraryItem::cover_download_urls() const
{
	return m->cover_download_urls;
}

void LibraryItem::set_cover_download_urls(const QStringList& url)
{
	m->cover_download_urls = url;
}

DbId LibraryItem::db_id() const
{
	return m->db_id;
}

void LibraryItem::set_db_id(DbId id)
{
	m->db_id = id;
}

void LibraryItem::print() const {}

UniqueId LibraryItem::unique_id() const
{
	return m->unique_id;
}

QHash<HashValue, QString> &LibraryItem::album_pool()
{
	static QHash<HashValue, QString> pool;
	return pool;
}

QHash<HashValue, QString> &LibraryItem::artist_pool()
{
	static QHash<HashValue, QString> pool;
	return pool;
}
