/* TagEdit.cpp */

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

#include "Editor.h"
#include "Expression.h"
#include "ChangeNotifier.h"
#include "Components/MetaDataInfo/MetaDataInfo.h"
#include "Components/Covers/CoverLocation.h"
#include "Database/CoverConnector.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/Filter.h"
#include "Utils/Library/Sortorder.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include <QHash>
#include <algorithm>

using namespace Tagging;

struct Editor::Private
{
	MetaDataList			v_md;			// the current metadata
	MetaDataList			v_md_orig;		// the original metadata

	MetaDataList			v_md_before_change;
	MetaDataList			v_md_after_change;
	BoolList				changed_md;	// indicates if metadata at idx was changed
	QMap<int, QPixmap>		cover_map;

	QHash<QString, ArtistId>	artist_map;
	QHash<QString, AlbumId>		album_map;

	DB::LibraryDatabase*	ldb=nullptr;	// database of LocalLibrary

	Private()
	{
		ldb = DB::Connector::instance()->library_db(-1, 0);

		AlbumList albums;
		ldb->getAllAlbums(albums, true);
		for(auto it=albums.begin(); it != albums.end(); it++)
		{
			if(album_map.contains(it->name()))
			{
				sp_log(Log::Warning, this) << "Album " << it->name() << " already exists";
				continue;
			}

			album_map[it->name()] = it->id;
		}

		ArtistList artists;
		ldb->getAllArtists(artists, true);
		for(auto it=artists.begin(); it != artists.end(); it++)
		{
			if(artist_map.contains(it->name()))
			{
				sp_log(Log::Warning, this) << "Artist " << it->name() << " already exists";
				continue;
			}

			artist_map[it->name()] = it->id;
		}
	}

	ArtistId get_artist_id(const QString& artist_name)
	{
		if(artist_map.contains(artist_name)){
			return artist_map[artist_name];
		} else {
			ArtistId id = ldb->insertArtistIntoDatabase(artist_name);
			artist_map[artist_name] = id;
			return id;
		}
	}

	AlbumId get_album_id(const QString& album_name)
	{
		if(album_map.contains(album_name)){
			return album_map[album_name];
		} else {
			AlbumId id = ldb->insertAlbumIntoDatabase(album_name);
			album_map[album_name] = id;
			return id;
		}
	}
};

Editor::Editor(QObject *parent) :
	QThread(parent)
{
	m = Pimpl::make<Editor::Private>();


	this->setObjectName("TagEditor" + ::Util::random_string(4));

	connect(this, &QThread::finished, this, &Editor::thread_finished);
}

Editor::Editor(const MetaDataList& v_md, QObject* parent) :
	Editor(parent)
{
	set_metadata(v_md);
}

Editor::~Editor() {}


void Editor::update_track(int idx, const MetaData& md)
{
	bool has_changed = !( md.is_equal_deep( m->v_md_orig[idx]) );
	m->changed_md[idx] = has_changed;
	if(!has_changed)
	{
		return;
	}

	auto it = m->v_md.begin() + idx;
	*it = md;

	MetaData new_md = m->v_md[idx];

	sp_log(Log::Info, this) << new_md.artist_id;

}

void Editor::undo(int idx)
{
	m->v_md[idx] = m->v_md_orig[idx];
	m->changed_md[idx] = false;
}


void Editor::undo_all()
{
	m->v_md = m->v_md_orig;
	std::fill(m->changed_md.begin(), m->changed_md.end(), false);
}

const MetaData& Editor::metadata(int idx) const
{
	return m->v_md[idx];
}

const MetaDataList& Editor::metadata() const
{
	return m->v_md;
}

bool Editor::apply_regex(const QString& regex, int idx)
{
	if(!between(idx, m->v_md)){
		return false;
	}

	MetaData md = m->v_md[idx];

	Expression e(regex, md.filepath());
	if(!e.is_valid()){
		return false;
	}

	const QMap<Tagging::TagName, QString> captured_tags = e.captured_tags();
	for(auto it=captured_tags.begin(); it != captured_tags.end(); it++)
	{
		Tagging::TagName key = it.key();
		QString value = it.value();

		if(key == Tagging::TagTitle) {
			md.set_title(value);
		}

		else if(key == Tagging::TagAlbum) {
			md.set_album(value);
		}

		else if(key == Tagging::TagArtist) {
			md.set_artist(value);
		}

		else if(key == Tagging::TagTrackNum) {
			md.track_num = value.toInt();
		}

		else if(key == Tagging::TagYear) {
			md.year = value.toInt();
		}

		else if(key == Tagging::TagDisc) {
			md.discnumber = value.toInt();
		}
	}

	update_track(idx, md);

	return true;
}


int Editor::count() const
{
	return m->v_md.count();
}


bool Editor::has_changes() const
{
	return ::Util::contains(m->changed_md, [](bool b){
		return (b);
	});
}

void Editor::add_genre(int idx, const Genre& genre)
{
	if(!between(idx, m->v_md)){
		return;
	}

	MetaData& md = m->v_md[idx];

	if(md.add_genre(genre)){
		m->changed_md[idx] = true;
	}
}

void Editor::delete_genre(int idx, const Genre& genre)
{
	if(!between(idx, m->v_md)){
		return;
	}

	MetaData& md = m->v_md[idx];
	if(md.remove_genre(genre)){
		m->changed_md[idx] = true;
	}
}

void Editor::rename_genre(int idx, const Genre& genre, const Genre& new_genre)
{
	if(!between(idx, m->v_md)){
		return;
	}

	MetaData& md = m->v_md[idx];
	if(md.remove_genre(genre)){
		m->changed_md[idx] = true;
	}

	if(md.add_genre(new_genre)){
		m->changed_md[idx] = true;
	}
}


void Editor::set_metadata(const MetaDataList& v_md)
{
	m->v_md = v_md;
	m->v_md_orig = v_md;

	m->cover_map.clear();
	m->changed_md.clear();

	m->changed_md.reserve(v_md.count());
	for(const MetaData& md : v_md) { Q_UNUSED(md); m->changed_md << false; }

	if( v_md.size() > 0)
	{
		m->ldb = DB::Connector::instance()->library_db(v_md.first().library_id, 0);
	}

	emit sig_metadata_received(m->v_md);
}

bool Editor::is_cover_supported(int idx) const
{
	return Tagging::Covers::is_cover_supported( m->v_md[idx].filepath() );
}

bool Editor::can_load_entire_album() const
{
	MetaDataInfo info(m->v_md);
	return (info.album_ids().count() == 1);
}

void Editor::load_entire_album()
{
	MetaDataInfo info(m->v_md);
	if(info.album_ids().empty()){
		return;
	}

	AlbumId id = info.album_ids().first();
	MetaDataList v_md;

	m->ldb->getAllTracksByAlbum(id, v_md, ::Library::Filter(), ::Library::SortOrder::TrackNumAsc);
	set_metadata(v_md);
}


void Editor::apply_artists_and_albums_to_md()
{
	for(int i=0; i<m->v_md.count(); i++)
	{
		bool changed = m->changed_md[i];
		if( !changed )
		{
			continue;
		}

		MetaData& md = m->v_md[i];

		ArtistId artist_id =	m->get_artist_id(md.artist());
		AlbumId album_id =		m->get_album_id(md.album());
		ArtistId album_artist_id =	m->get_artist_id(md.album_artist());

		md.album_id = album_id;
		md.artist_id = artist_id;
		md.set_album_artist_id(album_artist_id);
	}
}


void Editor::update_cover(int idx, const QPixmap& cover)
{
	if(cover.isNull()){
		return;
	}

	if(!between(idx, m->v_md) ){
		return;
	}

	bool cover_supported = is_cover_supported(idx);
	if(!cover_supported) {
		return;
	}

	m->cover_map[idx] = cover;
}


bool Editor::has_cover_replacement(int idx) const
{
	return m->cover_map.contains(idx);
}

void Editor::commit()
{
	this->start();
}

void Editor::run()
{
	MetaDataList v_md;
	MetaDataList v_md_orig;

	sp_log(Log::Debug, this) << "Apply albums and artists";
	apply_artists_and_albums_to_md();

	sp_log(Log::Debug, this) << "Have to change" <<
						  std::count(m->changed_md.begin(), m->changed_md.end(), true)
					   << " tracks";

	int i=0;
	int n_operations = m->v_md.count() + m->cover_map.size();

	for(auto i=0; i < m->v_md.count(); i++)
	{
		const MetaData& md = m->v_md[i];

		if(n_operations > 5){
			emit sig_progress( (i * 100) / n_operations);
		}

		if( m->changed_md[i] == false ) {
			continue;
		}

		bool success = Tagging::Utils::setMetaDataOfFile(md);
		if( !success ) {
			continue;
		}

		if( !md.is_extern && md.id >= 0 ){
			success = m->ldb->updateTrack(md);
		}

		if(success){
			v_md << std::move(md);
			v_md_orig.push_back(m->v_md_orig[i]);
		}
	}

	DB::Connector* db = DB::Connector::instance();
	DB::Covers* db_covers = db->cover_connector();

	for(auto it=m->cover_map.cbegin(); it != m->cover_map.cend(); it++)
	{
		int idx = it.key();
		QPixmap pm = it.value();

		const MetaData& md = *(m->v_md.cbegin() + idx);

		Tagging::Covers::write_cover(md.filepath(), pm);
		if(n_operations > 5){
			emit sig_progress( (i++ * 100) / n_operations);
		}

		Cover::Location cl = Cover::Location::cover_location(md);
		db_covers->set_cover(cl.hash(), pm);
	}


	DB::Library* db_library = db->library_connector();

	db_library->create_indexes();
	db->clean_up();

	m->v_md_after_change = v_md;
	m->v_md_before_change = v_md_orig;
	m->v_md_orig = m->v_md;

	emit sig_progress(-1);
}

void Editor::thread_finished()
{
	ChangeNotifier::instance()->change_metadata(m->v_md_before_change, m->v_md_after_change);

	emit sig_finished();
}



