/* CoverButton.cpp */

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

using namespace Cover;
using Gui::ImageButton;
using Gui::CoverButton;
using Util::Covers::Source;

struct CoverButton::Private
{
	QString coverHash;
	Location coverLocation;

	Cover::ChangeNotfier* coverChangeNotifier {ChangeNotfier::instance()};
	Lookup* coverLookup {nullptr};
	Source coverSource {Source::Unknown};
	bool silent {false};
	bool alternativeSearchEnabled {true};
};

CoverButton::CoverButton(QWidget* parent) :
	Gui::WidgetTemplate<ImageButton>(parent)
{
	m = Pimpl::make<CoverButton::Private>();

	this->setObjectName("CoverButton");

	connect(m->coverChangeNotifier, &ChangeNotfier::sigCoversChanged, this, &CoverButton::coversChanged);
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
		auto* alternativeCover =
			new GUI_AlternativeCovers(m->coverLocation, m->silent, this->parentWidget());

		connect(alternativeCover, &GUI_AlternativeCovers::sigCoverChanged, this, &CoverButton::alternativeCoverFetched);
		connect(alternativeCover, &GUI_AlternativeCovers::sigClosed,
		        alternativeCover, &GUI_AlternativeCovers::deleteLater);

		alternativeCover->show();
	}

	else
	{
		emit sigRejected();
	}
}

void CoverButton::setCoverLocation(const Location& coverLocation)
{
	if(!m->coverHash.isEmpty() && coverLocation.hash() == m->coverHash)
	{
		return;
	}

	m->coverHash = coverLocation.hash();

	if(!coverLocation.isValid())
	{
		this->showDefaultPixmap();
	}

	m->coverLocation = coverLocation;

	if(coverLocation.hash().isEmpty() || !coverLocation.isValid())
	{
		return;
	}

	if(!m->coverLookup)
	{
		m->coverLookup = new Lookup(coverLocation, 1, this);

		connect(m->coverLookup, &Lookup::sigCoverFound, this, &CoverButton::setPixmap);
		connect(m->coverLookup, &Lookup::sigFinished, this, &CoverButton::coverLookupFinished);
	}

	else
	{
		m->coverLookup->setCoverLocation(coverLocation);
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

void CoverButton::alternativeCoverFetched(const Location& coverLocation)
{
	m->coverHash.clear();
	m->coverSource = Source::Unknown;

	if(!isSilent())
	{
		if(coverLocation.isValid())
		{
			m->coverChangeNotifier->shout();
		}
	}

	else
	{
		this->setPixmapPath(coverLocation.alternativePath());
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

void CoverButton::languageChanged()
{
	this->setToolTip(tr("Search an alternative cover"));
}
