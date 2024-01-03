/* CoverFetcherUrl.cpp
 *
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

#include "CoverFetcherUrl.h"

#include <QString>

using Cover::Fetcher::Url;

struct Url::Private
{
	QString identifier;
	QString url;

	Private(const QString& identifier, const QString& url) :
		identifier(identifier),
		url(url) {}
};

Cover::Fetcher::Url::Url() :
	Url(QString(), QString()) {}

Url::Url(const QString& identifier, const QString& url)
{
	m = Pimpl::make<Private>(identifier, url);
}

Url::Url(const Cover::Fetcher::Url& other) :
	Url(other.identifier(), other.url()) {}

Cover::Fetcher::Url& Url::operator=(const Cover::Fetcher::Url& other)
{
	m->identifier = other.identifier();
	m->url = other.url();

	return *this;
}

Url::~Url() = default;

void Url::setIdentifier(const QString& identifier)
{
	m->identifier = identifier;
}

QString Url::identifier() const
{
	return m->identifier;
}

void Url::setUrl(const QString& url)
{
	m->url = url;
}

QString Url::url() const
{
	return m->url;
}

bool Cover::Fetcher::Url::operator==(const Url& rhs) const
{
	return (this->identifier() == rhs.identifier() && this->url() == rhs.url());
}
