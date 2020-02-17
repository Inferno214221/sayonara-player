/* LibraryItem.cpp */

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

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/Utils.h"

#include <QString>

#include <utility>

struct CustomField::Private
{
	QString displayName;
	QString value;
	QString id;

	Private(const QString& id, const QString& display_name, const QString& value) :
		displayName(display_name),
		value(value),
		id(id)
	{}

	Private(const Private& other) :
		CASSIGN(displayName),
		CASSIGN(value),
		CASSIGN(id)
	{}

	Private(Private&& other) noexcept :
		CMOVE(displayName),
		CMOVE(value),
		CMOVE(id)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(displayName);
		ASSIGN(value);
		ASSIGN(id);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(displayName);
		MOVE(value);
		MOVE(id);

		return *this;
	}
};


CustomField::CustomField(const QString& id, const QString& displayName, const QString& value)
{
	m = Pimpl::make<Private>(id, displayName, value);
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

QString CustomField::id() const
{
	return m->id;
}

QString CustomField::displayName() const
{
	return m->displayName;
}

QString CustomField::value() const
{
	return m->value;
}

static UniqueId static_unique_id=1;

struct LibraryItem::Private
{
	CustomFieldList		additionalData;
	QStringList			coverDownloadUrls;
	UniqueId			uniqueId;
	DbId				dbId;

	Private() :
		dbId(0)
	{
		uniqueId = (++static_unique_id);
	}

	Private(const Private& other) :
		CASSIGN(additionalData),
		CASSIGN(coverDownloadUrls),
		CASSIGN(dbId)
	{
		uniqueId = (++static_unique_id);

	}

	Private(Private&& other) noexcept :
		CMOVE(additionalData),
		CMOVE(coverDownloadUrls),
		CMOVE(uniqueId),
		CMOVE(dbId)
	{
		(void) uniqueId;
	}

	Private& operator=(const Private& other)
	{
		ASSIGN(additionalData);
		ASSIGN(coverDownloadUrls);
		ASSIGN(dbId);

		uniqueId = (++static_unique_id);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(additionalData);
		MOVE(coverDownloadUrls);
		MOVE(uniqueId);
		MOVE(dbId);

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

void LibraryItem::addCustomField(const CustomField& field)
{
	m->additionalData.push_back(field);
}

void LibraryItem::addCustomField(const QString& id, const QString& display_name, const QString& value)
{
	m->additionalData.push_back(CustomField(id, display_name, value));
}

void LibraryItem::replaceCustomField(const QString& id, const QString& display_name, const QString& value)
{
	for(int i=m->additionalData.size()-1; i>=0; i--)
	{
		CustomField field = m->additionalData.at(i);

		if(field.id() == id){
			m->additionalData.removeAt(i);
		}
	}

	this->addCustomField(id, display_name, value);
}

const CustomFieldList& LibraryItem::customFields() const
{
	return m->additionalData;
}


QString LibraryItem::customField(const QString& id) const
{
	for(const CustomField& field : m->additionalData)
	{
		if(field.id().compare(id, Qt::CaseInsensitive) == 0){
			return field.value();
		}
	}

	return "";
}


QString LibraryItem::customField(int idx) const
{
	if(idx < 0 || idx >= int(m->additionalData.size())){
		return "";
	}

	return m->additionalData[idx].value();
}

QStringList LibraryItem::coverDownloadUrls() const
{
	return m->coverDownloadUrls;
}

void LibraryItem::setCoverDownloadUrls(const QStringList& url)
{
	m->coverDownloadUrls = url;
}

DbId LibraryItem::databaseId() const
{
	return m->dbId;
}

void LibraryItem::setDatabaseId(DbId id)
{
	m->dbId = id;
}

void LibraryItem::print() const {}

UniqueId LibraryItem::uniqueId() const
{
	return m->uniqueId;
}

QHash<HashValue, QString> &LibraryItem::albumPool()
{
	static QHash<HashValue, QString> pool;
	return pool;
}

QHash<HashValue, QString> &LibraryItem::artistPool()
{
	static QHash<HashValue, QString> pool;
	return pool;
}
