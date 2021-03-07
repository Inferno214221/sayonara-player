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

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/globals.h"

#include <QPixmap>
#include <QStringList>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

#include <atomic>

using namespace Cover;
using Fetcher::Url;

namespace
{
	constexpr const auto Timeout = 5000U;

	bool isValidPixmap(const QPixmap& pixmap)
	{
		return (!pixmap.isNull() && (pixmap.width() >= 50) && (pixmap.height() >= 50));
	}

	QPixmap extractPixmap(const AsyncWebAccess* webAccess)
	{
		QPixmap pixmap;
		if(webAccess->url().endsWith("svg", Qt::CaseInsensitive))
		{
			pixmap = QPixmap(1000, 1000);
			pixmap.fill(Qt::transparent);
			auto painter = QPainter(&pixmap);
			auto renderer = QSvgRenderer(webAccess->data());
			renderer.render(&painter);
		}

		return (pixmap.isNull())
		       ? QPixmap::fromImage(webAccess->image())
		       : pixmap;
	}

	template<typename CallbackFunction>
	void startWebRequest(WebCoverFetcher* fetchThread, const QString& address, QList<AsyncWebAccess*>& requestList,
	                     CallbackFunction function)
	{
		auto* request = new AsyncWebAccess(fetchThread);
		fetchThread->connect(request, &AsyncWebAccess::sigFinished, fetchThread, function);

		requestList << request;

		request->setBehavior(AsyncWebAccess::Behavior::AsSayonara);
		request->run(address, Timeout);
	}
}

struct WebCoverFetcher::Private
{
	QList<AsyncWebAccess*> runningRequests;
	Cover::Fetcher::CoverFetcherPtr coverFetcher {nullptr};
	Cover::Fetcher::Manager* coverFetchManager {Cover::Fetcher::Manager::instance()};

	QList<QPixmap> pixmaps;

	QStringList imageAddresses;
	QStringList foundUrls;
	QList<Url> searchUrls;
	const int requestedCovers;
	std::atomic<bool> stopped {false};

	Private(const Location& coverLocation, int requestedCovers) :
		requestedCovers(requestedCovers)
	{
		Util::Algorithm::copyIf(coverLocation.searchUrls(), this->searchUrls, [&](const auto& url) {
			return (this->coverFetchManager->isActive(url.identifier()) && (!url.url().isEmpty()));
		});

		Util::Algorithm::remove_duplicates(this->searchUrls);
	}
};

WebCoverFetcher::WebCoverFetcher(QObject* parent, const Location& coverLocation, const int requestedCovers) :
	QObject(parent)
{
	m = Pimpl::make<Private>(coverLocation, requestedCovers);
}

WebCoverFetcher::~WebCoverFetcher()
{
	stop();
}

bool WebCoverFetcher::start()
{
	return startNextRequest();
}

void WebCoverFetcher::stop()
{
	if(!m->stopped)
	{
		m->stopped = true;
		m->searchUrls.clear();
		m->imageAddresses.clear();

		for(auto* webRequest : m->runningRequests)
		{
			webRequest->stop();
			webRequest->deleteLater();
		}

		m->runningRequests.clear();

		emit sigFinished();
	}
}

bool WebCoverFetcher::startNextRequest()
{
	if(m->requestedCovers == m->pixmaps.size())
	{
		stop();
		return true;
	}

	if(m->imageAddresses.isEmpty())
	{
		const auto canStart = processNextSearchUrl();
		if(canStart)
		{
			return true;
		}
	}

	return processNextImageUrl();
}

bool WebCoverFetcher::processNextSearchUrl()
{
	if(m->searchUrls.isEmpty())
	{
		return false;
	}

	const auto url = m->searchUrls.takeFirst();
	m->coverFetcher = m->coverFetchManager->coverfetcher(url);
	if(!m->coverFetcher)
	{
		return false;
	}

	const auto callback = m->coverFetcher->canFetchCoverDirectly()
		? &WebCoverFetcher::imageFetched
		: &WebCoverFetcher::contentFetched;

	startWebRequest(this, url.url(), m->runningRequests, callback);

	return true;
}

bool WebCoverFetcher::processNextImageUrl()
{
	if(!m->imageAddresses.isEmpty())
	{
		const auto address = m->imageAddresses.takeFirst();
		startWebRequest(this, address, m->runningRequests, &WebCoverFetcher::imageFetched);

		return true;
	}

	return false;
}

void WebCoverFetcher::contentFetched()
{
	auto* webAccess = dynamic_cast<AsyncWebAccess*>(sender());

	if(!m->stopped && (webAccess->status() == AsyncWebAccess::Status::GotData))
	{
		const auto websiteData = webAccess->data();
		m->imageAddresses = m->coverFetcher->parseAddresses(websiteData);
	}

	m->runningRequests.removeOne(webAccess);
	webAccess->deleteLater();

	startNextRequest();
}

void WebCoverFetcher::imageFetched()
{
	auto* webAccess = dynamic_cast<AsyncWebAccess*>(sender());
	if(!m->stopped)
	{
		if(webAccess->status() == AsyncWebAccess::Status::GotData)
		{
			const auto pixmap = extractPixmap(webAccess);
			if(isValidPixmap(pixmap))
			{
				m->pixmaps << pixmap;
				m->foundUrls << webAccess->url();

				emit sigCoverFound(m->pixmaps.count() - 1);
			}
		}
	}

	m->runningRequests.removeOne(webAccess);
	webAccess->deleteLater();

	startNextRequest();
}

QString WebCoverFetcher::url(int idx) const
{
	return (Util::between(idx, m->foundUrls))
	       ? m->foundUrls[idx]
	       : QString();
}

QPixmap WebCoverFetcher::pixmap(int idx) const
{
	return (Util::between(idx, m->pixmaps))
	       ? m->pixmaps[idx]
	       : QPixmap();
}
