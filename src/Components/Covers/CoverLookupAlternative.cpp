/* CoverLookupAlternative.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "CoverLookup.h"
#include "CoverLookupAlternative.h"
#include "CoverLocation.h"
#include "CoverFetchManager.h"

#include <QStringList>

using Cover::AlternativeLookup;
using Cover::Location;
using Cover::Lookup;
using Cover::LookupBase;
using Cover::Fetcher::Manager;

struct AlternativeLookup::Private
{
	Location	cover_location;
	Lookup*		lookup=nullptr;

	int			n_covers;

	Private(AlternativeLookup* alt_lookup, int n_covers) :
		n_covers(n_covers)
	{
		lookup = new Lookup(alt_lookup, n_covers);
	}

	~Private()
	{
		lookup->stop();
	}
};

AlternativeLookup::AlternativeLookup(QObject* parent, int n_covers) :
	LookupBase(parent)
{
	m = Pimpl::make<Private>(this, n_covers);

	connect(m->lookup, &Lookup::sig_cover_found, this, &AlternativeLookup::sig_cover_found);
	connect(m->lookup, &Lookup::sig_finished, this, &AlternativeLookup::sig_finished);
}

AlternativeLookup::~AlternativeLookup() {}

void AlternativeLookup::stop()
{
	m->lookup->stop();
}

void AlternativeLookup::go(const Cover::Location& location)
{
	bool can_fetch = m->lookup->fetch_cover(location, true);
	if(!can_fetch)
	{
		emit sig_finished(false);
	}

	else {
		emit sig_started();
	}
}

void AlternativeLookup::start()
{
	go(m->cover_location);
}


void AlternativeLookup::start(const QString& cover_fetcher_identifier)
{
	Location cl = m->cover_location;
	QStringList search_urls = m->cover_location.search_urls();
	QString search_url;

	Manager* cfm = Manager::instance();
	for(const QString& url : search_urls)
	{
		QString identifier = cfm->identifier_by_url(url);
		if(identifier == cover_fetcher_identifier){
			search_url = url;
			break;
		}
	}

	if(!search_url.isEmpty()){
		cl.set_search_urls({search_url});
	}

	go(cl);
}

void AlternativeLookup::start_text_search(const QString& search_term)
{
	Location cl = m->cover_location;
	cl.set_search_term(search_term);
	cl.enable_freetext_search(true);
	go(cl);
}

void AlternativeLookup::start_text_search(const QString& search_term, const QString& cover_fetcher_identifier)
{
	Location cl = m->cover_location;
	cl.set_search_term(search_term, cover_fetcher_identifier);
	cl.enable_freetext_search(true);
	go(cl);
}

Location AlternativeLookup::cover_location() const
{
	return m->cover_location;
}

void AlternativeLookup::set_cover_location(const Location& location)
{
	m->cover_location = location;
}
