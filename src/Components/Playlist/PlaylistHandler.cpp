/* Playlist.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
#include "Playlist.h"
#include "PlaylistLoader.h"
#include "PlaylistDBWrapper.h"
#include "PlaylistChangeNotifier.h"

#include "Components/Directories/MetaDataScanner.h"
#include "Components/PlayManager/PlayManager.h"
#include "Database/Connector.h"

#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QThread>

namespace Algorithm=Util::Algorithm;

#define CHECK_IDX_VOID(idx) if(!Util::between(idx, m->playlists)){ return; }
#define CHECK_IDX_RET(idx, ret) if(!Util::between(idx, m->playlists)){ return ret; }

using PlaylistCollection=QList<PlaylistPtr>;
using Playlist::Handler;
using Playlist::DBInterface;
using Playlist::DBWrapper;
using Playlist::Loader;

struct Handler::Private
{
	DB::Connector*			db=nullptr;
	PlayManagerPtr			play_manager=nullptr;
	PlaylistChangeNotifier*	pcn=nullptr;
	PlaylistCollection		playlists;
	int						active_playlist_idx;
	int						current_playlist_idx;

	Private() :
		db(DB::Connector::instance()),
		play_manager(PlayManager::instance()),
		pcn(PlaylistChangeNotifier::instance()),
		active_playlist_idx(-1),
		current_playlist_idx(-1)
	{}
};

Handler::Handler(QObject* parent) :
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

	connect(m->pcn, &PlaylistChangeNotifier::sig_playlist_renamed, this, &Handler::playlist_renamed);
	connect(m->pcn, &PlaylistChangeNotifier::sig_playlist_deleted, this, &Handler::playlist_deleted);
}

Handler::~Handler()	= default;


void Handler::current_track_changed(int track_index)
{
	Q_UNUSED(track_index)

	PlaylistPtr pl = active_playlist();

	MetaData md;
	bool success = pl->current_track(md);

	if(!success)
	{
		playlist_stopped();
		return;
	}

	SetSetting(Set::PL_LastPlaylist, pl->get_id());

	m->play_manager->change_track(md, track_index);

	emit sig_current_track_changed( track_index,	pl->index() );
}

void Handler::playlist_stopped()
{
	if(m->play_manager->playstate() != PlayState::Stopped)
	{
		m->play_manager->stop();
	}
}

int Handler::load_old_playlists()
{
	sp_log(Log::Debug, this) << "Loading playlists...";

	Loader loader;
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
	}

	return m->playlists.size();
}


PlaylistPtr Handler::new_playlist(PlaylistType type, QString name)
{
	int index = m->playlists.count();
	if(type == PlaylistType::Stream)
	{
		return PlaylistPtr(new ::Playlist::Playlist(index, PlaylistType::Stream, name));
	}

	return PlaylistPtr(new ::Playlist::Playlist(index, PlaylistType::Std, name));
}


int Handler::add_new_playlist(const QString& name, bool temporary, PlaylistType type)
{
	int idx = exists(name);
	if(idx >= 0) {
		return idx;
	}

	PlaylistPtr pl = new_playlist(type, name);
	pl->set_temporary(temporary);

	if(m->playlists.isEmpty()){
		m->active_playlist_idx = 0;
		m->current_playlist_idx = 0;
	}

	m->playlists.push_back(pl);

	emit sig_new_playlist_added(pl);

	connect(pl.get(), &Playlist::Playlist::sig_current_track_changed, this, &Handler::current_track_changed);
	connect(pl.get(), &Playlist::Playlist::sig_stopped, this, &Handler::playlist_stopped);
	connect(pl.get(), &Playlist::Playlist::sig_find_track, this, &Handler::sig_find_track_requested);

	return pl->index();
}

// create a playlist, where metadata is already available
int Handler::create_playlist(const MetaDataList& v_md, const QString& name, bool temporary, PlaylistType type)
{
	int idx = exists(name);
	if(idx == -1)
	{
		idx = add_new_playlist(name, temporary, type);
		PlaylistPtr tmp_pl = m->playlists[idx];
		tmp_pl->insert_temporary_into_db();
	}

	PlaylistPtr pl = m->playlists[idx];
	if(pl->is_busy()) {
		return idx;
	}

	pl->create_playlist(v_md);
	pl->set_temporary( pl->is_temporary() && temporary );

	set_current_index(idx);

	return idx;
}

// create a new playlist, where only filepaths are given
// Load Folder, Load File...
int Handler::create_playlist(const QStringList& paths, const QString& name, bool temporary, PlaylistType type)
{
	int index = create_playlist(MetaDataList(), name, temporary, type);
	create_filescanner(index, paths, -1);
	return index;
}

int Handler::create_playlist(const QString& dir, const QString& name, bool temporary, PlaylistType type)
{
	return create_playlist(QStringList{dir}, name, temporary, type);
}

int Handler::create_playlist(const CustomPlaylist& cpl)
{
	auto it = Algorithm::find(m->playlists, [&cpl](const PlaylistPtr& pl){
		return (pl->get_id() == cpl.id());
	});

	int idx;
	if(it == m->playlists.end()){
		idx = add_new_playlist(cpl.name(), cpl.temporary(), PlaylistType::Std);
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

		for(const PlaylistPtr& pl : Algorithm::AsConst(m->playlists))
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
	if(state == PlayState::Playing) {
		played();
	}

	else if(state == PlayState::Stopped) {
		stopped();
	}
}

void Handler::played()
{
	active_playlist()->play();
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
}

void Handler::wake_up()
{
	bool restore_track_after_stop = GetSetting(Set::PL_RememberTrackAfterStop);

	if(restore_track_after_stop)
	{
		if(active_playlist()->wake_up()){
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
	}
}


void Handler::change_track(int track_idx, int playlist_idx)
{
	if( !Util::between(playlist_idx, m->playlists) )
	{
		playlist_idx = active_playlist()->index();
	}

	if( playlist_idx != m->active_playlist_idx )
	{
		active_playlist()->stop();

		set_active_idx(playlist_idx);
	}

	active_playlist()->change_track(track_idx);
}

int	Handler::active_index() const
{
	return m->active_playlist_idx;
}

void Handler::set_active_idx(int idx)
{
	if(m->playlists.isEmpty()) {
		m->active_playlist_idx = idx;
	}

	else if(Util::between(idx, m->playlists)) {
		m->active_playlist_idx = idx;
	}

	else {
		m->active_playlist_idx = active_playlist()->index();
	}

	SetSetting(Set::PL_LastPlaylist, active_playlist()->get_id());
}


PlaylistPtr Handler::active_playlist()
{
	if(m->play_manager->playstate() == PlayState::Stopped) {
		m->active_playlist_idx = -1;
	}

	// assure we have at least one playlist
	if(m->playlists.size() == 0) {
		m->active_playlist_idx = create_empty_playlist();
	}

	// assure valid idx
	if( !Util::between(m->active_playlist_idx, m->playlists) )
	{
		if(Util::between(m->current_playlist_idx, m->playlists)){
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

	if(pl_idx == m->current_playlist_idx) {
		return;
	}

	m->current_playlist_idx = pl_idx;

	emit sig_current_playlist_changed(pl_idx);
}

int Handler::count() const
{
	return m->playlists.size();
}


void Handler::play_next(const MetaDataList& v_md)
{
	PlaylistPtr pl = active_playlist();
	insert_tracks(v_md, pl->current_track_index() + 1, pl->index());
}

void Handler::play_next(const QStringList& paths)
{
	PlaylistPtr pl = active_playlist();
	insert_tracks(paths, pl->current_track_index() + 1, pl->index());
}

void Handler::insert_tracks(const MetaDataList& v_md, int row, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	if(pl->is_busy()) {
		return;
	}

	bool is_empty = (pl->count() == 0);
	bool stopped = (m->play_manager->playstate() == PlayState::Stopped);
	bool play_if_stopped = GetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty);

	pl->insert_tracks(v_md, row);

	if(is_empty && stopped && play_if_stopped)
	{
		this->change_track(0, pl_idx);
	}
}

void Handler::insert_tracks(const QStringList& paths, int row, int pl_idx)
{
	create_filescanner(pl_idx, paths, row);
}


void Handler::append_tracks(const MetaDataList& v_md, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	if(!pl->is_busy()) 
	{
		pl->append_tracks(v_md);
	}
}

void Handler::append_tracks(const QStringList& paths, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	create_filescanner(pl_idx, paths, m->playlists.at(pl_idx)->count());
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


QString Handler::request_new_playlist_name(const QString& prefix) const
{
	return DBInterface::request_new_db_name(prefix);
}

int Handler::close_playlist(int pl_idx)
{
	CHECK_IDX_RET(pl_idx, m->playlists.count());

	bool was_active = (pl_idx == m->active_playlist_idx);

	if(m->playlists[pl_idx]->is_temporary()) {
		m->playlists[pl_idx]->delete_playlist();
	}

	m->playlists.removeAt(pl_idx);

	if(was_active)
	{
		set_active_idx(m->playlists.isEmpty() ? -1 : 0);
	}

	else if(m->active_playlist_idx > pl_idx) {
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

	else {
		SetSetting(Set::PL_LastPlaylist, active_playlist()->get_id());
	}

	return m->playlists.count();
}

PlaylistConstPtr Handler::playlist(int idx) const
{
	CHECK_IDX_RET(idx, nullptr)

	return std::const_pointer_cast<const Playlist>(m->playlists[idx]);
}

PlaylistPtr Handler::playlist(int idx, PlaylistPtr fallback) const
{
	CHECK_IDX_RET(idx, fallback)

	return m->playlists[idx];
}

int Handler::exists(const QString& name) const
{
	if( name.isEmpty() && Util::between(m->current_playlist_idx, m->playlists)) {
		return m->current_playlist_idx;
	}

	return Algorithm::indexOf(m->playlists, [&name](PlaylistPtr pl) {
		return (pl->get_name().compare(name, Qt::CaseInsensitive) == 0);
	});
}


void Handler::save_playlist_to_file(int pl_idx, const QString& filename, bool relative)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	PlaylistParser::save_m3u_playlist(filename, pl->tracks(), relative);
}


void Handler::reset_playlist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	if(!pl->is_busy())
	{
		DBWrapper db_connector;
		CustomPlaylist cpl = db_connector.get_playlist_by_id(pl->get_id());

		clear_playlist(pl_idx);
		create_playlist(cpl);
	}
}

Util::SaveAsAnswer Handler::save_playlist(int pl_idx)
{
	CHECK_IDX_RET(pl_idx, Util::SaveAsAnswer::OtherError)

	PlaylistPtr pl = m->playlists[pl_idx];

	m->db->transaction();
	Util::SaveAsAnswer ret = pl->save();
	m->db->commit();

	if(ret == Util::SaveAsAnswer::Success)
	{
		PlaylistChangeNotifier::instance()->add_playlist(pl->get_id(), pl->get_name());
	}

	return ret;
}


Util::SaveAsAnswer Handler::save_playlist_as(int pl_idx, const QString& new_name, bool force_override)
{
	CHECK_IDX_RET(pl_idx, Util::SaveAsAnswer::OtherError)

	PlaylistPtr pl = m->playlists[pl_idx];
	Util::SaveAsAnswer ret = pl->save_as(new_name, force_override);
	if(ret != Util::SaveAsAnswer::Success) {
		return ret;
	}

	{ // fetch id of new playlist
		auto db_connector = std::make_unique<DBWrapper>();
		CustomPlaylist pl_new = db_connector->get_playlist_by_name(new_name);
		if(pl_new.id() >= 0)
		{
			PlaylistChangeNotifier::instance()->add_playlist(pl_new.id(), new_name);
		}
	}

	emit sig_playlist_name_changed(pl_idx);

	return Util::SaveAsAnswer::Success;
}


Util::SaveAsAnswer Handler::rename_playlist(int pl_idx, const QString& new_name)
{
	CHECK_IDX_RET(pl_idx, Util::SaveAsAnswer::OtherError)

	// get playlist we want to save
	PlaylistPtr pl = m->playlists[pl_idx];
	QString old_name = pl->get_name();

	Util::SaveAsAnswer ret = pl->rename(new_name);
	if(ret == Util::SaveAsAnswer::Success)
	{
		PlaylistChangeNotifier::instance()->rename_playlist(pl->get_id(), old_name, new_name);
	}

	return ret;
}

void Handler::playlist_renamed(int id, const QString& old_name, const QString& new_name)
{
	Q_UNUSED(old_name)

	auto it = Algorithm::find(m->playlists, [&id](auto playlist) {
		return (playlist->get_id() == id);
	});

	if(it == m->playlists.end()) {
		return;
	}

	PlaylistPtr pl = *it;
	pl->set_name(new_name);

	emit sig_playlist_name_changed(pl->index());
}


void Handler::delete_playlist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	int id = pl->get_id();

	bool success = pl->remove_from_db();
	if(success){
		PlaylistChangeNotifier::instance()->delete_playlist(id);
	}
}

void Handler::playlist_deleted(int id)
{
	auto it = Algorithm::find(m->playlists, [&id](auto playlist){
		return (playlist->get_id() == id);
	});

	if(it == m->playlists.end()){
		return;
	}

	PlaylistPtr pl = *it;
	pl->set_temporary(true);
}


void Handler::delete_tracks(int pl_idx, const IndexSet& rows, Library::TrackDeletionMode deletion_mode)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];

	MetaDataList v_md;
	v_md.reserve(rows.size());

	for(int i : rows)
	{
		if(Util::between(i, pl->count()))
		{
			v_md << pl->track(i);
		}
	}

	if(v_md.isEmpty()) {
		return;
	}

	emit sig_track_deletion_requested(v_md, deletion_mode);
}

void Handler::www_track_finished(const MetaData& md)
{
	PlaylistPtr active_pl = active_playlist();

	if(GetSetting(Set::Stream_ShowHistory))
	{
		active_pl->insert_tracks(MetaDataList{md}, active_pl->current_track_index());
	}
}

struct MetaDataScannerData
{
	int playlist_id;
	int target_row_index;
};

void Handler::create_filescanner(int playlist_index, const QStringList& paths, int target_row_idx)
{
	CHECK_IDX_VOID(playlist_index)

	PlaylistPtr playlist = m->playlists.at(playlist_index);
	if(playlist->is_busy()){
		return;
	}

	playlist->set_busy(true);

	using Directory::MetaDataScanner;

	auto* t = new QThread();
	auto* worker = new MetaDataScanner(paths, true, nullptr);
	auto* data = new MetaDataScannerData{playlist->get_id(), target_row_idx};

	worker->set_data(data);

	connect(t, &QThread::started, worker, &MetaDataScanner::start);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(worker, &MetaDataScanner::sig_finished, this, &Handler::files_scanned);
	connect(worker, &MetaDataScanner::sig_current_path, this, &Handler::filescanner_progress_changed);
	connect(worker, &MetaDataScanner::sig_finished, t, &QThread::quit);

	worker->moveToThread(t);
	t->start();
}

void Handler::files_scanned()
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());
	auto* data = static_cast<MetaDataScannerData*>(worker->data());

	for (PlaylistPtr pl : m->playlists)
	{
		if(pl->get_id() != data->playlist_id)
		{
			continue;
		}

		pl->set_busy(false);

		int target_row_index = data->target_row_index;
		if(target_row_index < 0)
		{
			pl->clear();
			append_tracks(worker->metadata(), pl->index());
		}

		else if(target_row_index >= pl->count())
		{
			append_tracks(worker->metadata(), pl->index());
		}

		else
		{
			insert_tracks(worker->metadata(), target_row_index, pl->index());
		}
	}

	worker->set_data(nullptr);
	worker->deleteLater();
	delete data;
}

void Handler::filescanner_progress_changed(const QString& current_file)
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());
	auto* data = static_cast<MetaDataScannerData*>(worker->data());

	for (PlaylistPtr pl : m->playlists)
	{
		if(pl->get_id() != data->playlist_id)
		{
			continue;
		}

		pl->set_current_scanned_file(current_file);
	}
}
