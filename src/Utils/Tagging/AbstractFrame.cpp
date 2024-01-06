/* AbstractFrame.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "AbstractFrame.h"

#include <QString>

#include <taglib/tstring.h>

struct Tagging::AbstractFrameHelper::Private
{
	QByteArray key;

	explicit Private(QByteArray key) :
		key {std::move(key)} {}
};

Tagging::AbstractFrameHelper::AbstractFrameHelper(const QByteArray& key) :
	m {Pimpl::make<Private>(key)} {}

Tagging::AbstractFrameHelper::~AbstractFrameHelper() = default;

QByteArray Tagging::AbstractFrameHelper::key() const { return m->key; }

TagLib::ByteVector Tagging::AbstractFrameHelper::tagKey() const { return {m->key.constData()}; }
