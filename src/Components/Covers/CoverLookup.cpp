/* CoverLookup.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
#include <QDir>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Cover::FetchThread;

namespace FileUtils=::Util::File;

enum CoverSource : quint8
{
	Database=0,
	AudioFile,
	Filesystem,
	WWW
};

struct Lookup::Private
{
	QList<QPixmap>	pixmaps;
	FetchThread*	cft=nullptr;
	void*			user_data=nullptr;

	int				n_covers;
	CoverSource		source;
	bool			thread_running;
	bool			finished;
	bool			stopped;

	Private(int n_covers) :
		n_covers(n_covers),
		thread_running(false),
		finished(false),
		stopped(false)
	{}
};

Lookup::Lookup(const Location& cl, int n_covers, QObject* parent) :
	LookupBase(cl, parent)
{
	m = Pimpl::make<Private>(n_covers);
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

	m->source = CoverSource::WWW;
	thread->start();
	return true;
}

void Lookup::start()
{
	m->stopped = false;
	m->finished = false;
	bool success = false;

	if(!cover_location().valid()){
		emit_finished(false);
		return;
	}

	if(m->n_covers == 1)
	{
		success = fetch_from_database();
		if(success){
			return;
		}

		success = fetch_from_audio_source();
		if(success){
			return;
		}
	}

	success = fetch_from_file_system();
	if(success){
		return;
	}

	success = fetch_from_www();
	if(success){
		return;
	}

	emit_finished(false);
}


bool Lookup::fetch_from_database()
{
	m->source = CoverSource::Database;

	Cover::Location cl = cover_location();
	QString hash = cl.hash();

	DB::Covers* dbc = DB::Connector::instance()->cover_connector();

	QPixmap pm;
	bool success = dbc->get_cover(hash, pm);
	if(success && !pm.isNull())
	{
		add_new_cover(pm);
		emit_finished(true);
		return true;
	}

	return false;
}

bool Lookup::fetch_from_audio_source()
{
	m->source = CoverSource::AudioFile;

	Cover::Location cl = cover_location();

	if(!cl.has_audio_file_source()){
		return false;
	}

	QString audio_file_target = cl.audio_file_target();
	if(FileUtils::exists(audio_file_target))
	{
		QPixmap pm = QPixmap(cl.audio_file_target());
		bool success = add_new_cover(pm, cl.hash());
		if(success)
		{
			emit_finished(true);
			return true;
		}
	}

	Cover::Extractor* extractor = new Cover::Extractor(cl.audio_file_source(), nullptr);
	QThread* thread = new QThread(nullptr);
	extractor->moveToThread(thread);

	connect(extractor, &Cover::Extractor::sig_finished, this, &Cover::Lookup::extractor_finished);
	connect(extractor, &Cover::Extractor::destroyed, thread, &QThread::quit);

	connect(thread, &QThread::started, extractor, &Cover::Extractor::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();

	return true;
}

bool Lookup::fetch_from_file_system()
{
	m->source = CoverSource::Filesystem;

	Cover::Location cl = cover_location();
	QString local_path = cl.local_path();

	// Look, if cover exists in .Sayonara/covers
	if(FileUtils::exists(local_path) && m->n_covers == 1)
	{
		QPixmap pm(local_path);
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
	Cover::Location cl = cover_location();

	bool fetch_from_www_allowed = GetSetting(Set::Cover_FetchFromWWW);
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
		sp_log(Log::Debug, this) << " finished: success";
		this->add_new_cover(pm, cover_location().hash());
		emit_finished(true);
	}

	else
	{
		sp_log(Log::Debug, this) << " finished: PM is null";
		bool success = fetch_from_file_system();
		if(success){
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
	if(m->stopped){
		return false;
	}

	if(!pm.isNull())
	{
		m->pixmaps.push_back(pm);
		emit sig_cover_found(pm);
	}

	return (!pm.isNull());
}


bool Lookup::add_new_cover(const QPixmap& pm, const QString& hash)
{
	bool success = add_new_cover(pm);
	if(!success){
		return false;
	}

	if(GetSetting(Set::Cover_SaveToDB))
	{
		DB::Covers* dbc = DB::Connector::instance()->cover_connector();
		if(!dbc->exists(hash))
		{
			dbc->set_cover(hash, pm);
		}
	}

	if( GetSetting(Set::Cover_SaveToLibrary) &&
		(m->source == CoverSource::WWW))
	{
		Cover::Location cl = cover_location();
		QString local_path_dir = cl.local_path_dir();
		if(!local_path_dir.isEmpty())
		{
			QString cover_template = GetSetting(Set::Cover_TemplatePath);
			cover_template.replace("<h>", cl.hash());
			QString target_file = QDir(local_path_dir).absoluteFilePath(cover_template);
			pm.save(target_file);
		}
	}

	if(GetSetting(Set::Cover_SaveToSayonaraDir) &&
	  (m->source == CoverSource::WWW))
	{
		QString filepath = Cover::Utils::cover_directory(hash + ".jpg");
		if(!Util::File::exists(filepath))
		{
			pm.save(filepath);
		}
	}

	return true;
}


void Lookup::cover_found(int idx)
{
	FetchThread* cft = static_cast<FetchThread*>(sender());
	if(!cft){
		return;
	}

	QPixmap pm = cft->pixmap(idx);
	add_new_cover(pm, cover_location().hash());
}

void Lookup::stop()
{
	m->stopped = true;
	if(m->cft)
	{
		m->cft->stop();
		emit_finished(true);
	}
}

void Lookup::emit_finished(bool success)
{
	if(!m->finished)
	{
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
	m->pixmap = QPixmap();

	if(FileUtils::exists(m->filepath)){
		m->pixmap = Tagging::Covers::extract_cover(m->filepath);
	}

	emit sig_finished();
}
