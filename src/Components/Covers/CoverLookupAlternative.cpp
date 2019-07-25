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
#include "CoverFetcherInterface.h"
#include "CoverUtils.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>

namespace Algorithm=Util::Algorithm;
using Cover::AlternativeLookup;
using Cover::Location;
using Cover::Lookup;
using Cover::LookupBase;
using Cover::Fetcher::Manager;

struct AlternativeLookup::Private
{
	Lookup*		lookup=nullptr;
	int			n_covers;
	bool		running;

	Private(const Cover::Location& cl, int n_covers, AlternativeLookup* parent) :
		n_covers(n_covers),
		running(false)
	{
		lookup = new Lookup(cl, n_covers, parent);
	}

	~Private()
	{
		lookup->stop();
	}
};

AlternativeLookup::AlternativeLookup(const Cover::Location& cl, int n_covers, QObject* parent) :
	LookupBase(cl, parent)
{
	m = Pimpl::make<Private>(cl, n_covers, this);

	connect(m->lookup, &Lookup::sig_started, this, &AlternativeLookup::started);
	connect(m->lookup, &Lookup::sig_cover_found, this, &AlternativeLookup::cover_found);
	connect(m->lookup, &Lookup::sig_finished, this, &AlternativeLookup::finished);
}

AlternativeLookup::~AlternativeLookup() {}

void AlternativeLookup::stop()
{
	m->lookup->stop();
	m->running = false;
}

void AlternativeLookup::reset()
{
	stop();
	Cover::Utils::delete_temp_covers();
}


bool AlternativeLookup::save(const QPixmap& cover, bool save_to_library)
{
	if(cover.isNull()){
		sp_log(Log::Warning, this) << "Cannot save invalid cover";
		return false;
	}

	Cover::Location cl = cover_location();

	Cover::Utils::write_cover_to_db(cl, cover);
	Cover::Utils::write_cover_to_sayonara_dir(cl, cover);

	if(save_to_library) {
		Cover::Utils::write_cover_to_library(cl, cover);
	}

	emit sig_cover_changed(cl);

	return true;
}

bool AlternativeLookup::is_running() const
{
	return m->running;
}


QStringList AlternativeLookup::get_activated_coverfetchers(bool fulltext_search) const
{
	QStringList ret;
	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();
	QList<Cover::Fetcher::Base*> cover_fetchers = cfm->coverfetchers();
	for(const Cover::Fetcher::Base* cover_fetcher : cover_fetchers)
	{
		QString keyword = cover_fetcher->keyword();
		if(keyword.isEmpty()){
			continue;
		}

		bool suitable = false;
		if(fulltext_search) {
			suitable = cover_fetcher->is_search_supported();
		}

		else
		{
			QStringList search_urls = cover_location().search_urls();
			suitable = Algorithm::contains(search_urls, [=](const QString& url){
				QString id = cfm->identifier_by_url(url);
				return (id == keyword);
			});
		}

		if(suitable){
			ret << cover_fetcher->keyword();
		}
	}

	return ret;
}

void AlternativeLookup::started()
{
	m->running = true;
	emit sig_started();
}

void AlternativeLookup::finished(bool success)
{
	m->running = false;
	emit sig_finished(success);
}

void AlternativeLookup::cover_found(const QPixmap& pm)
{
	emit sig_cover_found(pm);
}


void AlternativeLookup::go(const Cover::Location& cl)
{
	set_cover_location(cl);

	m->lookup->set_cover_location(cl);
	m->lookup->start();

	emit sig_started();
}

void AlternativeLookup::start()
{
	go(cover_location());
}


void AlternativeLookup::start(const QString& cover_fetcher_identifier)
{
	Location cl = cover_location();
	QStringList search_urls = cover_location().search_urls();
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
	Location cl = cover_location();
	cl.set_search_term(search_term);
	cl.enable_freetext_search(true);
	go(cl);
}

void AlternativeLookup::start_text_search(const QString& search_term, const QString& cover_fetcher_identifier)
{
	Location cl = cover_location();
	cl.set_search_term(search_term, cover_fetcher_identifier);
	cl.enable_freetext_search(true);
	go(cl);
}

