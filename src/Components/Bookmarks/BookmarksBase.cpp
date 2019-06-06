/* BookmarksBase.cpp */

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

#include "BookmarksBase.h"
#include "Bookmark.h"

#include "Database/Connector.h"
#include "Database/Bookmarks.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"

namespace Algorithm=Util::Algorithm;

struct BookmarksBase::Private
{
	QList<Bookmark> bookmarks;
	DB::Bookmarks*	db=nullptr;
	MetaData		md;

	Private()
	{
		db = DB::Connector::instance()->bookmark_connector();
	}
};

BookmarksBase::BookmarksBase(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

BookmarksBase::~BookmarksBase() = default;

bool BookmarksBase::load()
{
	QMap<Seconds, QString> bookmarks;
	if(m->md.id >= 0)
	{
		m->db->searchBookmarks(m->md.id, bookmarks);
	}

	this->clear();
	for(auto it=bookmarks.cbegin(); it != bookmarks.cend(); it++)
	{
		m->bookmarks << Bookmark(it.key(), it.value(), true);
	}

	sort();

	return true;
}


void BookmarksBase::sort()
{
	Algorithm::sort(m->bookmarks, [](const Bookmark& bm1, const Bookmark& bm2){
		return (bm1.timestamp() < bm2.timestamp());
	});
}

BookmarksBase::CreationStatus BookmarksBase::create(Seconds timestamp)
{
	if(m->md.id < 0 || m->md.db_id() != 0)
	{
		return CreationStatus::NoDBTrack;
	}

	if(timestamp == 0) {
		return CreationStatus::OtherError;
	}

	bool already_there = Algorithm::contains(m->bookmarks, [&timestamp](const Bookmark& bm){
		return (bm.timestamp() == timestamp);
	});

	if(already_there){
		return CreationStatus::AlreadyThere;
	}

	QString name = Util::cvt_ms_to_string(timestamp * 1000, true, true, false);
	bool success = m->db->insertBookmark(m->md.id, timestamp, name);

	if(success)
	{
		load();
		return CreationStatus::Success;
	}

	return CreationStatus::DBError;
}

MetaData BookmarksBase::metadata() const
{
	return m->md;
}

void BookmarksBase::set_metadata(const MetaData& md)
{
	m->md = md;

	this->clear();

	if(!md.get_custom_field("Chapter1").isEmpty())
	{
		int chapter_idx = 1;
		QString entry;

		do
		{
			QString custom_field_name = QString("Chapter%1").arg(chapter_idx);

			entry = md.get_custom_field(custom_field_name);

			QStringList lst = entry.split(":");
			Seconds length = lst.takeFirst().toInt();
			QString name = lst.join(":");

			m->bookmarks << Bookmark(length, name, true);
			chapter_idx++;

		} while( !entry.isEmpty() );
	}

	else if(md.id >= 0)
	{
		QMap<Seconds, QString> bookmarks;
		m->db->searchBookmarks(md.id, bookmarks);

		this->clear();

		for(auto it=bookmarks.cbegin(); it != bookmarks.cend(); it++)
		{
			m->bookmarks << Bookmark(it.key(), it.value(), true);
		}
	}

	this->sort();
}

const QList<Bookmark> BookmarksBase::bookmarks() const
{
	return m->bookmarks;
}

void BookmarksBase::set_bookmarks(const QList<Bookmark> bookmarks)
{
	m->bookmarks = bookmarks;
}

void BookmarksBase::add(const Bookmark& bookmark)
{
	m->bookmarks.push_back(bookmark);
}

int BookmarksBase::count()
{
	return m->bookmarks.count();
}

void BookmarksBase::clear()
{
	m->bookmarks.clear();
}

const Bookmark& BookmarksBase::bookmark(int idx) const
{
	return m->bookmarks[idx];
}

Bookmark&BookmarksBase::bookmark(int idx)
{
	return m->bookmarks[idx];
}

bool BookmarksBase::remove(int idx)
{
	if(!Util::between(idx, this->count())){
		return false;
	}

	bool success = m->db->removeBookmark(m->md.id, m->bookmarks[idx].timestamp());

	if(success){
		load();
	}

	return success;
}

