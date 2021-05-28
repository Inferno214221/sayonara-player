/* Manager.cpp */

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

#include "CoverFetchManager.h"
#include "Fetcher/CoverFetcher.h"
#include "Fetcher/CoverFetcherUrl.h"

#include "Fetcher/Google.h"
#include "Fetcher/Audioscrobbler.h"
#include "Fetcher/Discogs.h"
#include "Fetcher/Allmusic.h"
#include "Fetcher/Amazon.h"
#include "Fetcher/Yandex.h"
#include "Fetcher/Website.h"
#include "Fetcher/DirectFetcher.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/FileUtils.h"

#include <QList>
#include <QMap>

#include <functional>

using namespace Cover;
using Cover::Fetcher::CoverFetcherList;
using Cover::Fetcher::CoverFetcherPtr;
using Cover::Fetcher::Manager;
using Cover::Fetcher::Url;

using SortMap = QMap<QString, int>;

namespace
{
	void sortCoverfetchers(CoverFetcherList& lst, const SortMap& sortMap)
	{
		Util::Algorithm::sort(lst, [&sortMap](auto fetcher1, auto fetcher2) {
			const auto order1 = sortMap[fetcher1->identifier()];
			const auto order2 = sortMap[fetcher2->identifier()];

			if(order1 != order2)
			{
				if(order1 == -1)
				{
					return false; // order1 is worse
				}

				if(order2 == -1)
				{
					return true; // order1 is better
				}

				return (order1 < order2);
			}

			const auto rating1 = fetcher1->estimatedSize();
			const auto rating2 = fetcher2->estimatedSize();

			return (rating1 > rating2);
		});
	}

	SortMap createSortmap(const QStringList& identifiers)
	{
		SortMap ret;
		for(int i = 0; i < identifiers.size(); i++)
		{
			ret.insert(identifiers[i], i);
		}

		return ret;
	}

	CoverFetcherPtr coverfetcherByIdentifier(const QString& identifier, const CoverFetcherList& fetchers)
	{
		if(identifier.isEmpty())
		{
			return nullptr;
		}

		const auto it = Util::Algorithm::find(fetchers, [&](const auto fetcher) {
			const auto fetcherIdentifier = fetcher->identifier().toLower();
			return (fetcherIdentifier == identifier.toLower());
		});

		return (it != fetchers.end()) ? *it : nullptr;
	}

	QList<Url> extractAddresses(const CoverFetcherList& fetchers,
	                            std::function<QString(const CoverFetcherPtr)> addressExtractor)
	{
		QList<Url> urls;
		for(const auto& fetcher : fetchers)
		{
			if(const auto address = addressExtractor(fetcher); !address.isEmpty())
			{
				urls << Url(fetcher->identifier(), address);
			}
		}

		return urls;
	}
}

struct Manager::Private
{
	QString websiteFetcherIdentifer;
	QString directFetcherIdentifier;
	QMap<QString, bool> activeMap;
	CoverFetcherList coverfetchers;

	Private() :
		websiteFetcherIdentifer {Website().identifier()},
		directFetcherIdentifier {DirectFetcher().identifier()} {}

	~Private()
	{
		coverfetchers.clear();
	}

	void setActive(const QString& identifier, bool enabled)
	{
		activeMap[identifier.toLower()] = enabled;
	}

	bool isActive(const QString& identifier) const
	{
		const auto identifierLower = identifier.toLower();
		if((identifierLower == directFetcherIdentifier) ||
		   (identifierLower == websiteFetcherIdentifer))
		{
			return true;
		}

		return activeMap.value(identifierLower, false);
	}
};

Manager::Manager() :
	QObject()
{
	m = Pimpl::make<Private>();

	registerCoverFetcher(std::make_shared<Fetcher::Allmusic>());
	registerCoverFetcher(std::make_shared<Fetcher::Amazon>());
	registerCoverFetcher(std::make_shared<Fetcher::Audioscrobbler>());
	registerCoverFetcher(std::make_shared<Fetcher::Discogs>());
	registerCoverFetcher(std::make_shared<Fetcher::Google>());
	registerCoverFetcher(std::make_shared<Fetcher::Yandex>());
	registerCoverFetcher(std::make_shared<Fetcher::Website>());
	registerCoverFetcher(std::make_shared<Fetcher::DirectFetcher>());

	ListenSetting(Set::Cover_Server, Manager::serversChanged);
}

Manager::~Manager() = default;

void Manager::registerCoverFetcher(CoverFetcherPtr fetcher)
{
	const auto temporaryFetcher = coverfetcherByIdentifier(fetcher->identifier(), m->coverfetchers);
	if(!temporaryFetcher)
	{
		m->setActive(fetcher->identifier(), true);
		m->coverfetchers << fetcher;
	}
}

CoverFetcherPtr Manager::coverfetcher(const Url& url) const
{
	const auto& identifier = url.identifier();
	return (identifier == m->websiteFetcherIdentifer)
	       ? std::make_shared<Website>(url.url())
	       : coverfetcherByIdentifier(identifier, m->coverfetchers);
}

Fetcher::Url Manager::directFetcherUrl(const QString& url) const
{
	return Url(m->directFetcherIdentifier, url);
}

Url Fetcher::Manager::websiteFetcherUrl(const QString& url) const
{
	return Url(m->websiteFetcherIdentifer, url);
}

CoverFetcherList Manager::coverfetchers() const
{
	return m->coverfetchers;
}

bool Manager::isActive(const CoverFetcherPtr fetcher) const
{
	return isActive(fetcher->identifier());
}

bool Manager::isActive(const QString& identifier) const
{
	return m->isActive(identifier);
}

void Manager::serversChanged()
{
	const auto servers = GetSetting(Set::Cover_Server);
	for(const auto& key : m->activeMap.keys())
	{
		m->activeMap[key] = servers.contains(key);
	}

	const auto sortMap = createSortmap(servers);
	sortCoverfetchers(m->coverfetchers, sortMap);
}

QList<Url> Manager::artistAddresses(const QString& artist) const
{
	return extractAddresses(m->coverfetchers, [&](const auto fetcher) {
		return fetcher->artistAddress(artist);
	});
}

QList<Url> Manager::albumAddresses(const QString& artist, const QString& album) const
{
	return extractAddresses(m->coverfetchers, [&](const auto fetcher) {
		return fetcher->albumAddress(artist, album);
	});
}

QList<Url> Manager::searchAddresses(const QString& searchstring) const
{
	if(isSearchstringWebsite(searchstring))
	{
		const auto url = websiteFetcherUrl(searchstring);
		return {url};
	}

	return extractAddresses(m->coverfetchers, [&](const auto fetcher) {
		return fetcher->fulltextSearchAddress(searchstring);
	});
}

QList<Url> Manager::searchAddresses(const QString& searchstring, const QString& coverFetcherIdentifier) const
{
	if(isSearchstringWebsite(searchstring))
	{
		const auto url = websiteFetcherUrl(searchstring);
		return {url};
	}

	const auto fetcher = coverfetcherByIdentifier(coverFetcherIdentifier, m->coverfetchers);
	if(fetcher && isActive(fetcher))
	{
		const auto url = Url(fetcher->identifier(), fetcher->fulltextSearchAddress(searchstring));
		return {url};
	}

	return QList<Url>();
}

bool Manager::isSearchstringWebsite(const QString& searchstring)
{
	if(Util::File::isWWW(searchstring) && (!searchstring.contains(QRegExp("\\s"))))
	{
		return true;
	}

	if(searchstring.startsWith("file://"))
	{
		return false;
	}

	const auto lastDot = searchstring.lastIndexOf(".");
	const auto isValid = (lastDot >= 0) &&
	                     (lastDot >= searchstring.size() - 4) &&
	                     (lastDot < searchstring.size() - 2);

	return isValid;
}
