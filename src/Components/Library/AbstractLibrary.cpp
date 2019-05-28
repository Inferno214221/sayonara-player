/* AbstractLibrary.cpp */

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

#include "AbstractLibrary.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Set.h"

#include <QHash>

struct AbstractLibrary::Private
{
	Util::Set<ArtistId>	selected_artists;
	Util::Set<AlbumId>	selected_albums;
	Util::Set<TrackID>	selected_tracks;

	ArtistList			artists;
	AlbumList			albums;
	MetaDataList		tracks;
	MetaDataList		current_tracks;
	MetaDataList		filtered_tracks;

	ExtensionSet		extensions;

	Playlist::Handler*	playlist=nullptr;

	Library::Sortings	sortorder;
	Library::Filter		filter;
	bool				loaded;

	Private()
	{
		loaded = false;
	}
};

AbstractLibrary::AbstractLibrary(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	m->playlist = Playlist::Handler::instance();
	m->sortorder = GetSetting(Set::Lib_Sorting);

	m->filter.set_mode(Library::Filter::Fulltext);
	m->filter.set_filtertext("", GetSetting(Set::Lib_SearchMode));

	Tagging::ChangeNotifier* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sig_metadata_changed,
			this, &AbstractLibrary::metadata_id3_changed);
}

AbstractLibrary::~AbstractLibrary() {}

void AbstractLibrary::load()
{
	m->filter.clear();

	refetch();

	m->loaded = true;
}

bool AbstractLibrary::is_loaded() const
{
	return m->loaded;
}

void AbstractLibrary::emit_stuff()
{
	prepare_artists();
	prepare_albums();
	prepare_tracks();

	emit sig_all_artists_loaded();
	emit sig_all_albums_loaded();
	emit sig_all_tracks_loaded();
}

void AbstractLibrary::refetch()
{
	m->selected_artists.clear();
	m->selected_albums.clear();
	m->selected_tracks.clear();
	m->filter.clear();

	m->artists.clear();
	m->albums.clear();
	m->tracks.clear();

	get_all_artists(m->artists);
	get_all_albums(m->albums);
	get_all_tracks(m->tracks);

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

	prepare_artists();
	for(int i=0; i<m->artists.count(); i++)
	{
		if(sel_artists.contains(m->artists[i].id)) {
			sel_artists_idx.insert(i);
		}
	}

	change_artist_selection(sel_artists_idx);
	prepare_albums();

	for(int i=0; i<m->albums.count(); i++)
	{
		if(sel_albums.contains(m->albums[i].id)) {
			sel_albums_idx.insert(i);
		}
	}

	change_album_selection(sel_albums_idx);

	prepare_tracks();

	const MetaDataList& v_md = this->tracks();
	for(int i=0; i<v_md.count(); i++)
	{
		if(sel_tracks.contains(v_md[i].id)) {
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
	Playlist::Mode plm = GetSetting(Set::PL_Mode);

	bool append = (plm.append() == Playlist::Mode::State::On);

	if(GetSetting(Set::Lib_DC_DoNothing))
	{
		return;
	}

	else if(GetSetting(Set::Lib_DC_PlayIfStopped))
	{
		if(play_manager->playstate() != PlayState::Playing)
		{
			m->playlist->change_track(0, m->playlist->current_index());
		}
	}

	else if(GetSetting(Set::Lib_DC_PlayImmediately) && !append)
	{
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
	Util::Set<ArtistId> selected_artists;
	for(auto it=indexes.begin(); it!=indexes.end(); it++)
	{
		int idx = *it;
		const Artist& artist = m->artists[idx];
		selected_artists.insert(artist.id);
	}

	if(selected_artists == m->selected_artists)
	{
		return;
	}

	m->albums.clear();
	m->tracks.clear();

	m->selected_artists = selected_artists;

	if(m->selected_artists.size() > 0) {
		get_all_tracks_by_artist(m->selected_artists.toList(), m->tracks, m->filter);
		get_all_albums_by_artist(m->selected_artists.toList(), m->albums, m->filter);
	}

	else if(!m->filter.cleared()) {
		get_all_tracks_by_searchstring(m->filter, m->tracks);
		get_all_albums_by_searchstring(m->filter, m->albums);
		get_all_artists_by_searchstring(m->filter, m->artists);
	}

	else{
		get_all_tracks(m->tracks);
		get_all_albums(m->albums);
	}

	prepare_artists();
	prepare_albums();
	prepare_tracks();
}


const MetaDataList& AbstractLibrary::tracks() const
{
	if(m->filtered_tracks.isEmpty())
	{
		return m->tracks;
	}

	return m->filtered_tracks;
}

const AlbumList& AbstractLibrary::albums() const
{
	return m->albums;
}

const ArtistList& AbstractLibrary::artists() const
{
	return m->artists;
}

const MetaDataList& AbstractLibrary::current_tracks() const
{
	if(m->selected_tracks.isEmpty()){
		return tracks();
	}

	return m->current_tracks;
}

void AbstractLibrary::change_current_disc(Disc disc)
{
	if( m->selected_albums.size() != 1 )
	{
		return;
	}

	get_all_tracks_by_album(m->selected_albums.toList(), m->tracks, m->filter);

	if(disc != std::numeric_limits<Disc>::max())
	{
		m->tracks.remove_tracks([disc](const MetaData& md)
		{
			return (md.discnumber != disc);
		});
	}

	prepare_tracks();
	emit sig_all_tracks_loaded();
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

	if(!filter.is_invalid_genre())
	{
		if( filtertext.join("").size() < 3){
			filter.clear();
		}

		else {
			Library::SearchModeMask mask = GetSetting(Set::Lib_SearchMode);
			filter.set_filtertext(filtertext.join(","), mask);
		}
	}

	else {
		// get everything which has no genre attached to it
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
	Util::Set<AlbumId> selected_albums;
	bool show_album_artists = GetSetting(Set::Lib_ShowAlbumArtists);

	for(auto it=indexes.begin(); it != indexes.end(); it++)
	{
		int idx = *it;
		if(idx >= m->albums.count()){
			break;
		}

		const Album& album = m->albums[idx];
		selected_albums.insert(album.id);
	}

	m->tracks.clear();
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
					m->tracks << std::move(md);
				}
			}
		}

		else{
			get_all_tracks_by_artist(m->selected_artists.toList(), m->tracks, m->filter);
		}
	}

	// only album is selected
	else if(m->selected_albums.size() > 0) {
		get_all_tracks_by_album(m->selected_albums.toList(), m->tracks, m->filter);
	}

	// neither album nor artist, but searchstring
	else if(!m->filter.cleared()) {
		get_all_tracks_by_searchstring(m->filter, m->tracks);
	}

	// no album, no artist, no searchstring
	else{
		get_all_tracks(m->tracks);
	}

	prepare_tracks();
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
		if(idx < 0 || idx >= tracks().count()){
			continue;
		}

		const MetaData& md = tracks()[idx];

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

	m->artists.clear();
	m->albums.clear();
	m->tracks.clear();

	m->selected_artists.clear();
	m->selected_albums.clear();

	if(m->filter.cleared())
	{
		get_all_artists(m->artists);
		get_all_albums(m->albums);
		get_all_tracks(m->tracks);
	}

	else {
		get_all_artists_by_searchstring(m->filter, m->artists);
		get_all_albums_by_searchstring(m->filter, m->albums);
		get_all_tracks_by_searchstring(m->filter, m->tracks);
	}
}

void AbstractLibrary::fetch_tracks_by_paths(const QStringList& paths)
{
	m->tracks.clear();

	MetaDataList tracks;
	get_all_tracks(tracks);

	for(const MetaData& md : tracks)
	{
		for(const QString& path : paths)
		{
			if(md.filepath().startsWith(path))
			{
				m->tracks << md;
			}
		}
	}

	emit_stuff();
}

void AbstractLibrary::change_album_rating(int idx, Rating rating)
{
	m->albums[idx].rating = rating;
	update_album(m->albums[idx]);
}

void AbstractLibrary::change_track_sortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_tracks){
		return;
	}

	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	so.so_tracks = s;
	SetSetting(Set::Lib_Sorting, so);
	m->sortorder = so;

	prepare_tracks();
	emit sig_all_tracks_loaded();
}

void AbstractLibrary::change_album_sortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_albums){
		return;
	}

	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	so.so_albums = s;
	SetSetting(Set::Lib_Sorting, so);

	m->sortorder = so;

	prepare_albums();
	emit sig_all_albums_loaded();
}

void AbstractLibrary::change_artist_sortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_artists){
		return;
	}

	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	so.so_artists = s;
	SetSetting(Set::Lib_Sorting, so);

	m->sortorder = so;

	prepare_artists();
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
	if(mode == Library::TrackDeletionMode::None || indexes.isEmpty()) {
		return;
	}

	MetaDataList v_md;
	const MetaDataList& tracks = this->tracks();
	for(auto it = indexes.begin(); it != indexes.end(); it++) {
		v_md.push_back(tracks[*it]);
	}

	delete_tracks(v_md, mode);
}

void AbstractLibrary::prepare_tracks()
{
	m->extensions.clear();
	m->filtered_tracks.clear();

	for(const MetaData& md : tracks())
	{
		m->extensions.add_extension(Util::File::get_file_extension(md.filepath()), false);
	}

	m->tracks.sort(m->sortorder.so_tracks);
}

void AbstractLibrary::prepare_albums()
{
	m->albums.sort(m->sortorder.so_albums);
}

void AbstractLibrary::prepare_artists()
{
	m->artists.sort(m->sortorder.so_artists);
}

ExtensionSet AbstractLibrary::extensions() const
{
	return m->extensions;
}

void AbstractLibrary::set_extensions(const ExtensionSet& extensions)
{
	m->extensions = extensions;
	m->filtered_tracks.clear();

	if(m->extensions.has_enabled_extensions())
	{
		for(const MetaData& md : m->tracks)
		{
			QString ext = ::Util::File::get_file_extension(md.filepath());
			if(m->extensions.is_enabled(ext)){
				m->filtered_tracks << md;
			}
		}
	}

	emit sig_all_tracks_loaded();
}

