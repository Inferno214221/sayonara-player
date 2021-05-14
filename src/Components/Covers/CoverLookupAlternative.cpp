/* CoverLookupAlternative.cpp */

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

#include "CoverLookup.h"
#include "CoverLookupAlternative.h"
#include "CoverLocation.h"
#include "CoverFetchManager.h"
#include "CoverPersistence.h"
#include "Fetcher/CoverFetcherUrl.h"
#include "Fetcher/CoverFetcher.h"

#include "Database/Connector.h"

#include "Utils/Algorithm.h"
#include "Utils/CoverUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>

namespace Algorithm = Util::Algorithm;
using Cover::AlternativeLookup;
using Cover::Location;
using Cover::Lookup;
using Cover::LookupBase;
using Cover::Fetcher::Manager;
using Cover::Fetcher::Url;
using UrlList = QList<Url>;

struct AlternativeLookup::Private
{
	Lookup* lookup;
	int coverCount;
	bool running;
	bool silent;

	Private(const Cover::Location& coverLocation, int coverCount, bool silent, AlternativeLookup* parent) :
		lookup {new Lookup(coverLocation, coverCount, parent)},
		coverCount(coverCount),
		running(false),
		silent(silent) {}

	~Private()
	{
		lookup->stop();
	}
};

AlternativeLookup::AlternativeLookup(const Cover::Location& coverLocation, int coverCount, bool silent,
                                     QObject* parent) :
	LookupBase(coverLocation, parent)
{
	m = Pimpl::make<Private>(coverLocation, coverCount, silent, this);

	connect(m->lookup, &Lookup::sigStarted, this, &AlternativeLookup::started);
	connect(m->lookup, &Lookup::sigCoverFound, this, &AlternativeLookup::coverFound);
	connect(m->lookup, &Lookup::sigFinished, this, &AlternativeLookup::finished);

	ListenSettingNoCall(Set::Cover_Server, AlternativeLookup::coverfetchersChanged);
}

AlternativeLookup::~AlternativeLookup() = default;

void AlternativeLookup::stop()
{
	m->lookup->stop();
	m->running = false;
}

void AlternativeLookup::reset()
{
	stop();
	Util::Covers::deleteTemporaryCovers();
}

bool AlternativeLookup::save(const QPixmap& cover, bool saveToLibrary)
{
	if(cover.isNull())
	{
		spLog(Log::Warning, this) << "Cannot save invalid cover";
		return false;
	}

	const auto coverLocation = LookupBase::coverLocation();
	if(!m->silent)
	{
		Cover::writeCoverIntoDatabase(coverLocation, cover);
		if(saveToLibrary)
		{
			Cover::writeCoverToLibrary(coverLocation, cover);
		}
	}

	else if(!cover.save(coverLocation.alternativePath()))
	{
		spLog(Log::Warning, this) << "Cannot save cover to " << coverLocation.alternativePath();
	}

	emit sigCoverChanged(coverLocation);

	return true;
}

bool AlternativeLookup::isRunning() const
{
	return m->running;
}

bool AlternativeLookup::isSilent() const
{
	return m->silent;
}

QStringList AlternativeLookup::activeCoverfetchers(AlternativeLookup::SearchMode mode) const
{
	auto* coverFetchManager = Cover::Fetcher::Manager::instance();
	const auto coverFetchers = coverFetchManager->coverfetchers();

	QStringList ret;
	for(const auto& coverFetcher : coverFetchers)
	{
		const auto identifier = coverFetcher->identifier();
		if(!coverFetchManager->isActive(identifier))
		{
			continue;
		}

		bool validIdentifier;
		if(mode == AlternativeLookup::SearchMode::Fulltext)
		{
			const auto address = coverFetcher->fulltextSearchAddress("some dummy text");
			validIdentifier = (!address.isEmpty());
		}

		else
		{
			const auto searchUrls = coverLocation().searchUrls();

			validIdentifier = Algorithm::contains(searchUrls, [&](const auto& url) {
				return (url.identifier().toLower() == identifier.toLower());
			});
		}

		if(validIdentifier)
		{
			ret << identifier;
		}
	}

	return ret;
}

void AlternativeLookup::started()
{
	m->running = true;
	emit sigStarted();
}

void AlternativeLookup::finished(bool success)
{
	m->running = false;
	emit sigFinished(success);
}

void AlternativeLookup::coverFound(const QPixmap& pm)
{
	emit sigCoverFound(pm);
}

void AlternativeLookup::coverfetchersChanged()
{
	emit sigCoverfetchersChanged();
}

void AlternativeLookup::go(const Cover::Location& cl)
{
	m->lookup->setCoverLocation(cl);
	m->lookup->start();

	emit sigStarted();
}

void AlternativeLookup::start()
{
	go(coverLocation());
}

void AlternativeLookup::start(const QString& identifier)
{
	auto coverLocation = LookupBase::coverLocation();
	const auto searchUrls = coverLocation.searchUrls();

	const auto it = Algorithm::find(searchUrls, [&identifier](const auto& url) {
		return (identifier == url.identifier());
	});

	if(it != searchUrls.end())
	{
		coverLocation.setSearchUrls({*it});
	}

	go(coverLocation);
}

void AlternativeLookup::startTextSearch(const QString& searchTerm)
{
	auto coverLocation = LookupBase::coverLocation();
	coverLocation.setSearchTerm(searchTerm);
	coverLocation.enableFreetextSearch(true);

	go(coverLocation);
}

void AlternativeLookup::startTextSearch(const QString& searchTerm, const QString& coverFetcherIdentifier)
{
	auto coverLocation = LookupBase::coverLocation();
	coverLocation.setSearchTerm(searchTerm, coverFetcherIdentifier);
	coverLocation.enableFreetextSearch(true);

	go(coverLocation);
}
