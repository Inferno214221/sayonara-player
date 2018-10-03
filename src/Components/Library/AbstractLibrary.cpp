/* AbstractLibrary.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "AbstractLibrary.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Components/Tagging/Editor.h"

#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Utils.h"


#include <QHash>

struct AbstractLibrary::Private
{
	SP::Set<ArtistId> selected_artists;
	SP::Set<AlbumId> selected_albums;
	SP::Set<TrackID> selected_tracks;

	MetaDataList current_tracks;

	Tagging::Editor* tag_edit=nullptr;
	Playlist::Handler* playlist=nullptr;

	Library::Sortings sortorder;
	Library::Filter filter;
	bool loaded;

	Private()
	{
		loaded = false;
	}
};

AbstractLibrary::AbstractLibrary(QObject *parent) :
	QObject(parent),
	SayonaraClass()
{
	m = Pimpl::make<Private>();

	m->playlist = Playlist::Handler::instance();
	m->sortorder = _settings->get<Set::Lib_Sorting>();

	m->filter.set_mode(Library::Filter::Fulltext);
	m->filter.set_filtertext("", _settings->get<Set::Lib_SearchMode>());

	Tagging::ChangeNotifier* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sig_metadata_changed,
			this, &AbstractLibrary::metadata_id3_changed);
}

AbstractLibrary::~AbstractLibrary() {}

void AbstractLibrary::load()
{
	m->filter.clear();

	get_all_artists(_artists);
	get_all_albums(_albums);
	get_all_tracks(_tracks);

	emit_stuff();

	m->loaded = true;
}

bool AbstractLibrary::is_loaded() const
{
	return m->loaded;
}


void AbstractLibrary::emit_stuff()
{
	_artists.sort(m->sortorder.so_artists);
	_albums.sort(m->sortorder.so_albums);
	_tracks.sort(m->sortorder.so_tracks);

	emit sig_all_albums_loaded();
	emit sig_all_artists_loaded();
	emit sig_all_tracks_loaded();
}

void AbstractLibrary::refetch()
{
	m->selected_albums.clear();
	m->selected_artists.clear();
	m->selected_tracks.clear();
	m->filter.clear();

	_albums.clear();
	_artists.clear();
	_tracks.clear();

	get_all_tracks(_tracks);
	get_all_albums(_albums);
	get_all_artists(_artists);

	emit_stuff();
}


void AbstractLibrary::refresh()
{
	/* Waring! Sorting after each fetch is important here! */
	/* Do not call emit_stuff() in order to avoid double sorting */
	IndexSet sel_artists_idx, sel_albums_idx, sel_tracks_idx;

	IdSet sel_artists = m->selected_artists;
	IdSet sel_albums = m->selected_albums;
	IdSet sel_tracks = m->selected_tracks;

	fetch_by_filter(m->filter, true);

	_artists.sort(m->sortorder.so_artists);
	for(int i=0; i<_artists.count(); i++){
		if(sel_artists.contains(_artists[i].id)) {
			sel_artists_idx.insert(i);
		}
	}

	change_artist_selection(sel_artists_idx);

	_albums.sort(m->sortorder.so_albums);
	for(int i=0; i<_albums.count(); i++){
		if(sel_albums.contains(_albums[i].id)) {
			sel_albums_idx.insert(i);
		}
	}

	change_album_selection(sel_albums_idx);

	_tracks.sort(m->sortorder.so_tracks);
	for(int i=0; i<_tracks.count(); i++)
	{
		if(sel_tracks.contains(_tracks[i].id)) {
			sel_tracks_idx.insert(i);
		}
	}

	emit sig_all_albums_loaded();
	emit sig_all_artists_loaded();
	emit sig_all_tracks_loaded();

	if(sel_tracks_idx.size() > 0)
	{
		change_track_selection(sel_tracks_idx);
	}
}


void AbstractLibrary::prepare_fetched_tracks_for_playlist(bool new_playlist)
{
	if(!new_playlist) {
		m->playlist->create_playlist( tracks() );
	}

	else {
		m->playlist->create_playlist( tracks(),
									  m->playlist->request_new_playlist_name());
	}

	set_playlist_action_after_double_click();
}

void AbstractLibrary::prepare_current_tracks_for_playlist(bool new_playlist)
{
	if(!new_playlist) {
		m->playlist->create_playlist( current_tracks() );
	}

	else {
		m->playlist->create_playlist( current_tracks(),
									  m->playlist->request_new_playlist_name());
	}

	set_playlist_action_after_double_click();
}

void AbstractLibrary::prepare_tracks_for_playlist(const QStringList& paths, bool new_playlist)
{
	if(!new_playlist) {
		m->playlist->create_playlist(paths);
	}

	else {
		m->playlist->create_playlist(paths, m->playlist->request_new_playlist_name());
	}

	set_playlist_action_after_double_click();
}


void AbstractLibrary::set_playlist_action_after_double_click()
{
	PlayManagerPtr play_manager = PlayManager::instance();

	if(_settings->get<Set::Lib_DC_DoNothing>()){
		return;
	}

	else if(_settings->get<Set::Lib_DC_PlayIfStopped>())
	{
		if(play_manager->playstate() != PlayState::Playing){
			m->playlist->change_track(0, m->playlist->current_index());
		}
	}

	else {
		m->playlist->change_track(0, m->playlist->current_index());
	}
}


void AbstractLibrary::play_next_fetched_tracks()
{
	m->playlist->play_next(tracks());
}

void AbstractLibrary::play_next_current_tracks()
{
	m->playlist->play_next( current_tracks() );
}


void AbstractLibrary::append_fetched_tracks()
{
	m->playlist->append_tracks(tracks(), m->playlist->current_index());
}

void AbstractLibrary::append_current_tracks()
{
	m->playlist->append_tracks(current_tracks(), m->playlist->current_index());
}

void AbstractLibrary::change_artist_selection(const IndexSet& indexes)
{
	SP::Set<ArtistId> selected_artists;
	for(auto it=indexes.begin(); it!=indexes.end(); it++)
	{
		int idx = *it;
		const Artist& artist = _artists[idx];
		selected_artists.insert(artist.id);
	}

	if(selected_artists == m->selected_artists)
	{
		return;
	}

	_albums.clear();
	_tracks.clear();

	m->selected_artists = selected_artists;

	if(m->selected_artists.size() > 0) {
		get_all_tracks_by_artist(m->selected_artists.toList(), _tracks, m->filter);
		get_all_albums_by_artist(m->selected_artists.toList(), _albums, m->filter);
	}

	else if(!m->filter.cleared()) {
		get_all_tracks_by_searchstring(m->filter, _tracks);
		get_all_albums_by_searchstring(m->filter, _albums);
		get_all_artists_by_searchstring(m->filter, _artists);
	}

	else{
		get_all_tracks(_tracks);
		get_all_albums(_albums);
	}

	_tracks.sort(m->sortorder.so_tracks);
	_albums.sort(m->sortorder.so_albums);
	_artists.sort(m->sortorder.so_artists);
}

const MetaDataList& AbstractLibrary::tracks() const
{
	return _tracks;
}

const AlbumList& AbstractLibrary::albums() const
{
	return _albums;
}

const ArtistList& AbstractLibrary::artists() const
{
	return _artists;
}

const MetaDataList& AbstractLibrary::current_tracks() const
{
	if(m->selected_tracks.isEmpty()){
		return _tracks;
	}

	return m->current_tracks;
}

const IdSet& AbstractLibrary::selected_tracks() const
{
	return m->selected_tracks;
}

const IdSet& AbstractLibrary::selected_albums() const
{
	return m->selected_albums;
}

const IdSet& AbstractLibrary::selected_artists() const
{
	return m->selected_artists;
}


Library::Filter AbstractLibrary::filter() const
{
	return m->filter;
}

void AbstractLibrary::change_filter(Library::Filter filter, bool force)
{
	QStringList filtertext = filter.filtertext(false);

	if(filtertext.join("").size() < 3){
		filter.clear();
	}

	else
	{
		Library::SearchModeMask mask = _settings->get<Set::Lib_SearchMode>();
		filter.set_filtertext(filtertext.join(","), mask);
	}

	if(filter == m->filter){
		return;
	}

	fetch_by_filter(filter, force);
	emit_stuff();
}

void AbstractLibrary::selected_artists_changed(const IndexSet& indexes)
{
	change_artist_selection(indexes);

	emit sig_all_albums_loaded();
	emit sig_all_tracks_loaded();
}


void AbstractLibrary::change_album_selection(const IndexSet& indexes, bool ignore_artists)
{
	SP::Set<AlbumId> selected_albums;
	bool show_album_artists = _settings->get<Set::Lib_ShowAlbumArtists>();

	for(auto it=indexes.begin(); it != indexes.end(); it++)
	{
		int idx = *it;
		if(idx >= _albums.count()){
			break;
		}

		const Album& album = _albums[idx];
		selected_albums.insert(album.id);
	}

	/*if(selected_albums == m->selected_albums) {
		return;
	}*/

	_tracks.clear();
	m->selected_albums = selected_albums;

	// only show tracks of selected album / artist
	if(m->selected_artists.size() > 0 && !ignore_artists)
	{
		if(m->selected_albums.size() > 0)
		{
			MetaDataList v_md;

			get_all_tracks_by_album(m->selected_albums.toList(), v_md, m->filter);

			// filter by artist

			for(const MetaData& md : v_md) {
				ArtistId artist_id;
				if(show_album_artists){
					artist_id = md.album_artist_id();
				}

				else{
					artist_id = md.artist_id;
				}

				if(m->selected_artists.contains(artist_id)){
					_tracks << std::move(md);
				}
			}
		}

		else{
			get_all_tracks_by_artist(m->selected_artists.toList(), _tracks, m->filter);
		}
	}

	// only album is selected
	else if(m->selected_albums.size() > 0) {
		get_all_tracks_by_album(m->selected_albums.toList(), _tracks, m->filter);
	}

	// neither album nor artist, but searchstring
	else if(!m->filter.cleared()) {
		get_all_tracks_by_searchstring(m->filter, _tracks);
	}

	// no album, no artist, no searchstring
	else{
		get_all_tracks(_tracks);
	}

	_tracks.sort(m->sortorder.so_tracks);
}


void AbstractLibrary::selected_albums_changed(const IndexSet& indexes, bool ignore_artists)
{
	change_album_selection(indexes, ignore_artists);
	emit sig_all_tracks_loaded();
}


void AbstractLibrary::change_track_selection(const IndexSet& indexes)
{
	m->selected_tracks.clear();
	m->current_tracks.clear();

	for(int idx : indexes)
	{
		if(idx < 0 || idx >= _tracks.count()){
			continue;
		}

		const MetaData& md = _tracks[idx];

		m->current_tracks << md;
		m->selected_tracks.insert(md.id);
	}
}


void AbstractLibrary::selected_tracks_changed(const IndexSet& indexes)
{
	change_track_selection(indexes);
}


void AbstractLibrary::fetch_by_filter(Library::Filter filter, bool force)
{
	if( (m->filter == filter) &&
		(m->selected_artists.empty()) &&
		(m->selected_albums.empty()) &&
		!force)
	{
		return;
	}

	m->filter = filter;

	_albums.clear();
	_artists.clear();
	_tracks.clear();

	m->selected_albums.clear();
	m->selected_artists.clear();

	if(m->filter.cleared()) {
		get_all_artists(_artists);
		get_all_albums(_albums);
		get_all_tracks(_tracks);
	}

	else {
		get_all_artists_by_searchstring(m->filter, _artists);
		get_all_albums_by_searchstring(m->filter, _albums);
		get_all_tracks_by_searchstring(m->filter, _tracks);
	}
}

void AbstractLibrary::fetch_tracks_by_paths(const QStringList& paths)
{
	_tracks.clear();

	MetaDataList tracks;
	get_all_tracks(tracks);

	for(const MetaData& md : tracks)
	{
		for(const QString& path : paths)
		{
			if(md.filepath().startsWith(path))
			{
				_tracks << md;
			}
		}
	}

	emit_stuff();
}


void AbstractLibrary::change_track_rating(int idx, Rating rating)
{
	_tracks[idx].rating = rating;
	update_track(_tracks[idx]);
}


void AbstractLibrary::change_album_rating(int idx, Rating rating)
{
	_albums[idx].rating = rating;
	update_album(_albums[idx]);
}

void AbstractLibrary::change_track_sortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_tracks){
		return;
	}

	Library::Sortings so = _settings->get<Set::Lib_Sorting>();
	so.so_tracks = s;
	_settings->set<Set::Lib_Sorting>(so);
	m->sortorder = so;

	_tracks.sort(s);

	emit sig_all_tracks_loaded();
}

void AbstractLibrary::change_album_sortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_albums){
		return;
	}

	Library::Sortings so = _settings->get<Set::Lib_Sorting>();
	so.so_albums = s;
	_settings->set<Set::Lib_Sorting>(so);

	m->sortorder = so;

	_albums.sort(s);

	emit sig_all_albums_loaded();
}

void AbstractLibrary::change_artist_sortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_artists){
		return;
	}

	Library::Sortings so = _settings->get<Set::Lib_Sorting>();
	so.so_artists = s;
	_settings->set<Set::Lib_Sorting>(so);

	m->sortorder = so;

	_artists.sort(s);

	emit sig_all_artists_loaded();
}


void AbstractLibrary::metadata_id3_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	Q_UNUSED(v_md_old)
	Q_UNUSED(v_md_new)

	refresh();
}

void AbstractLibrary::update_tracks(const MetaDataList& v_md)
{
	for(const MetaData& md : v_md){
		update_track(md);
	}

	refresh();
}

Library::Sortings AbstractLibrary::sortorder() const
{
	return m->sortorder;
}

Tagging::Editor* AbstractLibrary::tag_edit()
{
	if(!m->tag_edit){
		m->tag_edit = new Tagging::Editor(this);
		connect(m->tag_edit, &Tagging::Editor::finished, this, &AbstractLibrary::refresh);
		connect(m->tag_edit, &Tagging::Editor::sig_progress, this, [=](int progress){
			emit sig_reloading_library(Lang::get(Lang::ReloadLibrary), progress);
		});
	}

	return m->tag_edit;
}

void AbstractLibrary::insert_tracks(const MetaDataList &v_md)
{
	Q_UNUSED(v_md)
	refresh();
}

void AbstractLibrary::import_files(const QStringList &files)
{
	Q_UNUSED(files)
}


void AbstractLibrary::delete_current_tracks(Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None) return;
	delete_tracks( current_tracks(), mode);
}


void AbstractLibrary::delete_fetched_tracks(Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None) return;
	delete_tracks( tracks(), mode);
}

void AbstractLibrary::delete_all_tracks()
{
	MetaDataList v_md;
	get_all_tracks(v_md);
	delete_tracks(v_md, Library::TrackDeletionMode::OnlyLibrary);
}


void AbstractLibrary::delete_tracks(const MetaDataList& v_md, Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None) {
		return;
	}

	QString file_entry = Lang::get(Lang::Entries);
	QString answer_str;

	int n_fails = 0;
	if(mode == Library::TrackDeletionMode::AlsoFiles)
	{
		file_entry = Lang::get(Lang::Files);

		for( const MetaData& md : v_md )
		{
			QFile f(md.filepath());
			if(!f.remove()){
				n_fails++;
			}
		}
	}

	if(n_fails == 0) {
		// all entries could be removed
		answer_str = tr("All %1 could be removed").arg(file_entry);
	}

	else {
		// 5 of 20 entries could not be removed
		answer_str = tr("%1 of %2 %3 could not be removed")
				.arg(n_fails)
				.arg(v_md.size())
				.arg(file_entry);
	}

	emit sig_delete_answer(answer_str);
	Tagging::ChangeNotifier::instance()->delete_metadata(v_md);

	refresh();
}


void AbstractLibrary::delete_tracks_by_idx(const IndexSet& indexes, Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None) return;

	MetaDataList v_md, v_md_old, v_md_changed;
	for(auto it = indexes.begin(); it != indexes.end(); it++){
		int idx = *it;
		v_md.push_back(_tracks[idx]);
	}

	delete_tracks(v_md, mode);
}


void AbstractLibrary::add_genre(SP::Set<Id> ids, const Genre& genre)
{
	MetaDataList v_md;
	get_all_tracks(v_md);

	tag_edit()->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		if( ids.contains(v_md[i].id) ){
			tag_edit()->add_genre(i, genre);
		}
	}

	tag_edit()->commit();
}


void AbstractLibrary::delete_genre(const Genre& genre)
{
	sp_log(Log::Debug, this) << "Delete genre: Fetch all tracks";
	MetaDataList v_md, v_md_with_genre;
	get_all_tracks(v_md);

	for(auto it=v_md.begin(); it != v_md.end(); it++)
	{
		if(it->genre_ids().contains(genre.id()))
		{
			v_md_with_genre << std::move(*it);
		}
	}

	v_md.clear();

	sp_log(Log::Debug, this) << "Delete genre: Set Metadata";
	tag_edit()->set_metadata(v_md_with_genre);

	for(int i=0; i<v_md_with_genre.count(); i++)
	{
		tag_edit()->delete_genre(i, genre);
	}

	tag_edit()->commit();
}

void AbstractLibrary::rename_genre(const Genre& genre, const Genre& new_genre)
{
	MetaDataList v_md;

	sp_log(Log::Debug, this) << "Rename genre: Fetch all tracks";
	get_all_tracks(v_md);
	tag_edit()->set_metadata(v_md);

	for(int i=0; i<v_md.count(); i++)
	{
		if(v_md[i].has_genre(genre)){
			tag_edit()->delete_genre(i, genre);
			tag_edit()->add_genre(i, new_genre);
		}
	}

	tag_edit()->commit();
}

void AbstractLibrary::merge_artists(const IdSet& source_ids, ArtistId target_id)
{
	Q_UNUSED(source_ids)
	Q_UNUSED(target_id)
}

void AbstractLibrary::merge_albums(const IdSet& source_ids, AlbumId target_id)
{
	Q_UNUSED(source_ids)
	Q_UNUSED(target_id)
}
