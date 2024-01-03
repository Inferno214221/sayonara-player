/* PlaylistMode.cpp */

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

#include "PlaylistMode.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Algorithm.h"

#include <QStringList>

using Playlist::Mode;

namespace
{
	Mode::State setState(bool active, bool enabled)
	{
		auto ret = static_cast<uint8_t>(Mode::Off);
		if(active)
		{
			ret |= Mode::On;
		}

		if(!enabled)
		{
			ret |= Mode::Disabled;
		}

		return static_cast<Mode::State>(ret);
	}
}

struct Mode::Private
{
	Mode::State rep1;
	Mode::State repAll;
	Mode::State append;
	Mode::State shuffle;
	Mode::State dynamic;
	Mode::State gapless;

	Private() :
		rep1(Mode::Off),
		repAll(Mode::On),
		append(Mode::Off),
		shuffle(Mode::Off),
		dynamic(Mode::Off),
		gapless(Mode::Off) {}

	Private(const Private& other) = default;
	Private(Private&& other) noexcept = default;

	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) noexcept = default;

	bool isEqual(const Private& other) const
	{
		return (rep1 == other.rep1) &&
		       (repAll == other.repAll) &&
		       (append == other.append) &&
		       (shuffle == other.shuffle) &&
		       (dynamic == other.dynamic) &&
		       (gapless == other.gapless);
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

void Mode::setRep1(Mode::State state) { m->rep1 = state; }

void Mode::setRepAll(Mode::State state) { m->repAll = state; }

void Mode::setAppend(Mode::State state) { m->append = state; }

void Mode::setShuffle(Mode::State state) { m->shuffle = state; }

void Mode::setDynamic(Mode::State state) { m->dynamic = state; }

void Mode::setGapless(Mode::State state) { m->gapless = state; }

void Mode::setRep1(bool on, bool enabled) { m->rep1 = setState(on, enabled); }

void Mode::setRepAll(bool on, bool enabled) { m->repAll = setState(on, enabled); }

void Mode::setAppend(bool on, bool enabled) { m->append = setState(on, enabled); }

void Mode::setShuffle(bool on, bool enabled) { m->shuffle = setState(on, enabled); }

void Mode::setDynamic(bool on, bool enabled) { m->dynamic = setState(on, enabled); }

void Mode::setGapless(bool on, bool enabled) { m->gapless = setState(on, enabled); }

bool Mode::isActive(Mode::State state)
{
	const auto iState = static_cast<uint8_t>(state);
	return (iState & Mode::On);
}

bool Mode::isEnabled(Mode::State state)
{
	const auto iState = static_cast<uint8_t>(state);
	return ((iState & Mode::Disabled) == 0);
}

bool Mode::isActiveAndEnabled(Mode::State pl)
{
	return (isEnabled(pl) && isActive(pl));
}

void Mode::print()
{
	spLog(Log::Debug, this) << "rep1 = " << static_cast<int>(m->rep1) << ", "
	                        << "repAll = " << static_cast<int>(m->repAll) << ", "
	                        << "append = " << static_cast<int>(m->append) << ", "
	                        << "dynamic = " << static_cast<int>(m->dynamic) << ","
	                        << "gapless = " << static_cast<int>(m->gapless);
}

QString Mode::toString() const
{
	const QList<Mode::State> values
		{
			m->append,
			m->repAll,
			m->rep1,
			State::Off,
			m->shuffle,
			m->dynamic,
			m->gapless
		};

	QStringList stringList;
	Util::Algorithm::transform(values, stringList, [](const Mode::State state) {
		return QString::number(static_cast<int>(state));
	});

	return stringList.join(",");
}

Mode::State toState(const QString& str)
{
	return static_cast<Mode::State>(str.toInt());
}

bool Mode::loadFromString(const QString& str)
{
	const auto list = str.split(',');
	if(list.size() < 6)
	{
		return false;
	}

	setAppend(toState(list[0]));
	setRepAll(toState(list[1]));
	setRep1(toState(list[2]));
	// ignore 3
	setShuffle(toState(list[4]));
	setDynamic(toState(list[5]));

	if(list.size() > 6)
	{
		setGapless(toState(list[6]));
	}

	return true;
}

bool Mode::operator==(const Mode& pm) const
{
	return m->isEqual(*(pm.m));
}
