/* CrossFader.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

/* CrossFader.cpp */

#include "Fadeable.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QTimer>

#include <cmath>

using PipelineExtensions::Fadeable;

static const int FadingStepTime = 20;

struct Fadeable::Private
{
	CrossFadeableTimer*		timer = nullptr;

	double					fading_step;
	double					start_volume;
	double					target_volume;

	Fadeable::FadeMode		mode;

	Private(Fadeable* parent) :
		fading_step(0),
		start_volume(0),
		target_volume(0),
		mode(Fadeable::FadeMode::NoFading)
	{
		timer = new CrossFadeableTimer(parent);
	}

	~Private()
	{
		delete timer; timer=nullptr;
	}
};

Fadeable::Fadeable()
{
	m = Pimpl::make<Private>(this);
}

Fadeable::~Fadeable() = default;

bool Fadeable::init_fader(Fadeable::FadeMode mode)
{
	m->mode = mode;
	m->timer->stop();

	double current_volume = GetSetting(Set::Engine_Vol) / 100.0;

	int fading_time = GetSetting(Set::Engine_CrossFaderTime);

	if(mode == Fadeable::FadeMode::NoFading){
		return false;
	}

	else if(mode == Fadeable::FadeMode::FadeOut)
	{
		m->start_volume = current_volume;
		m->target_volume = 0;
	}

	else if(mode == Fadeable::FadeMode::FadeIn)
	{
		m->start_volume = 0;
		m->target_volume = current_volume;
	}

	this->set_internal_volume(m->start_volume);

	m->fading_step = FadingStepTime * (m->target_volume - m->start_volume) / (fading_time * 1.0);

	sp_log(Log::Develop, this) << "Fading step: " << m->fading_step << " every " << FadingStepTime << "ms from Volume " << m->start_volume << " to " << m->target_volume;

	bool b = std::fabs(m->fading_step) > 0.00001;
	if(b) {
		m->timer->start(FadingStepTime);
	}

	return b;
}

void Fadeable::fade_in()
{
	bool b = init_fader(Fadeable::FadeMode::FadeIn);
	if(!b){
		return;
	}

	play();
	fade_in_handler();
}

void Fadeable::fade_out()
{
	bool b = init_fader(Fadeable::FadeMode::FadeOut);
	if(!b){
		return;
	}

	fade_out_handler();
}

void Fadeable::timed_out()
{
	double current_volume = get_internal_volume();
	double new_volume = current_volume + m->fading_step;

	bool fade_allowed = (new_volume <= 1.0) && (new_volume >= 0);

	if(m->mode == Fadeable::FadeMode::FadeIn)
	{
		fade_allowed &= (new_volume < m->target_volume);
	}

	else if(m->mode == Fadeable::FadeMode::FadeOut)
	{
		fade_allowed &= (new_volume > m->target_volume);
		if(!fade_allowed){
			this->stop();
		}
	}

	if(fade_allowed)
	{
		if(m->mode == Fadeable::FadeMode::FadeIn)
		{
			sp_log(Log::Crazy, this) << "Set volume from " << current_volume << " to " << new_volume;
		}

		set_internal_volume(new_volume);
	}

	else {
		m->timer->stop();
	}
}


MilliSeconds Fadeable::get_fading_time_ms() const
{
	if(GetSetting(Set::Engine_CrossFaderActive))
	{
		auto ms = GetSetting(Set::Engine_CrossFaderTime);
		return MilliSeconds(ms);
	}

	return 0;
}

void Fadeable::abort_fader()
{
	m->timer->stop();
}

using PipelineExtensions::CrossFadeableTimer;

struct CrossFadeableTimer::Private
{
	QTimer*			timer=nullptr;
	Fadeable*	fadeable=nullptr;

	Private(Fadeable* fadeable) :
		fadeable(fadeable)
	{
		timer = new QTimer();
	}

	~Private()
	{
		timer->stop();
		delete timer; timer=nullptr;
	}
};

CrossFadeableTimer::CrossFadeableTimer(Fadeable* parent) :
	QObject()
{
	m = Pimpl::make<Private>(parent);

	connect(m->timer, &QTimer::timeout, this, &CrossFadeableTimer::timed_out);
}

CrossFadeableTimer::~CrossFadeableTimer() = default;

void CrossFadeableTimer::start(MilliSeconds ms)
{
	m->timer->start(ms);
}

void CrossFadeableTimer::stop()
{
	m->timer->stop();
}

void CrossFadeableTimer::timed_out()
{
	m->fadeable->timed_out();
}
