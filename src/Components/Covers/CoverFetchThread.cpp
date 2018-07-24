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

#include <QImage>
#include <QStringList>

const int Timeout = 10000;

using Cover::FetchThread;

struct FetchThread::Private
{
	QList<AsyncWebAccess*> active_connections;

	Cover::Location		cl;
	Fetcher::Base*		acf=nullptr;

	QString				url;
	QString				id;
	QStringList			addresses;
	QStringList			search_urls;
	int					n_covers;
	int					n_covers_found;
	bool				may_run;

	Private(const Location& cl, int n_covers) :
		cl(cl),
		id(Util::random_string(8)),
		search_urls(cl.search_urls()),
		n_covers(n_covers),
		n_covers_found(0),
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
		for(AsyncWebAccess* awa : ::Util::AsConst(m->active_connections))
		{
			awa->stop();
		}

		Util::sleep_ms(50);
	}
}

bool FetchThread::start()
{
	m->may_run = true;

	if(!m->search_urls.isEmpty())
	{
		m->url = m->search_urls.takeFirst();
	}

	else {
		return false;
	}

	Fetcher::Manager* cfm = Fetcher::Manager::instance();
	m->acf = cfm->available_coverfetcher(m->url);

	if(!m->acf){
		return false;
	}

	if( m->acf->can_fetch_cover_directly() )
	{
		m->addresses.clear();
		m->addresses << m->url;

		more();
	}

	else
	{
		AsyncWebAccess* awa = new AsyncWebAccess(this);
		awa->setObjectName(m->acf->keyword());
		awa->set_behavior(AsyncWebAccess::Behavior::AsSayonara);
		connect(awa, &AsyncWebAccess::sig_finished, this, &FetchThread::content_fetched);

		m->active_connections << awa;
		awa->run(m->url, Timeout);
	}

	return true;
}


bool FetchThread::more()
{
	if(m->may_run == false){
		return false;
	}

	// we have all our covers
	if(m->n_covers == m->n_covers_found)
	{
		emit sig_finished(true);
		return true;
	}

	// we have no more addresses and not all our covers
	if(m->addresses.isEmpty())
	{
		bool success = start();
		if(!success) {
			emit sig_finished(false);
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
	awa->run(address, Timeout);
	m->active_connections << awa;

	return true;
}

void FetchThread::stop()
{
	for(AsyncWebAccess* awa : ::Util::AsConst(m->active_connections))
	{
		awa->stop();
	}

	m->may_run = false;
	emit sig_finished(false);
}


void FetchThread::content_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	m->active_connections.removeAll(awa);

	if(awa->objectName() == m->acf->keyword())
	{
		if(awa->status() == AsyncWebAccess::Status::GotData)
		{
			QByteArray website = awa->data();
			m->addresses = m->acf->calc_addresses_from_website(website);
		}
	}

	awa->deleteLater();
	more();
}

void FetchThread::single_image_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	AsyncWebAccess::Status status = awa->status();

	m->active_connections.removeAll(awa);
	awa->deleteLater();

	if(status == AsyncWebAccess::Status::GotData)
	{
		QImage img  = awa->image();

		if(!img.isNull())
		{
			QString target_file = m->cl.cover_path();
			m->n_covers_found++;
			save_and_emit_image(target_file, img);
			emit sig_finished(true);
		}

		sp_log(Log::Info, this) << "Found cover in " << m->acf->keyword() << " for " << m->cl.identifer();
	}

	else {
		sp_log(Log::Warning, this) << "Could not fetch cover from " << m->acf->keyword();
		if(!more()){
			emit sig_finished(false);
		}
	}
}


void
FetchThread::multi_image_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	m->active_connections.removeAll(awa);

	if(awa->status() == AsyncWebAccess::Status::GotData){

		QImage img  = awa->image();

		if(!img.isNull())
		{
			QString filename, dir, cover_path;
			QString target_file = m->cl.cover_path();
			Util::File::split_filename(target_file, dir, filename);

			cover_path = dir + "/" + QString::number(m->n_covers_found) + "_" + filename;
			save_and_emit_image(cover_path, img);

			m->n_covers_found++;
		}
	}

	else {
		sp_log(Log::Warning, this) << "Could not fetch multi cover " << m->acf->keyword();
	}

	awa->deleteLater();
}


void
FetchThread::save_and_emit_image(const QString& filepath, const QImage& img)
{
	QString filename = filepath;
	QString ext = Util::File::calc_file_extension(filepath);
	if(ext.compare("gif", Qt::CaseInsensitive) == 0){
		filename = filename.left(filename.size() - 3);
		filename += "png";
	}

	filename = Util::File::get_absolute_filename(filename);

	bool success = img.save(filename);
	if(!success){
		sp_log(Log::Warning, this) << "Cannot save image to " << filename;
	}

	else{
		emit sig_cover_found(filename);
	}
}
