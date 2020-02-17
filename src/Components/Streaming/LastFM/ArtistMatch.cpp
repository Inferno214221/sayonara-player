/* ArtistMatch.cpp */

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

#include "ArtistMatch.h"

#include <QMap>
#include <QStringList>

#include <algorithm>

using namespace LastFM;

struct ArtistMatch::Private
{
	QMap<ArtistDesc, double> veryGood;
	QMap<ArtistDesc, double> well;
	QMap<ArtistDesc, double> poor;

	QString artist;

	Private() {}

	Private(const QString& artistName) :
		artist(artistName)
	{}

	Private(const Private& other) :
		CASSIGN(veryGood),
		CASSIGN(well),
		CASSIGN(poor),
		CASSIGN(artist)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(veryGood);
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

ArtistMatch::ArtistMatch(const QString& artistName)
{
	m = Pimpl::make<Private>(artistName);
}


ArtistMatch::ArtistMatch(const ArtistMatch& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

ArtistMatch::~ArtistMatch() = default;

bool ArtistMatch::isValid() const
{
	return ( m->veryGood.size() > 0 ||
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
		m->veryGood[artist] = match;
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
			return m->veryGood;
	}

	return m->veryGood;
}

QString ArtistMatch::artistName() const
{
	return m->artist;
}

QString ArtistMatch::toString() const
{
	QStringList lst;

	for(auto it=m->veryGood.cbegin(); it != m->veryGood.cend(); it++)
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


ArtistMatch::ArtistDesc::ArtistDesc(const QString& artistName, const QString& mbid)
{
	this->artistName = artistName;
	this->mbid = mbid;
}

bool ArtistMatch::ArtistDesc::operator ==(const ArtistMatch::ArtistDesc& other) const
{
	return (artistName == other.artistName);
}

bool ArtistMatch::ArtistDesc::operator <(const ArtistMatch::ArtistDesc& other) const
{
	return (artistName < other.artistName);
}

bool ArtistMatch::ArtistDesc::operator <=(const ArtistMatch::ArtistDesc& other) const
{
	return (artistName <= other.artistName);
}


QString ArtistMatch::ArtistDesc::to_string() const
{
	return mbid + "\t" + artistName;
}
