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
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QStringList>
#include <QList>
#include <QMap>

namespace Algorithm=Util::Algorithm;
using namespace Cover;
using Cover::Fetcher::Manager;
using Cover::Fetcher::Base;
using Cover::Fetcher::Url;

using SortMap=QMap<QString, int>;

static void sort_coverfetchers(QList<Fetcher::Base*>& lst, const SortMap& cfOrder)
{
	Algorithm::sort(lst, [&cfOrder](Fetcher::Base* t1, Fetcher::Base* t2)
	{
		const int order1 = cfOrder[t1->identifier()];
		const int order2 = cfOrder[t2->identifier()];

		if(order1 != order2) {
			if(order1 == -1){
				return false; // order1 is worse
			}

			if(order2 == -1){
				return true; // order1 is better
			}

			return (order1 < order2);
		}

		const int rating1 = t1->estimatedSize();
		const int rating2 = t2->estimatedSize();

		return (rating1 > rating2);
	});
}

static SortMap create_sortmap(const QStringList& lst)
{
	SortMap ret;

	for(int i=0; i<lst.size(); i++)
	{
		const QString str = lst[i];
		ret[str] = i;
	}

	return ret;
}


static Fetcher::Base* coverfetcherByIdentifier(const QString& identifier, const QList<Fetcher::Base*>& container)
{
	if(identifier.isEmpty()){
		return nullptr;
	}

	for(Fetcher::Base* cfi : container)
	{
		const QString cfiIdentifier = cfi->identifier();
		if(!cfiIdentifier.isEmpty())
		{
			if(cfiIdentifier.compare(identifier, Qt::CaseInsensitive) == 0)
			{
				return cfi;
			}
		}
	}

	return nullptr;
}

struct Manager::Private
{
	QMap<QString, int>		coverfetcherOrder;
	QMap<QString, bool>		active_map;
	QList<Fetcher::Base*>	coverfetchers;
	Fetcher::Website*		websiteCoverfetcher=nullptr;
	Fetcher::DirectFetcher* directCoverfetcher=nullptr;

	Private() = default;

	~Private()
	{
		for(auto it=coverfetchers.begin(); it != coverfetchers.end(); it++)
		{
			Fetcher::Base* b = *it;
			delete b;
		}

		coverfetchers.clear();
	}

	void set_active(QString identifier, bool enabled)
	{
		active_map[identifier.toLower()] = enabled;
	}

	bool is_active(QString identifier) const
	{
		identifier = identifier.toLower();

		if(identifier == directCoverfetcher->identifier()){
			return true;
		}

		else if(identifier == websiteCoverfetcher->identifier()){
			return true;
		}

		if(!active_map.keys().contains(identifier)){
			return false;
		}

		return active_map[identifier];
	}
};


Manager::Manager() :
	QObject()
{
	m = Pimpl::make<Private>();

	registerCoverFetcher(new Fetcher::Allmusic());
	registerCoverFetcher(new Fetcher::Amazon());
	registerCoverFetcher(new Fetcher::Audioscrobbler());
	registerCoverFetcher(new Fetcher::Discogs());
	registerCoverFetcher(new Fetcher::Google());
	registerCoverFetcher(new Fetcher::Yandex());

	m->websiteCoverfetcher = new Fetcher::Website();
	m->directCoverfetcher = new Fetcher::DirectFetcher();
	registerCoverFetcher(m->directCoverfetcher);
	registerCoverFetcher(m->websiteCoverfetcher);

	ListenSetting(Set::Cover_Server, Manager::serversChanged);
}

Manager::~Manager() = default;

void Manager::registerCoverFetcher(Base* t)
{
	Fetcher::Base* cfi = coverfetcherByIdentifier(t->identifier(), m->coverfetchers);
	if(cfi){ // already there
		return;
	}

	m->set_active(t->identifier(), true);
	m->coverfetchers << t;
}


Fetcher::Base* Manager::coverfetcher(const Url& url) const
{
	const QString& identifier = url.identifier();
	Fetcher::Base* cfi = coverfetcherByIdentifier(identifier, m->coverfetchers);

	if(identifier == m->websiteCoverfetcher->identifier())
	{
		auto* website_fetcher = dynamic_cast<Website*>(cfi);
		if(website_fetcher)
		{
			website_fetcher->setWebsite(url.url());
		}
	}

	else if(identifier == m->directCoverfetcher->identifier())
	{
		auto* direct_fetcher = dynamic_cast<DirectFetcher*>(cfi);
		if(direct_fetcher)
		{
			direct_fetcher->setDirectUrl(url.url());
		}
	}

	return cfi;
}

Fetcher::Url Manager::directFetcherUrl(const QString& url)
{
	static Cover::Fetcher::DirectFetcher df;
	return Cover::Fetcher::Url(df.identifier(), url);
}


QList<Fetcher::Base*> Manager::coverfetchers() const
{
	return m->coverfetchers;
}

QList<Fetcher::Base*> Manager::activeCoverfetchers() const
{
	QList<Fetcher::Base*> ret;
	for(Fetcher::Base* cfi : m->coverfetchers)
	{
		const QString identifier = cfi->identifier();
		if( (identifier == m->directCoverfetcher->identifier()) ||
			(identifier == m->websiteCoverfetcher->identifier()) )
		{
			continue;
		}

		if(isActive(cfi)){
			ret << cfi;
		}
	}

	return ret;
}

QList<Fetcher::Base*> Manager::inactiveCoverfetchers() const
{
	QList<Fetcher::Base*> ret;
	for(Fetcher::Base* cfi : m->coverfetchers)
	{
		const QString identifier = cfi->identifier();
		if( (identifier == m->directCoverfetcher->identifier()) ||
			(identifier == m->websiteCoverfetcher->identifier()) )
		{
			continue;
		}

		if(!isActive(cfi)){
			ret << cfi;
		}
	}

	return ret;
}

bool Manager::isActive(const Fetcher::Base* cfi) const
{
	return isActive(cfi->identifier());
}

bool Manager::isActive(const QString& identifier) const
{
	return m->is_active(identifier);
}


void Manager::serversChanged()
{
	QStringList servers = GetSetting(Set::Cover_Server);

	for(const QString& key : m->active_map.keys()){
		m->active_map[key] = servers.contains(key);
	}

	SortMap sortmap = create_sortmap(servers);
	sort_coverfetchers(m->coverfetchers, sortmap);
}


QList<Url> Manager::artistAddresses(const QString& artist) const
{
	QList<Url> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString identifier = cfi->identifier();
		const QString address = cfi->artistAddress(artist);

		if(!address.isEmpty())
		{
			urls << Url(identifier, address);
		}
	}

	return urls;
}


QList<Url> Manager::albumAddresses(const QString& artist, const QString& album) const
{
	QList<Url> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString identifier = cfi->identifier();
		const QString address = cfi->albumAddress(artist, album);

		if(!address.isEmpty())
		{
			urls << Url(identifier, address);
		}
	}

	return urls;
}

static bool is_searchstring_website(const QString& searchstring)
{
	if(Util::File::isWWW(searchstring)){
		return true;
	}

	// this.is.my.searchstring -> false
	// this.is.my.searchstring.urli -> true
	// this.is.my.searchstring.url -> true
	// this.is.my.searchstring.ur -> true
	// this.is.my.searchstring.u -> false
	int last_dot = searchstring.lastIndexOf(".");
	if((last_dot < 0) || (last_dot < searchstring.size() - 4) || (last_dot > searchstring.size() - 2))
	{
		return false;
	}

	return true;
}

QList<Url> Manager::searchAddresses(const QString& searchstring) const
{
	if(is_searchstring_website(searchstring))
	{
		m->websiteCoverfetcher->setWebsite(searchstring);
		const QString identifier = m->websiteCoverfetcher->identifier();
		return { Url(identifier, m->websiteCoverfetcher->fulltextSearchAddress("")) };
	}

	QList<Url> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString identifier = cfi->identifier();
		const QString address = cfi->fulltextSearchAddress(searchstring);

		if(!address.isEmpty())
		{
			urls << Url(identifier, cfi->fulltextSearchAddress(searchstring));
		}
	}

	return urls;
}

QList<Url> Manager::searchAddresses(const QString& searchstring, const QString& cover_fetcher_identifier) const
{
	if(is_searchstring_website(searchstring))
	{
		m->websiteCoverfetcher->setWebsite(searchstring);
		const QString identifier = m->websiteCoverfetcher->identifier();
		return {Url(identifier, m->websiteCoverfetcher->fulltextSearchAddress(""))};
	}

	QList<Url> urls;
	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString address = cfi->fulltextSearchAddress(searchstring);
		const QString identifier = cfi->identifier();

		if( (!address.isEmpty()) &&
			(isActive(cfi)) &&
			(cover_fetcher_identifier.compare(identifier, Qt::CaseInsensitive) == 0))
		{
			urls << Url(identifier, cfi->fulltextSearchAddress(searchstring));;
		}
	}

	if(urls.isEmpty()){
		return searchAddresses(searchstring);
	}

	return urls;
}
