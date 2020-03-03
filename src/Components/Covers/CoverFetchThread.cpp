/* CoverFetchThread.cpp */

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
 * CoverFetchThread.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "CoverFetchThread.h"
#include "CoverLocation.h"
#include "CoverFetchManager.h"
#include "Fetcher/CoverFetcher.h"
#include "Fetcher/CoverFetcherUrl.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/AsyncWebAccess.h"

#include <QPixmap>
#include <QImage>
#include <QStringList>

static const int Timeout = 5000;

using namespace Cover;
using Fetcher::Url;
using UrlList=QList<Url>;

struct FetchThread::Private
{
	QList<AsyncWebAccess*>	activeConnections;
	QList<QPixmap>			pixmaps;

	Fetcher::Base*		acf=nullptr;

	QString				id;
	QStringList			addresses;
	UrlList				searchUrls;
	QStringList			foundUrls;
	int					coverCount;
	bool				finished;
	bool				mayRun;

	Private(const Location& cl, int n_covers) :
		id(cl.identifer() + Util::randomString(8)),
		coverCount(n_covers),
		finished(false),
		mayRun(true)
	{
		auto* cfm = Cover::Fetcher::Manager::instance();

		const UrlList urls = cl.searchUrls();
		for(const Url& url : urls)
		{
			if(cfm->isActive(url.identifier()))
			{
				searchUrls << url;
			}
		}
	}
};


FetchThread::FetchThread(QObject* parent, const Location& cl, const int n_covers) :
	QObject(parent)
{
	m = Pimpl::make<Private>(cl, n_covers);
}

FetchThread::~FetchThread()
{
	while(!m->activeConnections.isEmpty())
	{
		AsyncWebAccess* awa = m->activeConnections.takeLast();

		awa->stop();
		awa->deleteLater();

		Util::sleepMs(50);
	}
}

bool FetchThread::start()
{
	m->mayRun = true;

	if(m->searchUrls.isEmpty()){
		return false;
	}

	const Url url = m->searchUrls.takeFirst();

	auto* cfm = Fetcher::Manager::instance();
	m->acf = cfm->coverfetcher(url);
	if(!m->acf) {
		return false;
	}

	spLog(Log::Debug, this) << "Taking Cover::Fetcher " << m->acf->identifier() << " for " << url.url();

	if( m->acf->canFetchCoverDirectly() )
	{
		m->addresses.clear();
		m->addresses << url.url();

		fetchNextCover();
	}

	else
	{
		auto* awa = new AsyncWebAccess(this);
		awa->setObjectName(m->acf->identifier());
		awa->setBehavior(AsyncWebAccess::Behavior::AsSayonara);
		connect(awa, &AsyncWebAccess::sigFinished, this, &FetchThread::contentFetched);

		m->activeConnections << awa;
		awa->run(url.url(), Timeout);
	}

	return true;
}


void FetchThread::contentFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	m->activeConnections.removeAll(awa);

	if(!m->mayRun){
		awa->deleteLater();
		return;
	}

	if(awa->objectName() == m->acf->identifier())
	{
		if(awa->status() == AsyncWebAccess::Status::GotData)
		{
			const QByteArray website = awa->data();
			m->addresses = m->acf->parseAddresses(website);
		}
	}

	awa->deleteLater();

	if(!fetchNextCover())
	{
		spLog(Log::Warning, this) << "No more adresses available";
	}
}


bool FetchThread::fetchNextCover()
{
	if(!m->mayRun){
		return false;
	}

	// we have all our covers
	if(m->coverCount == m->pixmaps.size())
	{
		emitFinished(true);
		return true;
	}

	// we have no more addresses and not all our covers
	if(m->addresses.isEmpty())
	{
		bool success = start();
		if(!success) {
			emitFinished(false);
		}

		return success;
	}

	const QString address = m->addresses.takeFirst();
	auto* awa = new AsyncWebAccess(this);
	awa->setBehavior(AsyncWebAccess::Behavior::AsBrowser);

	if(m->coverCount == 1) {
		connect(awa, &AsyncWebAccess::sigFinished, this, &FetchThread::singleImageFetched);
	}

	else {
		connect(awa, &AsyncWebAccess::sigFinished, this, &FetchThread::multiImageFetched);
	}

	spLog(Log::Develop, this) << "Fetch cover from " << address;

	m->activeConnections << awa;
	awa->run(address, Timeout);

	return true;
}

void FetchThread::stop()
{
	m->mayRun = false;
	emitFinished(false);
}

void FetchThread::emitFinished(bool success)
{
	if(!m->finished)
	{
		m->finished = true;
		emit sigFinished(success);
	}
}

void FetchThread::singleImageFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	const AsyncWebAccess::Status status = awa->status();
	const QImage image = awa->image();

	m->activeConnections.removeAll(awa);
	awa->deleteLater();

	if(!m->mayRun){
		return;
	}

	if(status == AsyncWebAccess::Status::GotData)
	{
		const QPixmap pm = QPixmap::fromImage(image);

		if(!pm.isNull())
		{
			spLog(Log::Debug, this) << "Found cover in " << m->acf->identifier() << " for " << m->id;

			m->pixmaps << pm;
			m->foundUrls << awa->url();

			emit sigCoverFound(m->pixmaps.count() - 1);
			emitFinished(true);

			return;
		}
	}

	if(!fetchNextCover())
	{
		spLog(Log::Warning, this) << "Could not fetch cover from " << m->acf->identifier();
	}
}

#include <QtSvg/QSvgRenderer>
#include <QPainter>
void FetchThread::multiImageFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());

	m->activeConnections.removeAll(awa);

	if(!m->mayRun){
		return;
	}

	if(awa->status() == AsyncWebAccess::Status::GotData)
	{
		QPixmap pm;
		if(awa->url().endsWith("svg", Qt::CaseInsensitive))
		{
			pm = QPixmap(1000, 1000);
			pm.fill(Qt::transparent);
			QPainter painter(&pm);
			QSvgRenderer renderer(awa->data());
			renderer.render(&painter);
		}

		if(pm.isNull())
		{
			pm = QPixmap::fromImage(awa->image());
		}

		if(!pm.isNull())
		{
			m->pixmaps << pm;
			m->foundUrls << awa->url();
			emit sigCoverFound(m->pixmaps.count() - 1);
		}
	}

	else
	{
		spLog(Log::Warning, this) << "Could not fetch multi cover " << m->acf->identifier();
	}

	awa->deleteLater();

	fetchNextCover();
}



QString FetchThread::url(int idx) const
{
	if(idx >= 0 && idx < m->foundUrls.size())
	{
		return m->foundUrls[idx];
	}

	return QString();
}

QPixmap FetchThread::pixmap(int idx) const
{
	if(idx >= 0 && idx < m->pixmaps.size())
	{
		return m->pixmaps[idx];
	}

	return QPixmap();
}

int FetchThread::foundImageCount() const
{
	return m->pixmaps.size();
}

