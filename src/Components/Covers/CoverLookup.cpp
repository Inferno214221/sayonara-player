/* CoverLookup.cpp */

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


/*
 * CoverLookup.cpp
 *
 *  Created on: Apr 4, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "CoverLookup.h"
#include "CoverFetchThread.h"
#include "CoverLocation.h"
#include "CoverUtils.h"
#include "CoverExtractor.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Album.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QImage>
#include <QImageWriter>
#include <QStringList>
#include <QThread>
#include <QDir>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Cover::FetchThread;
using Cover::Source;

namespace FileUtils=::Util::File;

struct Lookup::Private
{
	QList<QPixmap>	pixmaps;
	FetchThread*	cft=nullptr;
	void*			userData=nullptr;

	int				coverCount;
	Source			source;
	bool			isThreadRunning;
	bool			finished;
	bool			stopped;

	Private(int n_covers) :
		coverCount(n_covers),
		isThreadRunning(false),
		finished(false),
		stopped(false)
	{}
};

Lookup::Lookup(const Location& cl, int coverCount, QObject* parent) :
	LookupBase(cl, parent)
{
	m = Pimpl::make<Private>(coverCount);
}

Lookup::~Lookup()
{
	m->stopped = true;

	if(m->cft)
	{
		m->cft->stop();
		m->cft->deleteLater();
	}
}

bool Lookup::startNewThread(const Cover::Location& cl)
{
	bool hasSearchUrls = cl.hasSearchUrls();
	if(!hasSearchUrls || !cl.isValid()){
		return false;
	}

	spLog(Log::Develop, this) << "Start new cover fetch thread for " << cl.identifer();

	m->isThreadRunning = true;

	QThread* thread = new QThread(nullptr);

	m->cft = new FetchThread(nullptr, cl, m->coverCount);
	m->cft->moveToThread(thread);

	connect(m->cft, &FetchThread::sigCoverFound, this, &Lookup::coverFound);
	connect(m->cft, &FetchThread::sigFinished, this, &Lookup::threadFinished);
	connect(m->cft, &FetchThread::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, m->cft, &FetchThread::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	m->source = Source::WWW;
	thread->start();
	return true;
}

void Lookup::start()
{
	m->pixmaps.clear();
	m->isThreadRunning = false;
	m->stopped = false;
	m->finished = false;

	QString id = coverLocation().identifer();
	spLog(Log::Develop, this) << "Search cover for id " << id;

	if(!coverLocation().isValid()){
		emitFinished(false);
		return;
	}

	if(m->coverCount == 1)
	{
		if(fetchFromDatabase()){
			return;
		}

		if(fetchFromExtractor()){
			return;
		}
	}

	if(fetchFromWWW()){
		return;
	}

	emitFinished(false);
}


bool Lookup::fetchFromDatabase()
{
	m->source = Source::Database;

	Cover::Location cl = coverLocation();
	QString hash = cl.hash();

	DB::Covers* dbc = DB::Connector::instance()->coverConnector();

	QPixmap pm;
	bool success = dbc->getCover(hash, pm);
	if(success && !pm.isNull())
	{
		addNewCover(pm, false);
		return true;
	}

	return false;
}

bool Lookup::fetchFromExtractor()
{
	m->source = Source::AudioFile;

	Cover::Location cl = coverLocation();
	return startExtractor(cl);
}


bool Lookup::fetchFromWWW()
{
	Cover::Location cl = coverLocation();

	bool fetchFromWwwAllowed = GetSetting(Set::Cover_FetchFromWWW);
	if(fetchFromWwwAllowed)
	{
		spLog(Log::Debug, this) << "Start new thread for " << cl.identifer();
		return startNewThread( cl );
	}

	return false;
}

void Lookup::threadFinished(bool success)
{
	m->cft = nullptr;
	sender()->deleteLater();

	if(!success){
		emitFinished(false);
	}
}


bool Lookup::startExtractor(const Location& cl)
{
	auto* extractor = new Cover::Extractor(cl, nullptr);
	auto* thread = new QThread(nullptr);
	extractor->moveToThread(thread);

	connect(extractor, &Cover::Extractor::sigFinished, this, &Cover::Lookup::extractorFinished);
	connect(extractor, &Cover::Extractor::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, extractor, &Cover::Extractor::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();

	return true;
}


void Lookup::extractorFinished()
{
	auto* extractor = static_cast<Cover::Extractor*>(sender());
	QPixmap pm = extractor->pixmap();
	m->source = extractor->source();

	extractor->deleteLater();

	spLog(Log::Develop, this) << "Extractor finished. Pixmap valid " << !pm.isNull();

	if(!pm.isNull())
	{
		addNewCover(pm, true);
	}

	else
	{
		bool success = fetchFromWWW();
		if(!success){
			emitFinished(false);
		}
	}
}


bool Lookup::addNewCover(const QPixmap& pm, bool save)
{
	if(m->stopped || pm.isNull()){
		return false;
	}

	emit sigCoverFound(pm);
	m->pixmaps.push_back(pm);

	if(!save || m->coverCount > 1)
	{
		if(m->pixmaps.size() == m->coverCount){
			emitFinished(true);
		}

		return true;
	}

	if(GetSetting(Set::Cover_SaveToDB))
	{
		Cover::Utils::writeCoverIntoDatabase(coverLocation(), pm);
	}

	if( GetSetting(Set::Cover_SaveToLibrary) &&
		(m->source == Source::WWW))
	{
		Cover::Utils::writeCoverToLibrary(coverLocation(), pm);
	}

	if(GetSetting(Set::Cover_SaveToSayonaraDir) &&
	  (m->source == Source::WWW))
	{
		Cover::Utils::writeCoverToSayonaraDirectory(coverLocation(), pm);
	}

	if(m->pixmaps.size() == m->coverCount){
		emitFinished(true);
	}

	return true;
}


void Lookup::coverFound(int idx)
{
	FetchThread* cft = static_cast<FetchThread*>(sender());
	if(!cft){
		return;
	}

	QPixmap pm = cft->pixmap(idx);
	addNewCover(pm, true);
}

void Lookup::stop()
{
	if(m->cft)
	{
		m->cft->stop();
		m->cft = nullptr;
	}

	m->stopped = true;
	emitFinished(true);
}

void Lookup::emitFinished(bool success)
{
	if(!m->finished)
	{
		m->finished = true;
		emit sigFinished(success);
	}
}

bool Lookup::isThreadRunning() const
{
	return (m->cft != nullptr);
}

void Lookup::setUserData(void* data)
{
	m->userData = data;
}

void* Lookup::userData()
{
	void* data = m->userData;
	return data;
}

QList<QPixmap> Lookup::pixmaps() const
{
	return m->pixmaps;
}

Source Lookup::source() const
{
	return m->source;
}

