/* DelayedPlayHandler.cpp */

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

#include "DelayedPlayable.h"
#include "Utils/Utils.h"
#include <QTimer>

using PipelineExtensions::Logic;
using PipelineExtensions::DelayedPlayable;

struct Logic::Private
{
	DelayedPlayable* dph=nullptr;
	QTimer* t=nullptr;

	Private(DelayedPlayable* dph) :
		dph(dph)
	{
		t = new QTimer();
		t->setTimerType(Qt::PreciseTimer);
		t->setSingleShot(true);
	}

	~Private()
	{
		while(t->isActive())
		{
			t->stop();
			::Util::sleep_ms(100);
		}

		delete t; t = nullptr;
	}
};

Logic::Logic(DelayedPlayable* dph)
{
	m = Pimpl::make<Private>(dph);

	connect(m->t, &QTimer::timeout, this, [=](){
		m->dph->play();
	});
}

Logic::~Logic() {}

void Logic::start_timer(MilliSeconds ms)
{
	m->t->start(ms);
}

void Logic::stop_timer()
{
	m->t->stop();
}

struct DelayedPlayable::Private
{
	Logic* logic=nullptr;

	Private(DelayedPlayable* dph)
	{
		logic = new Logic(dph);
	}

	~Private()
	{
		delete logic; logic = nullptr;
	}
};

DelayedPlayable::DelayedPlayable()
{
	m = Pimpl::make<Private>(this);
}

DelayedPlayable::~DelayedPlayable() {}

void DelayedPlayable::play_in(MilliSeconds ms)
{
	abort_delayed_playing();

	if(ms <= 0){
		play();
	}

	else {
		m->logic->start_timer(ms);
	}
}

void DelayedPlayable::abort_delayed_playing()
{
	m->logic->stop_timer();
}

