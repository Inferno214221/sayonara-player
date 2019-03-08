/* CoverFetchThread.cpp */

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
 * CoverFetchThread.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: Lucio Carreras
 */

#include "CoverFetchThread.h"
#include "CoverLocation.h"
#include "CoverFetchManager.h"
#include "CoverFetcherInterface.h"

#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QPixmap>
#include <QImage>
#include <QStringList>

const int Timeout = 10000;

using namespace Cover;

struct FetchThread::Private
{
	QList<AsyncWebAccess*>	active_connections;
	QList<QPixmap>			pixmaps;

	Cover::Location		cl;
	Fetcher::Base*		acf=nullptr;

	QString				id;
	QStringList			addresses;
	QStringList			search_urls;
	int					n_covers;
	bool				finished;
	bool				may_run;

	Private(const Location& cl, int n_covers) :
		cl(cl),
		id(Util::random_string(8)),
		search_urls(cl.search_urls()),
		n_covers(n_covers),
		finished(false),
		may_run(true)
	{
		sp_log(Log::Develop, this) << "Search urls for " << cl.identifer() << ": " << cl.search_urls();
	}
};


FetchThread::FetchThread(QObject* parent, const Location& cl, const int n_covers) :
	QObject(parent)
{
	m = Pimpl::make<Private>(cl, n_covers);
}

FetchThread::~FetchThread()
{
	while(!m->active_connections.isEmpty())
	{
		AsyncWebAccess* awa = m->active_connections.takeLast();

		awa->stop();
		awa->deleteLater();

		Util::sleep_ms(50);
	}
}

bool FetchThread::start()
{
	m->may_run = true;

	if(m->search_urls.isEmpty()){
		return false;
	}

	QString url = m->search_urls.takeFirst();

	Fetcher::Manager* cfm = Fetcher::Manager::instance();
	m->acf = cfm->coverfetcher(url);
	if(!m->acf){
		return false;
	}

	if( m->acf->can_fetch_cover_directly() )
	{
		m->addresses.clear();
		m->addresses << url;

		fetch_next_cover();
	}

	else
	{
		AsyncWebAccess* awa = new AsyncWebAccess(this);
		awa->setObjectName(m->acf->keyword());
		awa->set_behavior(AsyncWebAccess::Behavior::AsSayonara);
		connect(awa, &AsyncWebAccess::sig_finished, this, &FetchThread::content_fetched);

		m->active_connections << awa;
		awa->run(url, Timeout);
	}

	return true;
}


void FetchThread::content_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	m->active_connections.removeAll(awa);

	if(!m->may_run){
		awa->deleteLater();
		return;
	}

	if(awa->objectName() == m->acf->keyword())
	{
		if(awa->status() == AsyncWebAccess::Status::GotData)
		{
			QByteArray website = awa->data();
			m->addresses = m->acf->calc_addresses_from_website(website);
		}
	}

	awa->deleteLater();

	if(!fetch_next_cover())
	{
		sp_log(Log::Warning, this) << "No more adresses available";
	}
}


bool FetchThread::fetch_next_cover()
{
	if(!m->may_run){
		return false;
	}

	// we have all our covers
	if(m->n_covers == m->pixmaps.size())
	{
		emit_finished(true);
		return true;
	}

	// we have no more addresses and not all our covers
	if(m->addresses.isEmpty())
	{
		bool success = start();
		if(!success) {
			emit_finished(false);
		}

		return success;
	}

	QString address = m->addresses.takeFirst();
	AsyncWebAccess* awa = new AsyncWebAccess(this);
	awa->set_behavior(AsyncWebAccess::Behavior::AsBrowser);

	if(m->n_covers == 1) {
		connect(awa, &AsyncWebAccess::sig_finished, this, &FetchThread::single_image_fetched);
	}

	else {
		connect(awa, &AsyncWebAccess::sig_finished, this, &FetchThread::multi_image_fetched);
	}

	sp_log(Log::Develop, this) << "Fetch cover from " << address;

	m->active_connections << awa;
	awa->run(address, Timeout);

	return true;
}

void FetchThread::stop()
{
	m->may_run = false;

	emit_finished(false);
}

QPixmap FetchThread::pixmap(int idx) const
{
	if(idx >= 0 && idx < m->pixmaps.size())
	{
		return m->pixmaps[idx];
	}

	return QPixmap();
}


void FetchThread::emit_finished(bool success)
{
	if(!m->finished)
	{
		emit sig_finished(success);
		m->finished = true;
	}
}

void FetchThread::single_image_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	AsyncWebAccess::Status status = awa->status();
	QImage image = awa->image();

	m->active_connections.removeAll(awa);
	awa->deleteLater();

	if(!m->may_run){
		return;
	}

	if(status == AsyncWebAccess::Status::GotData)
	{
		QPixmap pm = QPixmap::fromImage(image);

		if(!pm.isNull())
		{
			sp_log(Log::Debug, this) << "Found cover in " << m->acf->keyword() << " for " << m->cl.identifer();

			m->pixmaps << pm;

			emit sig_cover_found(m->pixmaps.count() - 1);
			emit_finished(true);

			return;
		}
	}

	if(!fetch_next_cover())
	{
		sp_log(Log::Warning, this) << "Could not fetch cover from " << m->acf->keyword();
	}
}


void FetchThread::multi_image_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	AsyncWebAccess::Status status = awa->status();
	QImage img = awa->image();

	m->active_connections.removeAll(awa);
	awa->deleteLater();

	if(!m->may_run){
		return;
	}

	if(status == AsyncWebAccess::Status::GotData)
	{
		QPixmap pm = QPixmap::fromImage(img);
		if(!pm.isNull())
		{
			m->pixmaps << pm;
			emit sig_cover_found(m->pixmaps.count() - 1);
		}
	}

	else
	{
		sp_log(Log::Warning, this) << "Could not fetch multi cover " << m->acf->keyword();
	}

	fetch_next_cover();
}
