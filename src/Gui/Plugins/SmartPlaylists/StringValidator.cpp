/* StringValidator.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "StringValidator.h"
#include "Components/SmartPlaylists/SmartPlaylist.h"

struct StringValidator::Private
{
	StringConverterPtr stringConverter = std::make_shared<SmartPlaylists::StringConverter>();
};

QValidator::State StringValidator::validate(QString& str, [[maybe_unused]] int& i) const
{
	return (str.isEmpty() || m->stringConverter->stringToInt(str).has_value())
	       ? Acceptable
	       : Invalid;
}

StringValidator::StringValidator(QObject* parent) :
	QValidator(parent),
	m {Pimpl::make<Private>()} {}

void StringValidator::setStringConverter(const StringConverterPtr& stringConverter)
{
	m->stringConverter = stringConverter;
}

StringValidator::~StringValidator() = default;
