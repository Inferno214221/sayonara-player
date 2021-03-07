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
#include "Utils/globals.h"

#include <QPixmap>
#include <QStringList>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

static const int Timeout = 5000;

using namespace Cover;
using Fetcher::Url;

struct FetchThread::Private
{
	AsyncWebAccess* currentWebAccess{nullptr};
	Cover::Fetcher::CoverFetcherPtr coverFetcher {nullptr};

	QList<QPixmap> pixmaps;
	QList<Url> searchUrls;              // links to websites found in the Cover::Location
	QStringList imageAddresses;         // direct links to images
	QStringList foundUrls;              // urls where images were successfully extracted from
	int coverCount;                     // amount of covers that should be fetched
	bool finished;
	bool mayRun;

	Private(const Location& cl, int nCovers) :
		coverCount(nCovers),
		finished(false),
		mayRun(true)
	{
		auto* cfm = Cover::Fetcher::Manager::instance();

		const QList<Url> urls = cl.searchUrls();
		for(const Url& url : urls)
		{
			const QString identifier = url.identifier();
			if(cfm->isActive(identifier))
			{
				if(!searchUrls.contains(url))
				{
					searchUrls << url;
				}
			}
		}
	}
};

FetchThread::FetchThread(QObject* parent, const Location& cl, const int nCovers) :
	QObject(parent)
{
	m = Pimpl::make<Private>(cl, nCovers);
}

FetchThread::~FetchThread()
{
	if(m->currentWebAccess)
	{
		m->currentWebAccess->stop();
		m->currentWebAccess->deleteLater();
		Util::sleepMs(50);
	}
}

bool FetchThread::start()
{
	if(m->searchUrls.isEmpty())
	{
		return false;
	}

	const Url url = m->searchUrls.takeFirst();

	auto* cfm = Fetcher::Manager::instance();
	m->coverFetcher = cfm->coverfetcher(url);
	if(!m->coverFetcher)
	{
		return false;
	}

	m->mayRun = true;

	if(m->coverFetcher->canFetchCoverDirectly())
	{
		m->imageAddresses.clear();
		if(!url.url().isEmpty())
		{
			m->imageAddresses << url.url();
		}

		fetchNextCover();
	}

	else
	{   // fetch website with cover links first
		m->currentWebAccess = new AsyncWebAccess(this);
		m->currentWebAccess->setObjectName(m->coverFetcher->identifier());
		m->currentWebAccess->setBehavior(AsyncWebAccess::Behavior::AsSayonara);
		connect(m->currentWebAccess, &AsyncWebAccess::sigFinished, this, &FetchThread::contentFetched);

		m->currentWebAccess->run(url.url(), Timeout);
	}

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

void FetchThread::contentFetched()
{
	if(m->mayRun)
	{
		if(m->currentWebAccess->status() == AsyncWebAccess::Status::GotData)
		{
			const QByteArray website = m->currentWebAccess->data();
			m->imageAddresses = m->coverFetcher->parseAddresses(website);
		}
	}

	const QString url = m->currentWebAccess->url();
	m->currentWebAccess->deleteLater();
	m->currentWebAccess = nullptr;

	if(!fetchNextCover())
	{
		spLog(Log::Warning, this) << "No addresses available";
	}
}

bool FetchThread::fetchNextCover()
{
	if(!m->mayRun)
	{
		return false;
	}

	if(m->coverCount == m->pixmaps.size())
	{
		emitFinished(true);
		return true;
	}

	if(m->imageAddresses.isEmpty())
	{
		bool success = start();
		if(!success)
		{
			emitFinished(false);
		}

		return success;
	}

	const QString address = m->imageAddresses.takeFirst();
	spLog(Log::Develop, this) << "Fetch cover from " << address;

	m->currentWebAccess = new AsyncWebAccess(this);
	m->currentWebAccess->setBehavior(AsyncWebAccess::Behavior::AsBrowser);

	connect(m->currentWebAccess, &AsyncWebAccess::sigFinished, this, &FetchThread::imageFetched);
	m->currentWebAccess->run(address, Timeout);

	return true;
}

void FetchThread::imageFetched()
{
	if(m->mayRun)
	{
		if(m->currentWebAccess->status() == AsyncWebAccess::Status::GotData)
		{
			QPixmap pm;
			if(m->currentWebAccess->url().endsWith("svg", Qt::CaseInsensitive))
			{
				pm = QPixmap(1000, 1000);
				pm.fill(Qt::transparent);
				QPainter painter(&pm);
				QSvgRenderer renderer(m->currentWebAccess->data());
				renderer.render(&painter);
			}

			if(pm.isNull())
			{
				pm = QPixmap::fromImage(m->currentWebAccess->image());
			}

			if(!pm.isNull())
			{
				m->pixmaps << pm;
				m->foundUrls << m->currentWebAccess->url();
				emit sigCoverFound(m->pixmaps.count() - 1);
			}
		}
	}

	m->currentWebAccess->deleteLater();
	m->currentWebAccess = nullptr;

	fetchNextCover();
}

QString FetchThread::url(int idx) const
{
	return (Util::between(idx, m->foundUrls) ? m->foundUrls[idx] : QString());
}

QPixmap FetchThread::pixmap(int idx) const
{
	return (Util::between(idx, m->pixmaps) ? m->pixmaps[idx] : QPixmap());
}
