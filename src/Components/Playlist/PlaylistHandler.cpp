/* Playlist.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 *
 *  Created on: Apr 6, 2011
 *      Author: Lucio Carreras
 */

#include "PlaylistHandler.h"
#include "StdPlaylist.h"
#include "StreamPlaylist.h"
#include "PlaylistLoader.h"
#include "PlaylistDBWrapper.h"

#include "Components/Directories/DirectoryReader.h"
#include "Components/PlayManager/PlayManager.h"
#include "Database/Connector.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"

#include <algorithm>
#include <memory>

#define CHECK_IDX_VOID(idx) if(!between(idx, m->playlists)){ return; }
#define CHECK_IDX_RET(idx, ret) if(!between(idx, m->playlists)){ return ret; }

using PlaylistCollection=QList<PlaylistPtr>;
using Playlist::Handler;
using Playlist::DBInterface;
using Playlist::DBWrapper;

struct Handler::Private
{
	DB::Connector*			db=nullptr;
	PlayManagerPtr			play_manager=nullptr;
	PlaylistCollection		playlists;
	int						active_playlist_idx;
	int						current_playlist_idx;

	Private() :
		db(DB::Connector::instance()),
		play_manager(PlayManager::instance()),
		active_playlist_idx(-1),
		current_playlist_idx(-1)
	{}
};

Handler::Handler(QObject * parent) :
	QObject(parent)
{
	qRegisterMetaType<PlaylistPtr>("PlaylistPtr");
	qRegisterMetaType<PlaylistConstPtr>("PlaylistConstPtr");

	m = Pimpl::make<Private>();

	connect(m->play_manager, &PlayManager::sig_playstate_changed, this, &Handler::playstate_changed);
	connect(m->play_manager, &PlayManager::sig_next, this, &Handler::next);
	connect(m->play_manager, &PlayManager::sig_wake_up, this, &Handler::wake_up);
	connect(m->play_manager, &PlayManager::sig_previous, this, &Handler::previous);
	connect(m->play_manager, &PlayManager::sig_www_track_finished, this, &Handler::www_track_finished);
}

Handler::~Handler()	{}

void Handler::emit_cur_track_changed()
{
	PlaylistPtr pl = active_playlist();

	MetaData md;
	bool success = pl->current_track(md);
	int cur_track_idx = pl->current_track_index();

	if(!success || cur_track_idx == -1)
	{
		m->play_manager->stop();
		return;
	}

	SetSetting(Set::PL_LastPlaylist, pl->get_id());

	m->play_manager->change_track(md, cur_track_idx);

	emit sig_current_track_changed( cur_track_idx,	pl->index() );
}


int Handler::load_old_playlists()
{
	sp_log(Log::Debug, this) << "Loading playlists...";

	Playlist::Loader loader;
	loader.create_playlists();

	int last_track_idx = -1;
	int last_playlist_idx = std::max(loader.get_last_playlist_idx(), 0);

	set_active_idx(last_playlist_idx);
	set_current_index(last_playlist_idx);

	if(active_playlist()->count() > 0){
		last_track_idx = loader.get_last_track_idx();
	}

	if(last_track_idx >= 0) {
		change_track(last_track_idx, last_playlist_idx);
	}

	else {
		m->play_manager->stop();
		return m->playlists.size();
	}

	if(GetSetting(Set::PL_StartPlaying)){
		m->play_manager->play();
	}

	else{
		m->play_manager->pause();
	}

	return m->playlists.size();
}


PlaylistPtr Handler::new_playlist(Playlist::Type type, int playlist_idx, QString name)
{
	if(type == Playlist::Type::Stream) {
		return PlaylistPtr(new Playlist::Stream(playlist_idx, name));
	}

	return PlaylistPtr(new Playlist::Standard(playlist_idx, name));
}


int Handler::add_new_playlist(const QString& name, bool temporary, Playlist::Type type)
{
	int idx = exists(name);
	if(idx >= 0) {
		return idx;
	}

	PlaylistPtr pl = new_playlist(type, m->playlists.size(), name);
	pl->set_temporary(temporary);

	if(m->playlists.isEmpty()){
		m->active_playlist_idx = 0;
		m->current_playlist_idx = 0;
	}

	m->playlists.push_back(pl);

	emit sig_new_playlist_added(pl);

	return pl->index();
}

// create a playlist, where metadata is already available
int Handler::create_playlist(const MetaDataList& v_md, const QString& name, bool temporary, Playlist::Type type)
{
	int idx = exists(name);
	if(idx == -1)
	{
		idx = add_new_playlist(name, temporary, type);
		PlaylistPtr tmp_pl = m->playlists[idx];
		tmp_pl->insert_temporary_into_db();
	}

	PlaylistPtr pl = m->playlists[idx];

	pl->create_playlist(v_md);
	pl->set_temporary( pl->is_temporary() && temporary );

	set_current_index(idx);

	return idx;
}

// create a new playlist, where only filepaths are given
// Load Folder, Load File...
int Handler::create_playlist(const QStringList& pathlist, const QString& name, bool temporary, Playlist::Type type)
{
	DirectoryReader reader;

	MetaDataList v_md = reader.scan_metadata(pathlist);
	v_md.sort(Library::SortOrder::TrackAlbumArtistAsc);

	return create_playlist(v_md, name, temporary, type);
}

int Handler::create_playlist(const QString& dir, const QString& name, bool temporary, Playlist::Type type)
{
	return create_playlist(QStringList{dir}, name, temporary, type);
}


int Handler::create_playlist(const CustomPlaylist& cpl)
{
	int idx;
	int id = cpl.id();
	auto it = Util::find(m->playlists, [id](const PlaylistPtr& pl){
		return (pl->get_id() == id);
	});

	if(it == m->playlists.end()){
		idx = add_new_playlist(cpl.name(), cpl.temporary(), Playlist::Type::Std);
	}

	else{
		idx = (*it)->index();
	}

	PlaylistPtr pl = m->playlists[idx];
	pl->create_playlist(cpl);
	pl->set_changed(false);

	return pl->index();
}

int Handler::create_empty_playlist(bool override_current)
{
	QString name;
	if(!override_current){
		name = request_new_playlist_name();
	}

	return create_playlist(MetaDataList(), name, true);
}

int Handler::create_empty_playlist(const QString& name)
{
	return create_playlist(MetaDataList(), name, true);
}

void Handler::shutdown()
{
	if(GetSetting(Set::PL_LoadTemporaryPlaylists))
	{
		m->db->transaction();

		for(const PlaylistPtr& pl : Util::AsConst(m->playlists))
		{
			if(pl->is_temporary() && pl->was_changed() && pl->is_storable())
			{
				pl->save();
			}
		}

		m->db->commit();
	}

	m->playlists.clear();
}

void Handler::clear_playlist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->clear();
}


void Handler::playstate_changed(PlayState state)
{
	switch(state)
	{
		case PlayState::Playing:
			played(); break;
		case PlayState::Paused:
			paused(); break;
		case PlayState::Stopped:
			stopped(); break;
		default: return;
	}
}

void Handler::played()
{
	active_playlist()->play();
}

void Handler::paused()
{
	active_playlist()->pause();
}

void Handler::stopped()
{
	m->active_playlist_idx = -1;

	for(PlaylistPtr pl : m->playlists){
		pl->stop();
	}
}

void Handler::next()
{
	active_playlist()->next();
	emit_cur_track_changed();
}

void Handler::wake_up()
{
	bool restore_track_after_stop = GetSetting(Set::PL_RememberTrackAfterStop);

	if(restore_track_after_stop)
	{
		if(active_playlist()->wake_up()){
			emit_cur_track_changed();
			return;
		}
	}

	next();
}


void Handler::previous()
{
	if( m->play_manager->current_position_ms() > 2000) {
		m->play_manager->seek_abs_ms(0);
	}

	else {
		active_playlist()->bwd();
		emit_cur_track_changed();
	}
}


void Handler::change_track(int track_idx, int playlist_idx)
{
	if( !between(playlist_idx, m->playlists) ) {
		playlist_idx = active_playlist()->index();
	}

	if( playlist_idx != m->active_playlist_idx )
	{
		active_playlist()->stop();

		set_active_idx(playlist_idx);
	}

	bool track_changed = active_playlist()->change_track(track_idx);
	if(track_changed) {
		emit_cur_track_changed();
	}

	else {
		m->play_manager->stop();
	}
}


int	Handler::active_index() const
{
	return m->active_playlist_idx;
}

void Handler::set_active_idx(int idx)
{
	if(m->playlists.isEmpty()){
		m->active_playlist_idx = idx;
	}

	else if(between(idx, m->playlists)){
		m->active_playlist_idx = idx;
	}

	else {
		m->active_playlist_idx = active_playlist()->index();
	}

	SetSetting(Set::PL_LastPlaylist, active_playlist()->get_id());
}


PlaylistPtr Handler::active_playlist()
{
	if(m->play_manager->playstate() == PlayState::Stopped){
		m->active_playlist_idx = -1;
	}

	// assure we have at least one playlist
	if(m->playlists.size() == 0){
		m->active_playlist_idx = create_empty_playlist();
	}

	// assure valid idx
	if( !between(m->active_playlist_idx, m->playlists) )
	{
		if(between(m->current_playlist_idx, m->playlists)){
			m->active_playlist_idx = m->current_playlist_idx;
		}

		else {
			m->active_playlist_idx = 0;
		}
	}

	return m->playlists[m->active_playlist_idx];
}


PlaylistConstPtr Handler::active_playlist() const
{
	return playlist(active_index());
}

int Handler::current_index() const
{
	return m->current_playlist_idx;
}

void Handler::set_current_index(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	if(pl_idx == m->current_playlist_idx){
		return;
	}

	m->current_playlist_idx = pl_idx;

	emit sig_current_playlist_changed(pl_idx);
}


void Handler::play_next(const MetaDataList& v_md)
{
	PlaylistPtr active = active_playlist();

	active->insert_tracks(v_md, active->current_track_index() + 1);
}


void Handler::insert_tracks(const MetaDataList& v_md, int row, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];

	bool is_empty = pl->is_empty();
	bool stopped = (m->play_manager->playstate() == PlayState::Stopped);
	bool play_if_stopped = GetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty);

	pl->insert_tracks(v_md, row);

	if(is_empty && stopped && play_if_stopped)
	{
		this->change_track(0, pl_idx);
	}
}


void Handler::append_tracks(const MetaDataList& v_md, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->append_tracks(v_md);
}

void Handler::remove_rows(const IndexSet& indexes, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->remove_tracks(indexes);
}


void Handler::move_rows(const IndexSet& indexes, int tgt_idx, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->move_tracks(indexes, tgt_idx);
}


QString Handler::request_new_playlist_name() const
{
	return DBInterface::request_new_db_name();
}

int Handler::close_playlist(int pl_idx)
{
	CHECK_IDX_RET(pl_idx, m->playlists.count());

	bool was_active = (pl_idx == m->active_playlist_idx);

	if(m->playlists[pl_idx]->is_temporary()){
		m->playlists[pl_idx]->delete_playlist();
	}

	m->playlists.removeAt(pl_idx);

	if(was_active)
	{
		set_active_idx(m->playlists.isEmpty() ? -1 : 0);
	}

	else if(m->active_playlist_idx > pl_idx){
		m->active_playlist_idx --;
	}

	for(PlaylistPtr pl : m->playlists)
	{
		if(pl->index() >= pl_idx && pl->index() > 0) {
			pl->set_index(pl->index() - 1);
		}
	}

	if(was_active)
	{
		SetSetting(Set::PL_LastPlaylist, -1);
		SetSetting(Set::PL_LastTrack, -1);
	}

	else{
		SetSetting(Set::PL_LastPlaylist, active_playlist()->get_id());
	}

	return m->playlists.count();
}

PlaylistConstPtr Handler::playlist(int idx) const
{
	CHECK_IDX_RET(idx, nullptr)

	return std::const_pointer_cast<const Base>(m->playlists[idx]);
}

PlaylistPtr Handler::playlist(int idx, PlaylistPtr fallback) const
{
	CHECK_IDX_RET(idx, fallback)

	return m->playlists[idx];
}

int Handler::exists(const QString& name) const
{
	if( name.isEmpty() && between(m->current_playlist_idx, m->playlists)) {
		return m->current_playlist_idx;
	}

	return Util::indexOf(m->playlists, [&name](PlaylistPtr pl){
		return (pl->get_name().compare(name, Qt::CaseInsensitive) == 0);
	});
}


void Handler::save_playlist_to_file(int pl_idx, const QString& filename, bool relative)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	PlaylistParser::save_m3u_playlist(filename, pl->playlist(), relative);
}


void Handler::reset_playlist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	DBWrapper* db_connector = new DBWrapper();

	int id = m->playlists[pl_idx]->get_id();

	CustomPlaylist cpl = db_connector->get_playlist_by_id(id);

	clear_playlist(pl_idx);
	create_playlist(cpl);

	delete db_connector; db_connector = nullptr;
}


DBInterface::SaveAsAnswer Handler::save_playlist(int pl_idx)
{
	CHECK_IDX_RET(pl_idx, DBInterface::SaveAsAnswer::Error)

	PlaylistPtr pl = m->playlists[pl_idx];

	m->db->transaction();
	DBInterface::SaveAsAnswer ret = pl->save();
	m->db->commit();

	if(!pl->is_temporary()){
		emit sig_saved_playlists_changed();
	}

	return ret;
}


DBInterface::SaveAsAnswer Handler::save_playlist_as(int pl_idx, const QString& name, bool force_override)
{
	CHECK_IDX_RET(pl_idx, DBInterface::SaveAsAnswer::Error)

	PlaylistPtr pl = m->playlists[pl_idx];

	// no empty playlists
	if(name.isEmpty())
	{
		return DBInterface::SaveAsAnswer::Error;
	}

	DBInterface::SaveAsAnswer ret = pl->save_as(name, force_override);
	if(ret != DBInterface::SaveAsAnswer::Success)
	{
		return ret;
	}

	if(!pl->is_temporary()){
		emit sig_saved_playlists_changed();
	}

	emit sig_playlist_name_changed(pl_idx);

	return DBInterface::SaveAsAnswer::Success;
}


DBInterface::SaveAsAnswer Handler::rename_playlist(int pl_idx, const QString& name)
{
	CHECK_IDX_RET(pl_idx, DBInterface::SaveAsAnswer::Error)

	// no empty playlists
	if(name.isEmpty()){
		return DBInterface::SaveAsAnswer::Error;
	}

	// get playlist we want to save
	PlaylistPtr pl = m->playlists[pl_idx];

	DBInterface::SaveAsAnswer ret = pl->rename(name);
	if(ret != DBInterface::SaveAsAnswer::Success){
		return ret;
	}

	emit sig_playlist_name_changed(pl_idx);

	// for PlaylistChooser
	if(!pl->is_temporary()){
		emit sig_saved_playlists_changed();
	}

	return DBInterface::SaveAsAnswer::Success;
}

void Handler::delete_playlist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];

	bool was_temporary = pl->is_temporary();
	bool success = pl->remove_from_db();

	// for PlaylistChooser
	if(success && !was_temporary){
		emit sig_saved_playlists_changed();
	}
}

void Handler::delete_tracks(int pl_idx, const IndexSet& rows, Library::TrackDeletionMode deletion_mode)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	const MetaDataList& tracks = pl->playlist();

	MetaDataList v_md;
	v_md.reserve(tracks.size());

	for(int i : rows)
	{
		if(i >= 0 && i < tracks.count())
		{
			v_md << tracks[i];
		}
	}

	if(v_md.isEmpty()){
		return;
	}

	emit sig_track_deletion_requested(v_md, deletion_mode);
}

void Handler::www_track_finished(const MetaData& md)
{
	PlaylistPtr active_pl = active_playlist();

	if(GetSetting(Set::Stream_ShowHistory))
	{
		active_pl->insert_track(md, active_pl->current_track_index());
	}
}
