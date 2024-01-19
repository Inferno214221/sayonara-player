/* CoverLookup.cpp */

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


/*
 * CoverLookup.cpp
 *
 *  Created on: Apr 4, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "CoverLookup.h"
#include "CoverExtractor.h"
#include "CoverFetchThread.h"
#include "CoverLocation.h"
#include "CoverPersistence.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/CoverUtils.h"

#include <QImageWriter>
#include <QStringList>
#include <QThread>
#include <QDir>

#include <atomic>

using Cover::Lookup;
using Cover::WebCoverFetcher;
using Util::Covers::Source;

struct Lookup::Private
{
	QList<QPixmap> pixmaps;
	WebCoverFetcher* coverFetchThread = nullptr;

	int coversRequested;
	Source source {Source::Unknown};
	std::atomic<bool> stopped {false};
	bool ignoreCache {false};

	explicit Private(int coversRequested) :
		coversRequested(coversRequested) {}

	void stopThread()
	{
		if(this->coverFetchThread)
		{
			this->coverFetchThread->stop();
			this->coverFetchThread = nullptr;
		}
	}
};

Lookup::Lookup(const Cover::Location& coverLocation, int coversRequested, QObject* parent) :
	LookupBase(coverLocation, parent)
{
	m = Pimpl::make<Private>(coversRequested);
}

Lookup::~Lookup()
{
	m->stopped = true;
	m->stopThread();
}

void Lookup::ignoreCache()
{
	m->ignoreCache = true;
}

bool Lookup::startNewThread(const Cover::Location& coverLocation)
{
	const auto hasSearchUrls = coverLocation.hasSearchUrls();
	if(m->stopped || !hasSearchUrls)
	{
		return false;
	}

	spLog(Log::Develop, this) << "Start new cover fetch thread for " << coverLocation.identifier();

	auto* thread = new QThread(nullptr);

	m->coverFetchThread = new WebCoverFetcher(nullptr, coverLocation, m->coversRequested);
	m->coverFetchThread->moveToThread(thread);

	connect(m->coverFetchThread, &WebCoverFetcher::sigCoverFound, this, &Lookup::coverFound);
	connect(m->coverFetchThread, &WebCoverFetcher::sigFinished, this, &Lookup::threadFinished);
	connect(m->coverFetchThread, &WebCoverFetcher::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, m->coverFetchThread, &WebCoverFetcher::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();

	return true;
}

void Lookup::start()
{
	m->pixmaps.clear();
	m->stopped = false;
	m->source = Source::Unknown;

	spLog(Log::Develop, this) << "Search cover for id " << coverLocation().identifier();

	const auto coverLocation = LookupBase::coverLocation();
	if(coverLocation.isValid())
	{
		if((m->coversRequested == 1) && (!m->ignoreCache))
		{
			if(!fetchFromDatabase())
			{
				fetchFromExtractor();
			}

			return;
		}

		if(fetchFromWWW())
		{
			return;
		}
	}

	done(false);
}

bool Lookup::fetchFromDatabase()
{
	m->source = Source::Database;

	QPixmap pixmap;

	auto* coverConnector = DB::Connector::instance()->coverConnector();
	const auto hash = LookupBase::coverLocation().hash();
	const auto success = coverConnector->getCover(hash, pixmap);

	return success && addNewCover(pixmap, false);
}

void Lookup::fetchFromExtractor()
{
	m->source = Source::AudioFile;

	auto* extractor = new Cover::Extractor(LookupBase::coverLocation(), nullptr);
	auto* thread = new QThread(nullptr);
	extractor->moveToThread(thread);

	connect(extractor, &Cover::Extractor::sigFinished, this, &Cover::Lookup::extractorFinished);
	connect(extractor, &Cover::Extractor::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, extractor, &Cover::Extractor::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();
}

void Lookup::extractorFinished()
{
	auto* extractor = dynamic_cast<Cover::Extractor*>(sender());

	const auto pixmap = extractor->pixmap();
	m->source = extractor->source();

	extractor->deleteLater();

	const auto coverAdded = addNewCover(pixmap, true);
	if(!coverAdded)
	{
		if(!fetchFromWWW())
		{
			done(false);
		}
	}
}

bool Lookup::fetchFromWWW()
{
	m->source = Source::WWW;

	const auto fetchFromWebAllowed = GetSetting(Set::Cover_FetchFromWWW);
	if(fetchFromWebAllowed)
	{
		const auto coverLocation = LookupBase::coverLocation();
		return startNewThread(coverLocation);
	}

	return false;
}

void Lookup::threadFinished()
{
	sender()->deleteLater();
	m->coverFetchThread = nullptr;
}

bool Lookup::addNewCover(const QPixmap& pixmap, bool save)
{
	if(m->stopped || pixmap.isNull())
	{
		return false;
	}

	emit sigCoverFound(pixmap);
	m->pixmaps.push_back(pixmap);

	if(save && (m->coversRequested == 1))
	{
		if(GetSetting(Set::Cover_SaveToDB))
		{
			Cover::writeCoverIntoDatabase(coverLocation(), pixmap);
		}

		if(GetSetting(Set::Cover_SaveToLibrary) && (m->source == Source::WWW))
		{
			Cover::writeCoverToLibrary(coverLocation(), pixmap);
		}
	}

	if(m->pixmaps.size() == m->coversRequested)
	{
		done(true);
	}

	return true;
}

void Lookup::coverFound(int index)
{
	auto* coverFetchThread = dynamic_cast<WebCoverFetcher*>(sender());
	if(coverFetchThread)
	{
		addNewCover(coverFetchThread->pixmap(index), true);
	}
}

void Lookup::stop()
{
	m->stopThread();
	done(true);
}

void Lookup::done(bool success)
{
	if(!success)
	{
		m->source = Source::Unknown;
		m->pixmaps.clear();
	}

	if(!m->stopped)
	{
		m->stopped = true;
		emit sigFinished(success);
	}
}

QList<QPixmap> Lookup::pixmaps() const
{
	return m->pixmaps;
}

Source Lookup::source() const
{
	return m->source;
}
