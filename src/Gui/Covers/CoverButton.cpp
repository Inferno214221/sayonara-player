/* CoverButton.cpp */

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

#include "CoverButton.h"
#include "GUI_AlternativeCovers.h"

#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverChangeNotifier.h"

#include "Utils/CoverUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QMenu>
#include <QThread>

using Gui::ImageButton;
using Gui::CoverButton;
using Cover::Location;
using Cover::Lookup;
using Cover::ChangeNotfier;
using Util::Covers::Source;
using CoverButtonBase = Gui::WidgetTemplate<QPushButton>;

struct CoverButton::Private
{
	QString coverHash;
	Location coverLocation;

	Lookup* coverLookup = nullptr;
	Source coverSource;
	bool silent;
	bool alternativeSearchEnabled;

	Private() :
		coverSource(Source::Unknown),
		silent(false),
		alternativeSearchEnabled(true) {}
};

CoverButton::CoverButton(QWidget* parent) :
	Gui::WidgetTemplate<ImageButton>(parent)
{
	m = Pimpl::make<CoverButton::Private>();

	this->setObjectName("CoverButton");
	this->setToolTip(tr("Search an alternative cover"));

	auto* cn = Cover::ChangeNotfier::instance();
	connect(cn, &Cover::ChangeNotfier::sigCoversChanged, this, &CoverButton::coversChanged);
	connect(this, &ImageButton::sigPixmapChanged, this, &CoverButton::sigCoverChanged);
	connect(this, &ImageButton::sigTriggered, this, &CoverButton::trigger);

	ListenSetting(Set::Player_FadingCover, CoverButton::coverFadingChanged);
}

CoverButton::~CoverButton()
{
	if(m->coverLookup)
	{
		m->coverLookup->stop();
		m->coverLookup->deleteLater();
	}
}

void CoverButton::setAlternativeSearchEnabled(bool b)
{
	m->alternativeSearchEnabled = b;
}

bool CoverButton::isAlternativeSearchEnabled() const
{
	return m->alternativeSearchEnabled;
}

void CoverButton::coverFadingChanged()
{
	this->setFadingEnabled(GetSetting(Set::Player_FadingCover));
}

void CoverButton::trigger()
{
	if(m->coverSource == Source::AudioFile && !isSilent())
	{
		emit sigRejected();
		return;
	}

	if(m->alternativeSearchEnabled)
	{
		auto* alternativeCover = new GUI_AlternativeCovers(m->coverLocation,
		                                                   m->silent,
		                                                   this->parentWidget());

		connect(alternativeCover,
		        &GUI_AlternativeCovers::sigCoverChanged,
		        this,
		        &CoverButton::alternativeCoverFetched);
		connect(alternativeCover,
		        &GUI_AlternativeCovers::sigClosed,
		        alternativeCover,
		        &GUI_AlternativeCovers::deleteLater);

		alternativeCover->show();
	}

	else
	{
		emit sigRejected();
	}
}

void CoverButton::setCoverLocation(const Location& cl)
{
	if(!m->coverHash.isEmpty() && cl.hash() == m->coverHash)
	{
		return;
	}

	m->coverHash = cl.hash();

	if(!cl.isValid())
	{
		this->showDefaultPixmap();
	}

	m->coverLocation = cl;

	if(cl.hash().isEmpty() || !cl.isValid())
	{
		return;
	}

	if(!m->coverLookup)
	{
		m->coverLookup = new Lookup(cl, 1, this);

		connect(m->coverLookup, &Lookup::sigCoverFound, this, &CoverButton::setPixmap);
		connect(m->coverLookup, &Lookup::sigFinished, this, &CoverButton::coverLookupFinished);
	}

	else
	{
		m->coverLookup->setCoverLocation(cl);
	}

	m->coverLookup->start();
}

void CoverButton::coverLookupFinished(bool success)
{
	if(!success)
	{
		spLog(Log::Warning, this) << "Cover lookup finished: false";
		this->showDefaultPixmap();
	}

	auto* lookup = dynamic_cast<Cover::Lookup*>(sender());
	m->coverSource = lookup->source();
}

void CoverButton::coversChanged()
{
	if(!isSilent())
	{
		m->coverHash.clear();
		this->setCoverLocation(m->coverLocation);
	}
}

void CoverButton::alternativeCoverFetched(const Location& cl)
{
	m->coverHash.clear();
	m->coverSource = Source::Unknown;

	if(!isSilent())
	{
		if(cl.isValid())
		{
			ChangeNotfier::instance()->shout();
		}
	}

	else
	{
		this->setPixmapPath(cl.alternativePath());
	}
}

void CoverButton::setSilent(bool silent)
{
	m->silent = silent;
}

bool CoverButton::isSilent() const
{
	return m->silent;
}
