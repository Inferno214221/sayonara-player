/* Manager.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

static void sort_coverfetchers(QList<Fetcher::Base*>& lst, const SortMap& cf_order)
{
	Algorithm::sort(lst, [&cf_order](Fetcher::Base* t1, Fetcher::Base* t2)
	{
		const int order1 = cf_order[t1->identifier()];
		const int order2 = cf_order[t2->identifier()];

		if(order1 != order2) {
			if(order1 == -1){
				return false; // order1 is worse
			}

			if(order2 == -1){
				return true; // order1 is better
			}

			return (order1 < order2);
		}

		const int rating1 = t1->estimated_size();
		const int rating2 = t2->estimated_size();

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


static Fetcher::Base* coverfetcher_by_identifier(const QString& identifier, const QList<Fetcher::Base*>& container)
{
	if(identifier.isEmpty()){
		return nullptr;
	}

	for(Fetcher::Base* cfi : container)
	{
		const QString cfi_identifier = cfi->identifier();
		if(!cfi_identifier.isEmpty())
		{
			if(cfi_identifier.compare(identifier, Qt::CaseInsensitive) == 0)
			{
				return cfi;
			}
		}
	}

	return nullptr;
}

struct Manager::Private
{
	QMap<QString, int>		cf_order;
	QMap<QString, bool>		active_map;
	QList<Fetcher::Base*>	coverfetchers;
	Fetcher::Website*		website_coverfetcher=nullptr;
	Fetcher::DirectFetcher* direct_coverfetcher=nullptr;

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

		if(identifier == direct_coverfetcher->identifier()){
			return true;
		}

		else if(identifier == website_coverfetcher->identifier()){
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

	register_coverfetcher(new Fetcher::Allmusic());
	register_coverfetcher(new Fetcher::Amazon());
	register_coverfetcher(new Fetcher::Audioscrobbler());
	register_coverfetcher(new Fetcher::Discogs());
	register_coverfetcher(new Fetcher::Google());
	register_coverfetcher(new Fetcher::Yandex());

	m->website_coverfetcher = new Fetcher::Website();
	m->direct_coverfetcher = new Fetcher::DirectFetcher();
	register_coverfetcher(m->direct_coverfetcher);
	register_coverfetcher(m->website_coverfetcher);

	ListenSetting(Set::Cover_Server, Manager::servers_changed);
}

Manager::~Manager() = default;

void Manager::register_coverfetcher(Base* t)
{
	Fetcher::Base* cfi = coverfetcher_by_identifier(t->identifier(), m->coverfetchers);
	if(cfi){ // already there
		return;
	}

	m->set_active(t->identifier(), true);
	m->coverfetchers << t;
}


Fetcher::Base* Manager::coverfetcher(const Url& url) const
{
	const QString& identifier = url.identifier();
	Fetcher::Base* cfi = coverfetcher_by_identifier(identifier, m->coverfetchers);

	if(identifier == m->website_coverfetcher->identifier())
	{
		auto* website_fetcher = dynamic_cast<Website*>(cfi);
		if(website_fetcher)
		{
			website_fetcher->set_website(url.url());
		}
	}

	else if(identifier == m->direct_coverfetcher->identifier())
	{
		auto* direct_fetcher = dynamic_cast<DirectFetcher*>(cfi);
		if(direct_fetcher)
		{
			direct_fetcher->set_direct_url(url.url());
		}
	}

	return cfi;
}

Fetcher::Url Manager::direct_fetch_url(const QString& url)
{
	static Cover::Fetcher::DirectFetcher df;
	return Cover::Fetcher::Url(df.identifier(), url);
}


QList<Fetcher::Base*> Manager::coverfetchers() const
{
	return m->coverfetchers;
}

QList<Fetcher::Base*> Manager::active_coverfetchers() const
{
	QList<Fetcher::Base*> ret;
	for(Fetcher::Base* cfi : m->coverfetchers)
	{
		const QString identifier = cfi->identifier();
		if( (identifier == m->direct_coverfetcher->identifier()) ||
			(identifier == m->website_coverfetcher->identifier()) )
		{
			continue;
		}

		if(is_active(cfi)){
			ret << cfi;
		}
	}

	return ret;
}

QList<Fetcher::Base*> Manager::inactive_coverfetchers() const
{
	QList<Fetcher::Base*> ret;
	for(Fetcher::Base* cfi : m->coverfetchers)
	{
		const QString identifier = cfi->identifier();
		if( (identifier == m->direct_coverfetcher->identifier()) ||
			(identifier == m->website_coverfetcher->identifier()) )
		{
			continue;
		}

		if(!is_active(cfi)){
			ret << cfi;
		}
	}

	return ret;
}

bool Manager::is_active(const Fetcher::Base* cfi) const
{
	return is_active(cfi->identifier());
}

bool Manager::is_active(const QString& identifier) const
{
	return m->is_active(identifier);
}


void Manager::servers_changed()
{
	QStringList servers = GetSetting(Set::Cover_Server);

	for(const QString& key : m->active_map.keys()){
		m->active_map[key] = servers.contains(key);
	}

	SortMap sortmap = create_sortmap(servers);
	sort_coverfetchers(m->coverfetchers, sortmap);
}


QList<Url> Manager::artist_addresses(const QString& artist) const
{
	QList<Url> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString identifier = cfi->identifier();
		const QString address = cfi->artist_address(artist);

		if(!address.isEmpty())
		{
			urls << Url(identifier, address);
		}
	}

	return urls;
}


QList<Url> Manager::album_addresses(const QString& artist, const QString& album) const
{
	QList<Url> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString identifier = cfi->identifier();
		const QString address = cfi->album_address(artist, album);

		if(!address.isEmpty())
		{
			urls << Url(identifier, address);
		}
	}

	return urls;
}

static bool is_searchstring_website(const QString& searchstring)
{
	if(Util::File::is_www(searchstring)){
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

QList<Url> Manager::search_addresses(const QString& searchstring) const
{
	if(is_searchstring_website(searchstring))
	{
		m->website_coverfetcher->set_website(searchstring);
		const QString identifier = m->website_coverfetcher->identifier();
		return { Url(identifier, m->website_coverfetcher->search_address("")) };
	}

	QList<Url> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString identifier = cfi->identifier();
		const QString address = cfi->search_address(searchstring);

		if(!address.isEmpty())
		{
			urls << Url(identifier, cfi->search_address(searchstring));
		}
	}

	return urls;
}

QList<Url> Manager::search_addresses(const QString& searchstring, const QString& cover_fetcher_identifier) const
{
	if(is_searchstring_website(searchstring))
	{
		m->website_coverfetcher->set_website(searchstring);
		const QString identifier = m->website_coverfetcher->identifier();
		return {Url(identifier, m->website_coverfetcher->search_address(""))};
	}

	QList<Url> urls;
	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		const QString address = cfi->search_address(searchstring);
		const QString identifier = cfi->identifier();

		if( (!address.isEmpty()) &&
			(is_active(cfi)) &&
			(cover_fetcher_identifier.compare(identifier, Qt::CaseInsensitive) == 0))
		{
			urls << Url(identifier, cfi->search_address(searchstring));;
		}
	}

	if(urls.isEmpty()){
		return search_addresses(searchstring);
	}

	return urls;
}
