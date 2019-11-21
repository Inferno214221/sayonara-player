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
#include "CoverUtils.h"
#include "Fetcher/CoverFetcherUrl.h"

#include "Fetcher/CoverFetcher.h"

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
using Cover::Fetcher::Url;
using UrlList=QList<Url>;

struct AlternativeLookup::Private
{
	Lookup*		lookup=nullptr;
	int			n_covers;
	bool		running;
	bool		silent;

	Private(const Cover::Location& cl, int n_covers, bool silent, AlternativeLookup* parent) :
		n_covers(n_covers),
		running(false),
		silent(silent)
	{
		lookup = new Lookup(cl, n_covers, parent);
	}

	~Private()
	{
		lookup->stop();
	}
};

AlternativeLookup::AlternativeLookup(const Cover::Location& cl, int n_covers, bool silent, QObject* parent) :
	LookupBase(cl, parent)
{
	m = Pimpl::make<Private>(cl, n_covers, silent, this);

	connect(m->lookup, &Lookup::sig_started, this, &AlternativeLookup::started);
	connect(m->lookup, &Lookup::sig_cover_found, this, &AlternativeLookup::cover_found);
	connect(m->lookup, &Lookup::sig_finished, this, &AlternativeLookup::finished);

	ListenSettingNoCall(Set::Cover_Server, AlternativeLookup::coverfetchers_changed);
}

AlternativeLookup::~AlternativeLookup() = default;

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

	if(!m->silent)
	{
		Cover::Utils::write_cover_to_db(cl, cover);
		Cover::Utils::write_cover_to_sayonara_dir(cl, cover);

		if(save_to_library) {
			Cover::Utils::write_cover_to_library(cl, cover);
		}
	}

	else
	{
		cover.save(cl.alternative_path());
	}

	emit sig_cover_changed(cl);

	return true;
}

bool AlternativeLookup::is_running() const
{
	return m->running;
}

bool AlternativeLookup::is_silent() const
{
	return m->silent;
}

QStringList AlternativeLookup::active_coverfetchers(AlternativeLookup::SearchMode mode) const
{
	using CoverFetcher=Cover::Fetcher::Base;

	auto* cfm = Cover::Fetcher::Manager::instance();
	const QList<CoverFetcher*> cover_fetchers = cfm->coverfetchers();

	QStringList ret;
	for(const CoverFetcher* cover_fetcher : cover_fetchers)
	{
		const QString identifier = cover_fetcher->identifier();
		if(!cfm->is_active(identifier)) {
			continue;
		}

		bool valid_identifier = false;
		if(mode == AlternativeLookup::SearchMode::Fulltext)
		{
			const QString address = cover_fetcher->search_address("some dummy text");
			valid_identifier = (!address.isEmpty());
		}

		else
		{
			const UrlList search_urls = cover_location().search_urls();

			valid_identifier = Algorithm::contains(search_urls, [identifier](const Url& url) {
				return (url.identifier().compare(identifier, Qt::CaseInsensitive) == 0);
			});
		}

		if(valid_identifier) {
			ret << identifier;
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

void AlternativeLookup::coverfetchers_changed()
{
	emit sig_coverfetchers_changed();
}

void AlternativeLookup::go(const Cover::Location& cl)
{
	m->lookup->set_cover_location(cl);
	m->lookup->start();

	emit sig_started();
}


void AlternativeLookup::start()
{
	go(cover_location());
}


void AlternativeLookup::start(const QString& identifier)
{
	Location cl = cover_location();
	const UrlList search_urls = cover_location().search_urls();

	auto it = Algorithm::find(search_urls, [&identifier](const Url& url) {
		return (identifier == url.identifier());
	});

	if(it != search_urls.end())
	{
		Url url = *it;
		cl.set_search_urls({url});
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

