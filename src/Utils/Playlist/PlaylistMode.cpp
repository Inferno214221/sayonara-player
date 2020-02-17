/* PlaylistMode.cpp */

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

#include "PlaylistMode.h"
#include "Utils/Logger/Logger.h"
#include <QStringList>

using Playlist::Mode;

struct Mode::Private
{
	Mode::State	rep1;
	Mode::State	repAll;
	Mode::State	append;
	Mode::State	shuffle;
	Mode::State	dynamic;
	Mode::State	gapless;

	Private() :
		rep1(Mode::Off),
		repAll(Mode::On),
		append(Mode::Off),
		shuffle(Mode::Off),
		dynamic(Mode::Off),
		gapless(Mode::Off)
	{}

	Private(const Private& other) :
		CASSIGN(rep1),
		CASSIGN(repAll),
		CASSIGN(append),
		CASSIGN(shuffle),
		CASSIGN(dynamic),
		CASSIGN(gapless)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(rep1);
		ASSIGN(repAll);
		ASSIGN(append);
		ASSIGN(shuffle);
		ASSIGN(dynamic);
		ASSIGN(gapless);

		return *this;
	}


	static
	Mode::State set_state(bool active, bool enabled)
	{
		uint8_t ret = Mode::Off;
		if(active){
			ret |= Mode::On;
		}

		if(!enabled){
			ret |= Mode::Disabled;
		}

		return (Mode::State) ret;
	}

};


Mode::Mode()
{
	m = Pimpl::make<Private>();
}

Mode::~Mode() = default;

Mode::Mode(const Playlist::Mode& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

Mode& Mode::operator=(const Playlist::Mode& other)
{
	*m = *(other.m);
	return *this;
}

Mode::State Mode::rep1() const { return m->rep1; }
Mode::State Mode::repAll() const { return m->repAll; }
Mode::State Mode::append() const { return m->append; }
Mode::State Mode::shuffle() const { return m->shuffle; }
Mode::State Mode::dynamic() const { return m->dynamic; }
Mode::State Mode::gapless() const { return m->gapless; }

void Mode::setRep1(Mode::State state){ m->rep1 = state; }
void Mode::setRepAll(Mode::State state){ m->repAll = state; }
void Mode::setAppend(Mode::State state){ m->append = state; }
void Mode::setShuffle(Mode::State state){ m->shuffle = state; }
void Mode::setDynamic(Mode::State state){ m->dynamic = state; }
void Mode::setGapless(Mode::State state){ m->gapless = state; }

void Mode::setRep1(bool on, bool enabled){ m->rep1 = Private::set_state(on, enabled); }
void Mode::setRepAll(bool on, bool enabled){ m->repAll = Private::set_state(on, enabled); }
void Mode::setAppend(bool on, bool enabled){ m->append = Private::set_state(on, enabled); }
void Mode::setShuffle(bool on, bool enabled){ m->shuffle = Private::set_state(on, enabled); }
void Mode::setDynamic(bool on, bool enabled){ m->dynamic = Private::set_state(on, enabled); }
void Mode::setGapless(bool on, bool enabled){ m->gapless = Private::set_state(on, enabled); }

bool Mode::isActive(Mode::State pl)
{
	uint8_t ipl = (uint8_t) pl;
	return ( ipl & Mode::On );
}

bool Mode::isEnabled(Mode::State pl){
	uint8_t ipl = (uint8_t) pl;
	return (( ipl & Mode::Disabled ) == 0);
}

bool Mode::isActiveAndEnabled(Mode::State pl)
{
	return (isEnabled(pl) && isActive(pl));
}

void Mode::print()
{
	spLog(Log::Debug, this) << "rep1 = "   << (int) m->rep1 << ", "
		<< "repAll = "  << (int) m->repAll << ", "
		<< "append = "  << (int) m->append <<", "
		<< "dynamic = " << (int) m->dynamic << ","
		<< "gapless = " << (int) m->gapless;
}

QString Mode::toString() const
{
	QString str;
	str += QString::number((int) m->append)  + QString(",");
	str += QString::number((int) m->repAll)  + QString(",");
	str += QString::number((int) m->rep1)    + QString(",");
	str += "0,";
	str += QString::number((int) m->shuffle) + QString(",");
	str += QString::number((int) m->dynamic) + QString(",");
	str += QString::number((int) m->gapless);

	return str;
}

#define state_cast(x) static_cast<Mode::State>( x .toInt())
bool Mode::loadFromString(const QString& str)
{
	QStringList list = str.split(',');

	if(list.size() < 6) {
		return false;
	}

	this->setAppend( state_cast(list[0]) );
	this->setRepAll( state_cast(list[1]) );
	this->setRep1(   state_cast(list[2]) );
	this->setShuffle(state_cast(list[4]) );
	this->setDynamic(state_cast(list[5]) );

	if(list.size() > 6){
		this->setGapless(state_cast(list[6]));
	}

	return true;
}

bool Mode::operator==(const Mode& pm) const
{
	if(pm.append() != m->append) return false;
	if(pm.repAll() != m->repAll) return false;
	if(pm.rep1() != m->rep1) return false;
	if(pm.shuffle() != m->shuffle) return false;
	if(pm.dynamic() != m->dynamic) return false;
	if(pm.gapless() != m->gapless) return false;

	return true;
}
