/* LibraryItem.h */

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
	CustomField(const QString& id, const QString& displayName, const QString& value);
	CustomField(const CustomField& other);
	CustomField(CustomField&& other) noexcept;

	CustomField& operator=(const CustomField& other);
	CustomField& operator=(CustomField&& other) noexcept;

	~CustomField();

	QString id() const;
	QString displayName() const;
	QString value() const;
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

	void addCustomField(const CustomField& field);
	void addCustomField(const QString& id, const QString& displayName, const QString& value);
	void replaceCustomField(const QString& id, const QString& displayName, const QString& value);

	const CustomFieldList& customFields() const;
	QString customField(const QString& id) const;
	QString customField(int idx) const;

	QStringList coverDownloadUrls() const;
	void setCoverDownloadUrls(const QStringList& url);

	DbId databaseId() const;
	void setDatabaseId(DbId id);

	virtual void print() const;

	UniqueId uniqueId() const;

protected:
	static QHash<HashValue, QString>& albumPool();
	static QHash<HashValue, QString>& artistPool();
};

#endif

