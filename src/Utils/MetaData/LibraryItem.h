/* LibraryItem.h */

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

#ifndef SAYONARA_LIBRARY_ITEM_H_
#define SAYONARA_LIBRARY_ITEM_H_

#include "Utils/Pimpl.h"
#include <QHash>

using HashValue=uint32_t;
using UniqueId=uint64_t;

class CustomField;
using CustomFieldList=QList<CustomField>;

/**
 * @brief The CustomField class
 * a CustomField is some additional entry than can be set for MetaData, Albums and Artists
 * and will be displayed on the Info Dialog
 * These custom fields are intendend for Plugins
 * @ingroup MetaDataHelper
 */
class CustomField
{
	PIMPL(CustomField)

public:
	CustomField(const QString& id, const QString& display_name, const QString& value);
	CustomField(const CustomField& other);
	CustomField(CustomField&& other) noexcept;

	CustomField& operator=(const CustomField& other);
	CustomField& operator=(CustomField&& other) noexcept;

	~CustomField();

	QString get_id() const;
	QString get_display_name() const;
	QString get_value() const;
};


/**
 * @brief The LibraryItem class
 * @ingroup MetaDataHelper
 */
class LibraryItem
{
	PIMPL(LibraryItem)

public:
	LibraryItem();
	LibraryItem(const LibraryItem& other);
	LibraryItem(LibraryItem&& other) noexcept;

	LibraryItem& operator=(const LibraryItem& other);
	LibraryItem& operator=(LibraryItem&& other) noexcept;

	virtual ~LibraryItem();

	void add_custom_field(const CustomField& field);
	void add_custom_field(const QString& id, const QString& display_name, const QString& value);
	void replace_custom_field(const QString& id, const QString& display_name, const QString& value);

	const CustomFieldList& get_custom_fields() const;
	QString get_custom_field(const QString& id) const;
	QString get_custom_field(int idx) const;

	QStringList cover_download_urls() const;
	void set_cover_download_urls(const QStringList& url);

	DbId db_id() const;
	void set_db_id(DbId id);

	virtual void print() const;

	UniqueId unique_id() const;

protected:
	static QHash<HashValue, QString>& album_pool();
	static QHash<HashValue, QString>& artist_pool();
};

#endif

