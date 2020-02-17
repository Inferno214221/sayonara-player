/* CrossFader.cpp */

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

	double					fadingStep;
	double					startVolume;
	double					targetVolume;

	Fadeable::FadeMode		mode;

	Private(Fadeable* parent) :
		fadingStep(0),
		startVolume(0),
		targetVolume(0),
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

bool Fadeable::initFader(Fadeable::FadeMode mode)
{
	m->mode = mode;
	m->timer->stop();

	double currentVolume = GetSetting(Set::Engine_Vol) / 100.0;
	int fadingTime = GetSetting(Set::Engine_CrossFaderTime);

	if(mode == Fadeable::FadeMode::NoFading){
		return false;
	}

	else if(mode == Fadeable::FadeMode::FadeOut)
	{
		m->startVolume = currentVolume;
		m->targetVolume = 0;
	}

	else if(mode == Fadeable::FadeMode::FadeIn)
	{
		m->startVolume = 0;
		m->targetVolume = currentVolume;
	}

	this->setInternalVolume(m->startVolume);

	m->fadingStep = FadingStepTime * (m->targetVolume - m->startVolume) / (fadingTime * 1.0);

	spLog(Log::Develop, this) << "Fading step: " << m->fadingStep << " every " << FadingStepTime << "ms from Volume " << m->startVolume << " to " << m->targetVolume;

	bool b = std::fabs(m->fadingStep) > 0.00001;
	if(b) {
		m->timer->start(FadingStepTime);
	}

	return b;
}

void Fadeable::fadeIn()
{
	bool b = initFader(Fadeable::FadeMode::FadeIn);
	if(!b){
		return;
	}

	play();
	getFadeInHandler();
}

void Fadeable::fadeOut()
{
	bool b = initFader(Fadeable::FadeMode::FadeOut);
	if(!b){
		return;
	}

	getFadeOutHandler();
}

void Fadeable::timedOut()
{
	double currentVolume = internalVolume();
	double new_volume = currentVolume + m->fadingStep;

	bool fade_allowed = (new_volume <= 1.0) && (new_volume >= 0);

	if(m->mode == Fadeable::FadeMode::FadeIn)
	{
		fade_allowed &= (new_volume < m->targetVolume);
	}

	else if(m->mode == Fadeable::FadeMode::FadeOut)
	{
		fade_allowed &= (new_volume > m->targetVolume);
		if(!fade_allowed){
			this->stop();
		}
	}

	if(fade_allowed)
	{
		if(m->mode == Fadeable::FadeMode::FadeIn)
		{
			spLog(Log::Crazy, this) << "Set volume from " << currentVolume << " to " << new_volume;
		}

		setInternalVolume(new_volume);
	}

	else {
		m->timer->stop();
	}
}


MilliSeconds Fadeable::fadingTimeMs() const
{
	if(GetSetting(Set::Engine_CrossFaderActive))
	{
		auto ms = GetSetting(Set::Engine_CrossFaderTime);
		return MilliSeconds(ms);
	}

	return 0;
}

void Fadeable::abortFader()
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

	connect(m->timer, &QTimer::timeout, this, &CrossFadeableTimer::timedOut);
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

void CrossFadeableTimer::timedOut()
{
	m->fadeable->timedOut();
}
