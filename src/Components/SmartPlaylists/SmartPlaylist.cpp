/* SmartPlaylist.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "SmartPlaylist.h"

#include "Database/SmartPlaylists.h"
#include "Utils/Algorithm.h"

struct SmartPlaylist::Private
{
	int id;
	QList<int> values;
	std::shared_ptr<SmartPlaylists::StringConverter> stringConverter {nullptr};
	bool isRandomized;

	Private(const int id, const QList<int>& values, const bool isRandomized) :
		id {id},
		values {values},
		isRandomized {isRandomized} {}
};

SmartPlaylist::SmartPlaylist(const int id, const QList<int>& values, const bool isRandomized) :
	m {Pimpl::make<Private>(id, values, isRandomized)} {}

SmartPlaylist::~SmartPlaylist() = default;

SmartPlaylistDatabaseEntry SmartPlaylist::toDatabaseEntry() const
{
	return SmartPlaylistDatabaseEntry {
		id(),
		classType(),
		attributesToString(),
		isRandomized()
	};
}

QString SmartPlaylist::attributesToString() const
{
	auto valueStrings = QStringList {};
	Util::Algorithm::transform(m->values, valueStrings, [](const auto value) {
		return QString::number(value);
	});

	return valueStrings.join(',');
}

int SmartPlaylist::id() const { return m->id; }

void SmartPlaylist::setId(const int id) { m->id = id; }

int SmartPlaylist::count() const { return m->values.count(); }

int SmartPlaylist::value(const int index) const { return m->values[index]; }

void SmartPlaylist::setValue(const int index, const int value)
{
	if(Util::between(index, m->values))
	{
		m->values[index] = value;
	}
}

bool SmartPlaylist::isRandomized() const { return (isRandomizable() && m->isRandomized); }

void SmartPlaylist::setRandomized(const bool b) { m->isRandomized = b; }

bool SmartPlaylist::isRandomizable() const { return true; }

SmartPlaylists::InputFormat SmartPlaylist::inputFormat() const { return SmartPlaylists::InputFormat::Text; }

bool SmartPlaylist::canFetchTracks() const { return false; }

SmartPlaylists::StringConverterPtr SmartPlaylist::createConverter() const
{
	return std::make_shared<SmartPlaylists::StringConverter>();
}

SmartPlaylists::StringConverterPtr SmartPlaylist::stringConverter() const
{
	if(m->stringConverter != nullptr)
	{
		return m->stringConverter;
	}

	m->stringConverter = createConverter();
	return m->stringConverter;
}

QString SmartPlaylist::text(const int value) const
{
	return (value == 0)
	       ? QObject::tr("Between")
	       : QObject::tr("and");
}
