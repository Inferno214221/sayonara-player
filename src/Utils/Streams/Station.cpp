/* Station.cpp
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

#include "Station.h"

#include <QString>

Station::Station() = default;
Station::Station(const Station&) = default;
Station::~Station() = default;

Station& Station::station(const Station& /*unused*/)
{
	return *this;
}

struct Stream::Private
{
	QString name;
	QString url;
	QString userAgent;
	bool isUpdatable;

	Private(QString name, QString url, const bool isUpdatable, QString userAgent) :
		name(std::move(name)),
		url(std::move(url)),
		userAgent(std::move(userAgent)),
		isUpdatable(isUpdatable) {}
};

Stream::Stream() :
	Stream(QString(), QString(), true) {}

Stream::Stream(const QString& name, const QString& url, const bool isUpdatable, const QString& userAgent) :
	m {Pimpl::make<Private>(name, url, isUpdatable, userAgent)} {}

Stream::~Stream() = default;

Stream::Stream(const Stream& other) :
	Stream()
{
	*m = *(other.m);
}

Stream& Stream::operator=(const Stream& other)
{
	*m = *(other.m);
	return *this;
}

QString Stream::name() const { return m->name; }

void Stream::setName(const QString& name) { m->name = name; }

QString Stream::url() const { return m->url; }

void Stream::setUrl(const QString& url) { m->url = url; }

bool Stream::isUpdatable() const { return m->isUpdatable; }

QString Stream::userAgent() const { return m->userAgent; }

struct Podcast::Private
{
	QString name;
	QString url;
	QString userAgent;
	bool reversed;

	Private(QString name, QString url, const bool reversed, QString userAgent) :
		name(std::move(name)),
		url(std::move(url)),
		userAgent(std::move(userAgent)),
		reversed(reversed) {}
};

Podcast::Podcast() :
	Podcast(QString(), QString(), false) {}

Podcast::Podcast(const QString& name, const QString& url, const bool reversed, const QString& userAgent) :
	m {Pimpl::make<Private>(name, url, reversed, userAgent)} {}

Podcast::Podcast(const Podcast& other) :
	Podcast()
{
	*m = *(other.m);
}

Podcast::~Podcast() = default;

QString Podcast::name() const { return m->name; }

void Podcast::setName(const QString& name) { m->name = name; }

QString Podcast::url() const { return m->url; }

void Podcast::setUrl(const QString& url) { m->url = url; }

bool Podcast::reversed() const { return m->reversed; }

void Podcast::setReversed(bool b) { m->reversed = b; }

QString Podcast::userAgent() const { return m->userAgent; }

Podcast& Podcast::operator=(const Podcast& other)
{
	*m = *(other.m);
	return *this;
}
