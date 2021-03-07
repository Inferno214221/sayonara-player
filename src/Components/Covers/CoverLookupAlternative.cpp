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
	Lookup* lookup = nullptr;
	int coverCount;
	bool running;
	bool silent;

	Private(const Cover::Location& cl, int coverCount, bool silent, AlternativeLookup* parent) :
		coverCount(coverCount),
		running(false),
		silent(silent)
	{
		lookup = new Lookup(cl, coverCount, parent);
	}

	~Private()
	{
		lookup->stop();
	}
};

AlternativeLookup::AlternativeLookup(const Cover::Location& cl, int coverCount, bool silent, QObject* parent) :
	LookupBase(cl, parent)
{
	m = Pimpl::make<Private>(cl, coverCount, silent, this);

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

	Cover::Location cl = coverLocation();

	if(!m->silent)
	{
		Cover::writeCoverIntoDatabase(cl, cover);

		if(saveToLibrary)
		{
			Cover::writeCoverToLibrary(cl, cover);
		}
	}

	else if(!cover.save(cl.alternativePath()))
	{
		spLog(Log::Warning, this) << "Cannot save cover to " << cl.alternativePath();
	}

	emit sigCoverChanged(cl);

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
	auto* cfm = Cover::Fetcher::Manager::instance();
	const auto coverFetchers = cfm->coverfetchers();

	QStringList ret;
	for(const auto coverFetcher : coverFetchers)
	{
		const QString identifier = coverFetcher->identifier();
		if(!cfm->isActive(identifier))
		{
			continue;
		}

		bool validIdentifier = false;
		if(mode == AlternativeLookup::SearchMode::Fulltext)
		{
			const QString address = coverFetcher->fulltextSearchAddress("some dummy text");
			validIdentifier = (!address.isEmpty());
		}

		else
		{
			const UrlList searchUrls = coverLocation().searchUrls();

			validIdentifier = Algorithm::contains(searchUrls, [identifier](const Url& url) {
				return (url.identifier().compare(identifier, Qt::CaseInsensitive) == 0);
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
	Location cl = coverLocation();
	const UrlList searchUrls = coverLocation().searchUrls();

	auto it = Algorithm::find(searchUrls, [&identifier](const Url& url) {
		return (identifier == url.identifier());
	});

	if(it != searchUrls.end())
	{
		Url url = *it;
		cl.setSearchUrls({url});
	}

	go(cl);
}

void AlternativeLookup::startTextSearch(const QString& searchTerm)
{
	Location cl = coverLocation();
	cl.setSearchTerm(searchTerm);
	cl.enableFreetextSearch(true);
	go(cl);
}

void AlternativeLookup::startTextSearch(const QString& searchTerm, const QString& coverFetcherIdentifier)
{
	Location cl = coverLocation();
	cl.setSearchTerm(searchTerm, coverFetcherIdentifier);
	cl.enableFreetextSearch(true);
	go(cl);
}

