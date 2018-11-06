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

#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"

#include <QImage>
#include <QImageWriter>
#include <QStringList>

using Cover::Location;
using Cover::Lookup;
using Cover::FetchThread;

namespace FileUtils=::Util::File;

struct Lookup::Private
{
	Location		cl;
	QList<QPixmap>	pixmaps;
	int				n_covers;

	FetchThread*	cft=nullptr;
	void*			user_data=nullptr;

	bool			thread_running;

	Private(int n_covers) :
		n_covers(n_covers),
		thread_running(false)
	{}
};

Lookup::Lookup(QObject* parent, int n_covers) :
	LookupBase(parent)
{
	m = Pimpl::make<Private>(n_covers);
}

Lookup::~Lookup()
{
	if(m->cft){
		m->cft->stop();
	}
}

bool Lookup::start_new_thread(const Cover::Location& cl )
{
	bool has_search_urls = cl.has_search_urls();
	if(!has_search_urls || !cl.valid()){
		return false;
	}

	m->cl = cl;

	sp_log(Log::Develop, this) << cl.search_urls();
	m->thread_running = true;

	FetchThread* cft = new FetchThread(this, cl, m->n_covers);

	connect(cft, &FetchThread::sig_cover_found, this, &Lookup::cover_found);
	connect(cft, &FetchThread::sig_finished, this, &Lookup::finished);

	cft->start();

	m->cft = cft;

	return true;
}



bool Lookup::fetch_cover(const Cover::Location& cl, bool also_www)
{
	DB::Covers* dbc = DB::Connector::instance()->cover_connector();

	if(m->n_covers == 1)
	{
		{ // first, let's look in database
			QPixmap pm;
			bool success = dbc->get_cover(cl.hash(), pm);
			if(success)
			{
				add_new_cover(pm);
				emit sig_finished(true);
				return true;
			}
		}

		{ // Let's look if the cover can be extracted from a file
			if(cl.has_audio_file_source())
			{
				QPixmap pm;
				if(FileUtils::exists(cl.audio_file_target()))
				{
					pm = QPixmap(cl.audio_file_target());
				}

				else
				{
					pm = Tagging::Utils::extract_cover(cl.audio_file_source());
				}

				bool success = add_new_cover(pm, cl.hash());
				if(success)
				{
					emit sig_finished(true);
					return true;
				}
			}
		}
	}

	{ // Let's look at the preferred path where we look at the local dirs, too
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
				emit sig_finished(true);
				return true;
			}
		}
	}

	{// we have to fetch the cover from the internet
		if(also_www)
		{
			sp_log(Log::Debug, this) << "Start new thread for " << cl.identifer();
			return start_new_thread( cl );
		}
	}

	return false;
}


void Lookup::finished(bool success)
{
	m->cft = nullptr;
	emit sig_finished(success);
}


bool Lookup::add_new_cover(const QPixmap& pm)
{
	if(!pm.isNull())
	{
		m->pixmaps << pm;
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

	if(!cft->more())
	{
		emit sig_finished(true);
	}
}

void Lookup::stop()
{
	if(m->cft)
	{
		m->cft->stop();
		emit sig_finished(true);
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

void* Lookup::take_user_data()
{
	void* data = m->user_data;
	m->user_data = nullptr;
	return data;
}

QList<QPixmap> Lookup::pixmaps() const
{
	return m->pixmaps;
}
