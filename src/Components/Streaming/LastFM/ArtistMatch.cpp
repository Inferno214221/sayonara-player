/* ArtistMatch.cpp */

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

#include "ArtistMatch.h"

#include <QMap>
#include <QStringList>

#include <algorithm>

using namespace LastFM;

struct ArtistMatch::Private
{
	QMap<ArtistDesc, double> very_good;
	QMap<ArtistDesc, double> well;
	QMap<ArtistDesc, double> poor;

	QString artist;

	Private() {}

	Private(const QString& artist_name) :
		artist(artist_name)
	{}

	Private(const Private& other) :
		CASSIGN(very_good),
		CASSIGN(well),
		CASSIGN(poor),
		CASSIGN(artist)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(very_good);
		ASSIGN(well);
		ASSIGN(poor);
		ASSIGN(artist);

		return *this;
	}
};

ArtistMatch::ArtistMatch()
{
	m = Pimpl::make<Private>();
}

ArtistMatch::ArtistMatch(const QString& artist_name)
{
	m = Pimpl::make<Private>(artist_name);
}


ArtistMatch::ArtistMatch(const ArtistMatch& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

ArtistMatch::~ArtistMatch() {}

bool ArtistMatch::is_valid() const
{
	return ( m->very_good.size() > 0 ||
			 m->well.size() > 0  ||
			 m->poor.size() > 0 );
}

bool ArtistMatch::operator ==(const ArtistMatch& other) const
{
	return (m->artist == other.m->artist);
}

ArtistMatch &ArtistMatch::operator =(const ArtistMatch &other)
{
	*m = *(other.m);

	return *this;
}

void ArtistMatch::add(const ArtistDesc& artist, double match)
{
	if(match > 0.15) {
		m->very_good[artist] = match;
	}

	else if(match > 0.05) {
		m->well[artist] = match;
	}

	else {
		m->poor[artist] = match;
	}
}

QMap<ArtistMatch::ArtistDesc, double> ArtistMatch::get(Quality q) const
{
	switch(q) {
		case Quality::Poor:
			return m->poor;
		case Quality::Well:
			return m->well;
		case Quality::Very_Good:
			return m->very_good;
	}

	return m->very_good;
}

QString ArtistMatch::get_artist_name() const
{
	return m->artist;
}

QString ArtistMatch::to_string() const
{
	QStringList lst;

	for(auto it=m->very_good.cbegin(); it != m->very_good.cend(); it++)
	{
		lst << QString::number(it.value()).left(5) + "\t" + it.key().to_string();
	}

	for(auto it=m->well.cbegin(); it != m->well.cend(); it++)
	{
		lst << QString::number(it.value()).left(5) + "\t" + it.key().to_string();
	}

	for(auto it=m->poor.cbegin(); it != m->poor.cend(); it++)
	{
		lst << QString::number(it.value()).left(5) + "\t" + it.key().to_string();
	}

	std::sort(lst.begin(), lst.end());
	return lst.join("\n");
}


ArtistMatch::ArtistDesc::ArtistDesc(const QString& artist_name, const QString& mbid)
{
	this->artist_name = artist_name;
	this->mbid = mbid;
}

bool ArtistMatch::ArtistDesc::operator ==(const ArtistMatch::ArtistDesc& other) const
{
	return (artist_name == other.artist_name);
}

bool ArtistMatch::ArtistDesc::operator <(const ArtistMatch::ArtistDesc& other) const
{
	return (artist_name < other.artist_name);
}

bool ArtistMatch::ArtistDesc::operator <=(const ArtistMatch::ArtistDesc& other) const
{
	return (artist_name <= other.artist_name);
}


QString ArtistMatch::ArtistDesc::to_string() const
{
	return mbid + "\t" + artist_name;
}
