/* DelayedPlayHandler.cpp */

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

#include "DelayedPlayable.h"
#include "Utils/Utils.h"
#include <QTimer>

using PipelineExtensions::DelayedPlayableLogic;
using PipelineExtensions::DelayedPlayable;

struct DelayedPlayableLogic::Private
{
	DelayedPlayable* dph=nullptr;
	QTimer* timer=nullptr;

	Private(DelayedPlayable* dph) :
		dph(dph)
	{
		timer = new QTimer();
		timer->setTimerType(Qt::PreciseTimer);
		timer->setSingleShot(true);
	}

	~Private()
	{
		while(timer->isActive())
		{
			timer->stop();
			::Util::sleepMs(100);
		}

		delete timer; timer = nullptr;
	}
};

DelayedPlayableLogic::DelayedPlayableLogic(DelayedPlayable* dph)
{
	m = Pimpl::make<Private>(dph);

	connect(m->timer, &QTimer::timeout, this, [=](){
		m->dph->play();
	});
}

DelayedPlayableLogic::~DelayedPlayableLogic() {}

void DelayedPlayableLogic::startTimer(MilliSeconds ms)
{
	m->timer->start(ms);
}

void DelayedPlayableLogic::stopTimer()
{
	m->timer->stop();
}

struct DelayedPlayable::Private
{
	DelayedPlayableLogic* logic=nullptr;

	Private(DelayedPlayable* dph)
	{
		logic = new DelayedPlayableLogic(dph);
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

void DelayedPlayable::playIn(MilliSeconds ms)
{
	abortDelayedPlaying();

	if(ms <= 0){
		play();
	}

	else {
		m->logic->startTimer(ms);
	}
}

void DelayedPlayable::abortDelayedPlaying()
{
	m->logic->stopTimer();
}

