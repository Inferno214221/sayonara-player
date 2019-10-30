/* UserTaggingOperations.cpp */

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

#include "UserTaggingOperations.h"
#include "Editor.h"
#include "ChangeNotifier.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

using Tagging::UserOperations;
using Tagging::Editor;

struct UserOperations::Private
{
	DB::LibraryDatabase*	library_db=nullptr;

	Private(LibraryId library_id)
	{
		DB::Connector* db = DB::Connector::instance();
		library_db = db->library_db(library_id, db->db_id());
	}
};

UserOperations::UserOperations(LibraryId library_id, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(library_id);
}

UserOperations::~UserOperations() = default;

Editor* UserOperations::create_editor()
{
	auto* editor = new Tagging::Editor();

	connect(editor, &Tagging::Editor::sig_finished, this, &UserOperations::sig_finished);
	connect(editor, &Tagging::Editor::sig_progress, this, &UserOperations::sig_progress);
	connect(editor, &Tagging::Editor::sig_progress, this, &UserOperations::sig_progress);
	connect(editor, &Tagging::Editor::sig_progress, this, &UserOperations::sig_progress);

	return editor;
}

void UserOperations::run_editor(Editor* editor)
{
	auto* t = new QThread();
	t->setObjectName(QString("EditorWorkerUserOperations%1").arg(Util::random_string(10)));
	editor->moveToThread(t);

	connect(editor, &Tagging::Editor::sig_finished, t, &QThread::quit);
	connect(editor, &Tagging::Editor::sig_finished, editor, &QThread::deleteLater);

	connect(t, &QThread::started, editor, &Editor::commit);
	connect(t, &QThread::finished, t, &QObject::deleteLater);

	t->start();
}

void UserOperations::set_track_rating(const MetaData& md, Rating rating)
{
	set_track_rating(MetaDataList(md), rating);
}

void UserOperations::set_track_rating(const MetaDataList& v_md, Rating rating)
{
	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		MetaData md(v_md[i]);
		md.set_rating(rating);
		editor->update_track(i, md);
	}

	run_editor(editor);
}

void UserOperations::set_album_rating(const Album& album, Rating rating)
{
	m->library_db->updateAlbumRating(album.id, rating);

	Album album_new(album);
	album_new.rating = rating;

	AlbumList albums_old; albums_old << album;
	AlbumList albums_new; albums_new << album_new;

	Tagging::ChangeNotifier::instance()->update_albums(albums_old, albums_new);
}

void UserOperations::merge_artists(const Util::Set<Id>& artist_ids, ArtistId target_artist)
{
	if(artist_ids.isEmpty()) {
		return;
	}

	if(target_artist < 0){
		sp_log(Log::Warning, this) << "Cannot merge artist: Target artist id < 0";
		return;
	}

	bool show_album_artists = GetSetting(Set::Lib_ShowAlbumArtists);

	Artist artist;
	bool success = m->library_db->getArtistByID(target_artist, artist);
	if(!success){
		return;
	}

	Util::Set<ArtistId> wrong_ids = artist_ids;
	wrong_ids.remove(target_artist);

	MetaDataList v_md;
	m->library_db->getAllTracksByArtist(wrong_ids.toList(), v_md);

	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int idx=0; idx<v_md.count(); idx++)
	{
		MetaData md(v_md[idx]);
		if(show_album_artists){
			md.set_album_artist(artist.name(), artist.id());
		}

		else {
			md.set_artist_id(artist.id());
			md.set_artist(artist.name());
		}

		editor->update_track(idx, md);
	}

	run_editor(editor);

	for(auto it = artist_ids.begin(); it != artist_ids.end(); it++)
	{
		if(*it == target_artist){
			continue;
		}

		m->library_db->deleteArtist(*it);
	}
}

void UserOperations::merge_albums(const Util::Set<Id>& album_ids, AlbumId target_album)
{
	if(album_ids.isEmpty())	{
		return;
	}

	if(target_album < 0){
		sp_log(Log::Warning, this) << "Cannot merge albums: Target album id < 0";
		return;
	}

	Album album;
	bool success = m->library_db->getAlbumByID(target_album, album, true);
	if(!success) {
		return;
	}

	Util::Set<AlbumId> wrong_ids = album_ids;
	wrong_ids.remove(target_album);

	MetaDataList v_md;
	m->library_db->getAllTracksByAlbum(wrong_ids.toList(), v_md);

	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int idx=0; idx<v_md.count(); idx++)
	{
		MetaData md(v_md[idx]);
		md.set_album_id(album.id);
		md.set_album(album.name());

		editor->update_track(idx, md);
	}

	run_editor(editor);
}


void UserOperations::add_genre(Util::Set<Id> ids, const Genre& genre)
{
	MetaDataList v_md;
	m->library_db->getAllTracks(v_md);

	v_md.remove_tracks([&ids](const MetaData& md) {
		return (!ids.contains(md.id()));
	});

	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		editor->add_genre(i, genre);
	}

	run_editor(editor);
}


void UserOperations::delete_genre(const Genre& genre)
{
	MetaDataList v_md;
	m->library_db->getAllTracks(v_md);

	v_md.remove_tracks([&genre](const MetaData& md){
		return (!md.has_genre(genre));
	});

	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		editor->delete_genre(i, genre);
	}

	run_editor(editor);
}

void UserOperations::rename_genre(const Genre& genre, const Genre& new_genre)
{
	MetaDataList v_md;
	m->library_db->getAllTracks(v_md);

	v_md.remove_tracks([&genre](const MetaData& md){
		return (!md.has_genre(genre));
	});

	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		editor->delete_genre(i, genre);
		editor->add_genre(i, new_genre);
	}

	run_editor(editor);
}

void UserOperations::add_genre_to_md(const MetaDataList& v_md, const Genre& genre)
{
	auto* editor = create_editor();
	editor->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		editor->add_genre(i, genre);
	}

	run_editor(editor);
}
