/* CoverLookup.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "CoverLookup.h"
#include "CoverFetchThread.h"
#include "CoverLocation.h"
#include "CoverUtils.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QImage>
#include <QImageWriter>
#include <QStringList>
#include <QThread>
#include <mutex>

static std::mutex mtx;
using Cover::Location;
using Cover::Lookup;
using Cover::FetchThread;

namespace FileUtils=::Util::File;

struct Lookup::Private
{
	Location		cl;
	QList<QPixmap>	pixmaps;
	FetchThread*	cft=nullptr;
	void*			user_data=nullptr;

	int				n_covers;
	bool			thread_running;
	bool			finished;

	Private(int n_covers) :
		n_covers(n_covers),
		thread_running(false),
		finished(false)
	{}
};

Lookup::Lookup(QObject* parent, int n_covers) :
	LookupBase(parent)
{
	m = Pimpl::make<Private>(n_covers);
}

Lookup::Lookup(QObject* parent, int n_covers, const Cover::Location& cl) :
	LookupBase(parent)
{
	m = Pimpl::make<Private>(n_covers);
	m->cl = cl;
}

Lookup::~Lookup()
{
	m->pixmaps.clear();
	if(m->cft)
	{
		m->cft->stop();
		m->cft->deleteLater();
	}
}

bool Lookup::start_new_thread(const Cover::Location& cl )
{
	bool has_search_urls = cl.has_search_urls();
	if(!has_search_urls || !cl.valid()){
		return false;
	}

	sp_log(Log::Develop, this) << "Start new cover fetch thread for " << cl.identifer();

	m->cl = cl;

	sp_log(Log::Develop, this) << cl.search_urls();
	m->thread_running = true;

	QThread* thread = new QThread(nullptr);

	m->cft = new FetchThread(nullptr, cl, m->n_covers);
	m->cft->moveToThread(thread);

	connect(m->cft, &FetchThread::sig_cover_found, this, &Lookup::cover_found);
	connect(m->cft, &FetchThread::sig_finished, this, &Lookup::thread_finished);
	connect(m->cft, &FetchThread::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, m->cft, &FetchThread::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();
	return true;
}

void Lookup::start()
{
	bool success = fetch_cover(m->cl, true);
	if(!success)
	{
		emit_finished(false);
	}
}

bool Lookup::fetch_cover(const Cover::Location& cl, bool also_www)
{
	m->finished = false;

	bool success = false;

	if(m->n_covers == 1)
	{
		success = fetch_from_database();
		if(success){
			return true;
		}

		if(cl.has_audio_file_source())
		{
			success = fetch_from_audio_source();
			if(success){
				return true;
			}
		}
	}

	success = fetch_from_file_system();
	if(success){
		return true;
	}

	if(also_www)
	{
		success = fetch_from_www();
		if(success){
			return true;
		}
	}

	emit_finished(false);
	return false;
}


bool Lookup::fetch_from_database()
{
	DB::Covers* dbc = DB::Connector::instance()->cover_connector();

	QPixmap pm;
	QString hash = m->cl.hash();
	bool success = dbc->get_cover(hash, pm);
	if(success)
	{
		add_new_cover(pm);
		emit_finished(true);
	}

	return success;
}

bool Lookup::fetch_from_audio_source()
{
	Cover::Location cl = m->cl;
	QPixmap pm;
	if(FileUtils::exists(cl.audio_file_target()))
	{
		pm = QPixmap(cl.audio_file_target());
		bool success = add_new_cover(pm, cl.hash());
		if(success)
		{
			emit_finished(true);
			return true;
		}
	}

	Cover::Extractor* extractor = new Cover::Extractor(cl.audio_file_source(), this);
	QThread* thread = new QThread();

	connect(extractor, &Cover::Extractor::sig_finished, this, &Cover::Lookup::extractor_finished);
	connect(extractor, &Cover::Extractor::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, extractor, &Cover::Extractor::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();

	return true;
}

bool Lookup::fetch_from_file_system()
{
	Cover::Location cl = m->cl;

	QString cover_path = cl.preferred_path();
	if(Location::is_invalid(cover_path))
	{
		// cover path always return the path in .Sayonara/cover
		// No matter if it exists or not
		cover_path = cl.cover_path();
	}

	// Look, if cover exists in .Sayonara/covers
	if(FileUtils::exists(cover_path) && m->n_covers == 1)
	{
		QPixmap pm(cover_path);
		bool success = add_new_cover(pm, cl.hash());
		if(success)
		{
			emit_finished(true);
			return true;
		}
	}

	return false;
}

bool Lookup::fetch_from_www()
{
	Cover::Location cl = m->cl;

	bool fetch_from_www_allowed = Settings::instance()->get<Set::Cover_FetchFromWWW>();
	if(fetch_from_www_allowed)
	{
		sp_log(Log::Debug, this) << "Start new thread for " << cl.identifer();
		return start_new_thread( cl );
	}

	return false;
}



void Lookup::thread_finished(bool success)
{
	emit_finished(success);

	m->cft = nullptr;
	sender()->deleteLater();
}

void Lookup::extractor_finished()
{
	Cover::Extractor* extractor = static_cast<Cover::Extractor*>(sender());
	QPixmap pm = extractor->pixmap();

	extractor->deleteLater();

	if(!pm.isNull())
	{
		this->add_new_cover(pm, m->cl.hash());
		emit_finished(true);
	}

	else
	{
		bool success = fetch_from_file_system();
		if(!success){
			return;
		}

		success = fetch_from_www();
		if(!success){
			emit_finished(false);
		}
	}
}


bool Lookup::add_new_cover(const QPixmap& pm)
{
	if(!pm.isNull())
	{
		LOCK_GUARD(mtx)
		{
			m->pixmaps.push_back(pm);
		}

		emit sig_cover_found(pm);
	}

	return (!pm.isNull());
}


bool Lookup::add_new_cover(const QPixmap& pm, const QString& hash)
{
	bool success = add_new_cover(pm);
	if(success)
	{
		DB::Covers* dbc = DB::Connector::instance()->cover_connector();
		if(!dbc->exists(hash))
		{
			dbc->set_cover(hash, pm);
		}
	}

	return success;
}


void Lookup::cover_found(int idx)
{
	FetchThread* cft = static_cast<FetchThread*>(sender());
	if(!cft){
		return;
	}

	QPixmap pm = cft->pixmap(idx);
	add_new_cover(pm);
	if(m->n_covers == 1)
	{
		pm.save(Cover::Utils::cover_directory(m->cl.hash() + ".jpg"));
	}
}

void Lookup::stop()
{
	if(m->cft)
	{
		m->cft->stop();
		emit_finished(true);
	}
}

void Lookup::emit_finished(bool success)
{
	if(!m->finished){
		m->finished = true;
		emit sig_finished(success);
	}
}

bool Lookup::is_thread_running() const
{
	return (m->cft != nullptr);
}

void Lookup::set_user_data(void* data)
{
	m->user_data = data;
}

void* Lookup::user_data()
{
	void* data = m->user_data;
	return data;
}

QList<QPixmap> Lookup::pixmaps() const
{
	return m->pixmaps;
}

QList<QPixmap> Lookup::take_pixmaps()
{
	QList<QPixmap> ret;
	LOCK_GUARD(mtx)
	{
		ret = m->pixmaps;
		m->pixmaps.clear();
	}

	return ret;
}


struct Cover::Extractor::Private
{
	QPixmap pixmap;
	QString filepath;

	Private(const QString& filepath) : filepath(filepath) {}
};

Cover::Extractor::Extractor(const QString& filepath, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(filepath);
}

Cover::Extractor::~Extractor() {}

QPixmap Cover::Extractor::pixmap()
{
	return m->pixmap;
}

void Cover::Extractor::start()
{
	m->pixmap = Tagging::Covers::extract_cover(m->filepath);
	emit sig_finished();
}
