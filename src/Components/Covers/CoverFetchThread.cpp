/* CoverFetchThread.cpp */

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
#include "Utils/WebAccess/WebClientImpl.h"
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
		constexpr const auto MinimumPixmapSize = 50;
		return !pixmap.isNull() &&
		       (pixmap.width() >= MinimumPixmapSize) &&
		       (pixmap.height() >= MinimumPixmapSize);
	}

	QPixmap extractPixmap(const WebClient* webAccess)
	{
		QPixmap pixmap;
		if(webAccess->url().endsWith("svg", Qt::CaseInsensitive))
		{
			constexpr const auto PixmapSize = 1000;
			pixmap = QPixmap(PixmapSize, PixmapSize);
			pixmap.fill(Qt::transparent);
			auto painter = QPainter(&pixmap);
			auto renderer = QSvgRenderer(webAccess->data());
			renderer.render(&painter);
		}

		return (pixmap.isNull())
		       ? QPixmap::fromImage(QImage::fromData(webAccess->data()))
		       : pixmap;
	}

	template<typename CallbackFunction>
	void startWebRequest(WebCoverFetcher* fetchThread, const QString& address, QList<WebClient*>& requestList,
	                     CallbackFunction function)
	{
		auto* request = new WebClientImpl(fetchThread);
		fetchThread->connect(request, &WebClient::sigFinished, fetchThread, function);

		requestList << request;

		request->setMode(WebClient::Mode::AsBrowser);
		request->run(address, Timeout);
	}
}

struct WebCoverFetcher::Private
{
	QList<WebClient*> runningRequests;
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

		for(auto* webRequest: m->runningRequests)
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
	auto* webClient = dynamic_cast<WebClient*>(sender());

	if(!m->stopped && (webClient->status() == WebClient::Status::GotData))
	{
		const auto websiteData = webClient->data();
		m->imageAddresses = m->coverFetcher->parseAddresses(websiteData);
	}

	m->runningRequests.removeOne(webClient);
	webClient->deleteLater();

	startNextRequest();
}

void WebCoverFetcher::imageFetched()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());
	if(!m->stopped)
	{
		if(webClient->status() == WebClient::Status::GotData)
		{
			const auto pixmap = extractPixmap(webClient);
			if(isValidPixmap(pixmap))
			{
				m->pixmaps << pixmap;
				m->foundUrls << webClient->url();

				emit sigCoverFound(m->pixmaps.count() - 1);
			}
		}
	}

	m->runningRequests.removeOne(webClient);
	webClient->deleteLater();

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
