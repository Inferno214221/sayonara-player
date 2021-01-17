/* Playlist.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
 *      Author: Michael Lugmair (Lucio Carreras)
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
#include "Utils/Playlist/PlaylistMode.h"
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
	PlayManager*			playManager=nullptr;
	PlaylistChangeNotifier*	pcn=nullptr;
	PlaylistCollection		playlists;
	int						activePlaylistIndex;
	int						currentPlaylistIndex;

	Private() :
		playManager(PlayManagerProvider::instance()->playManager()),
		pcn(PlaylistChangeNotifier::instance()),
		activePlaylistIndex(-1),
		currentPlaylistIndex(-1)
	{}
};

Handler::Handler(QObject* parent) :
	QObject(parent)
{
	qRegisterMetaType<PlaylistPtr>("PlaylistPtr");
	qRegisterMetaType<PlaylistConstPtr>("PlaylistConstPtr");

	m = Pimpl::make<Private>();

	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &Handler::playstateChanged);
	connect(m->playManager, &PlayManager::sigNext, this, &Handler::next);
	connect(m->playManager, &PlayManager::sigWakeup, this, &Handler::wakeUp);
	connect(m->playManager, &PlayManager::sigPrevious, this, &Handler::previous);
	connect(m->playManager, &PlayManager::sigStreamFinished, this, &Handler::wwwTrackFinished);

	connect(m->pcn, &PlaylistChangeNotifier::sigPlaylistRenamed, this, &Handler::playlistRenamed);
	connect(m->pcn, &PlaylistChangeNotifier::sigPlaylistDeleted, this, &Handler::playlistDeleted);
}

Handler::~Handler()	= default;

void Handler::shutdown()
{
	if(GetSetting(Set::PL_LoadTemporaryPlaylists))
	{
		DB::Connector::instance()->transaction();

		for(const PlaylistPtr& pl : Algorithm::AsConst(m->playlists))
		{
			if(pl->isTemporary() && pl->wasChanged())
			{
				pl->save();
			}
		}

		DB::Connector::instance()->commit();
	}

	m->playlists.clear();
}

void Handler::currentTrackChanged(int track_index)
{
	Q_UNUSED(track_index)

	PlaylistPtr pl = activePlaylist();

	MetaData md;
	bool success = pl->currentTrack(md);

	if(!success)
	{
		playlistStopped();
		return;
	}

	SetSetting(Set::PL_LastPlaylist, pl->id());

	m->playManager->changeCurrentTrack(md, track_index);

	emit sigCurrentTrackChanged( track_index,	pl->index() );
}

void Handler::playlistStopped()
{
	if(m->playManager->playstate() != PlayState::Stopped)
	{
		m->playManager->stop();
	}
}

int Handler::loadOldPlaylists()
{
	spLog(Log::Debug, this) << "Loading playlists...";

	Loader loader;
	loader.createPlaylists();

	int lastTrackIndex = -1;
	int lastPlaylistIndex = std::max(loader.getLastPlaylistIndex(), 0);

	setActiveIndex(lastPlaylistIndex);
	set_current_index(lastPlaylistIndex);

	if(activePlaylist()->count() > 0){
		lastTrackIndex = loader.getLastTrackIndex();
	}

	if(lastTrackIndex >= 0) {
		changeTrack(lastTrackIndex, lastPlaylistIndex);
	}

	else {
		m->playManager->stop();
	}

	return m->playlists.size();
}


PlaylistPtr Handler::newPlaylist(QString name)
{
	int index = m->playlists.count();
	return PlaylistPtr(new ::Playlist::Playlist(index, name, m->playManager));
}


int Handler::addNewPlaylist(const QString& name, bool temporary)
{
	int idx = exists(name);
	if(idx >= 0) {
		return idx;
	}

	PlaylistPtr pl = newPlaylist(name);
	pl->setTemporary(temporary);

	if(m->playlists.isEmpty()){
		m->activePlaylistIndex = 0;
		m->currentPlaylistIndex = 0;

		emit sigActivePlaylistChanged(m->activePlaylistIndex);
	}

	m->playlists.push_back(pl);

	emit sigNewPlaylistAdded(pl);

	connect(pl.get(), &Playlist::Playlist::sigCurrentTrackChanged, this, &Handler::currentTrackChanged);
	connect(pl.get(), &Playlist::Playlist::sigStopped, this, &Handler::playlistStopped);
	connect(pl.get(), &Playlist::Playlist::sigFindTrack, this, &Handler::sigFindTrackRequested);

	return pl->index();
}

// create a playlist, where metadata is already available
int Handler::createPlaylist(const MetaDataList& v_md, const QString& name, bool temporary)
{
	int idx = exists(name);
	if(idx == -1)
	{
		idx = addNewPlaylist(name, temporary);
		PlaylistPtr tmp_pl = m->playlists[idx];
		tmp_pl->insertTemporaryIntoDatabase();
	}

	PlaylistPtr pl = m->playlists[idx];
	if(pl->isBusy()) {
		return idx;
	}

	pl->createPlaylist(v_md);
	pl->setTemporary( pl->isTemporary() && temporary );

	set_current_index(idx);

	return idx;
}

// create a new playlist, where only filepaths are given
// Load Folder, Load File...
int Handler::createPlaylist(const QStringList& paths, const QString& name, bool temporary)
{
	int index = createPlaylist(MetaDataList(), name, temporary);
	createFilescanner(index, paths, -1);
	return index;
}

int Handler::createPlaylist(const QString& dir, const QString& name, bool temporary)
{
	return createPlaylist(QStringList{dir}, name, temporary);
}

int Handler::createPlaylist(const CustomPlaylist& cpl)
{
	auto it = Algorithm::find(m->playlists, [&cpl](const PlaylistPtr& pl){
		return (pl->id() == cpl.id());
	});

	int idx;
	if(it == m->playlists.end()){
		idx = addNewPlaylist(cpl.name(), cpl.temporary());
	}

	else{
		idx = (*it)->index();
	}

	PlaylistPtr pl = m->playlists[idx];
	pl->createPlaylist(cpl);
	pl->setChanged(false);

	return pl->index();
}

int Handler::createEmptyPlaylist(bool override_current)
{
	QString name;
	if(!override_current){
		name = requestNewPlaylistName();
	}

	return createPlaylist(MetaDataList(), name, true);
}

int Handler::createEmptyPlaylist(const QString& name)
{
	return createPlaylist(MetaDataList(), name, true);
}

void Handler::clearPlaylist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->clear();
}


void Handler::playstateChanged(PlayState state)
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
	activePlaylist()->play();
}

void Handler::stopped()
{
	m->activePlaylistIndex = -1;
	emit sigActivePlaylistChanged(m->activePlaylistIndex);

	for(PlaylistPtr pl : m->playlists){
		pl->stop();
	}
}

void Handler::next()
{
	activePlaylist()->next();
}

void Handler::wakeUp()
{
	bool restore_track_after_stop = GetSetting(Set::PL_RememberTrackAfterStop);

	if(restore_track_after_stop)
	{
		if(activePlaylist()->wakeUp()){
			return;
		}
	}

	next();
}


void Handler::previous()
{
	if( m->playManager->currentPositionMs() > 2000) {
		m->playManager->seekAbsoluteMs(0);
	}

	else {
		activePlaylist()->bwd();
	}
}


void Handler::changeTrack(int trackIdx, int playlist_idx)
{
	if( !Util::between(playlist_idx, m->playlists) )
	{
		playlist_idx = activePlaylist()->index();
	}

	if( playlist_idx != m->activePlaylistIndex )
	{
		activePlaylist()->stop();

		setActiveIndex(playlist_idx);
	}

	activePlaylist()->changeTrack(trackIdx);
}

int	Handler::activeIndex() const
{
	return m->activePlaylistIndex;
}

void Handler::setActiveIndex(int idx)
{
	int old_active_index = m->activePlaylistIndex;

	if(m->playlists.isEmpty()) {
		m->activePlaylistIndex = idx;
	}

	else if(Util::between(idx, m->playlists)) {
		m->activePlaylistIndex = idx;
	}

	else {
		m->activePlaylistIndex = activePlaylist()->index();
	}

	SetSetting(Set::PL_LastPlaylist, activePlaylist()->id());

	if(old_active_index != m->activePlaylistIndex)
	{
		emit sigActivePlaylistChanged(m->activePlaylistIndex);
	}
}


PlaylistPtr Handler::activePlaylist()
{
	int old_active_index = m->activePlaylistIndex;
	if(m->playManager->playstate() == PlayState::Stopped) {
		m->activePlaylistIndex = -1;
	}

	// assure we have at least one playlist
	if(m->playlists.size() == 0) {
		m->activePlaylistIndex = createEmptyPlaylist();
	}

	// assure valid idx
	if( !Util::between(m->activePlaylistIndex, m->playlists) )
	{
		if(Util::between(m->currentPlaylistIndex, m->playlists)){
			m->activePlaylistIndex = m->currentPlaylistIndex;
		}

		else {
			m->activePlaylistIndex = 0;
		}
	}

	if(old_active_index != m->activePlaylistIndex)
	{
		emit sigActivePlaylistChanged(m->activePlaylistIndex);
	}

	return m->playlists[m->activePlaylistIndex];
}


PlaylistConstPtr Handler::activePlaylist() const
{
	return playlist(activeIndex());
}

int Handler::current_index() const
{
	return m->currentPlaylistIndex;
}

void Handler::set_current_index(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	if(pl_idx == m->currentPlaylistIndex) {
		return;
	}

	m->currentPlaylistIndex = pl_idx;

	emit sigCurrentPlaylistChanged(pl_idx);
}

int Handler::count() const
{
	return m->playlists.size();
}


void Handler::playNext(const MetaDataList& v_md)
{
	PlaylistPtr pl = activePlaylist();
	insertTracks(v_md, pl->currentTrackIndex() + 1, pl->index());
}

void Handler::playNext(const QStringList& paths)
{
	PlaylistPtr pl = activePlaylist();
	insertTracks(paths, pl->currentTrackIndex() + 1, pl->index());
}

void Handler::insertTracks(const MetaDataList& v_md, int row, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	if(pl->isBusy()) {
		return;
	}

	bool is_empty = (pl->count() == 0);
	bool stopped = (m->playManager->playstate() == PlayState::Stopped);
	bool play_if_stopped = GetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty);

	pl->insertTracks(v_md, row);

	if(is_empty && stopped && play_if_stopped)
	{
		this->changeTrack(0, pl_idx);
	}
}

void Handler::insertTracks(const QStringList& paths, int row, int pl_idx)
{
	createFilescanner(pl_idx, paths, row);
}


void Handler::appendTracks(const MetaDataList& v_md, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	if(!pl->isBusy()) 
	{
		pl->appendTracks(v_md);
	}
}

void Handler::appendTracks(const QStringList& paths, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	createFilescanner(pl_idx, paths, m->playlists.at(pl_idx)->count());
}

void Handler::removeRows(const IndexSet& indexes, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->removeTracks(indexes);
}

void Handler::moveRows(const IndexSet& indexes, int tgt_idx, int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)
	m->playlists[pl_idx]->moveTracks(indexes, tgt_idx);
}


QString Handler::requestNewPlaylistName(const QString& prefix) const
{
	return DBInterface::requestNewDatabaseName(prefix);
}

int Handler::closePlaylist(int pl_idx)
{
	CHECK_IDX_RET(pl_idx, m->playlists.count())

	bool was_active = (pl_idx == m->activePlaylistIndex);

	if(m->playlists[pl_idx]->isTemporary()) {
		m->playlists[pl_idx]->deletePlaylist();
	}

	m->playlists.removeAt(pl_idx);

	if(was_active)
	{
		setActiveIndex(m->playlists.isEmpty() ? -1 : 0);
	}

	else if(m->activePlaylistIndex > pl_idx) {
		m->activePlaylistIndex --;
		emit sigActivePlaylistChanged(m->activePlaylistIndex);
	}

	for(PlaylistPtr pl : m->playlists)
	{
		if(pl->index() >= pl_idx && pl->index() > 0) {
			pl->setIndex(pl->index() - 1);
		}
	}

	if(was_active)
	{
		SetSetting(Set::PL_LastPlaylist, -1);
		SetSetting(Set::PL_LastTrack, -1);
	}

	else {
		SetSetting(Set::PL_LastPlaylist, activePlaylist()->id());
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
	if( name.isEmpty() && Util::between(m->currentPlaylistIndex, m->playlists)) {
		return m->currentPlaylistIndex;
	}

	return Algorithm::indexOf(m->playlists, [&name](PlaylistPtr pl) {
		return (pl->name().compare(name, Qt::CaseInsensitive) == 0);
	});
}


void Handler::savePlaylistToFile(int pl_idx, const QString& filename, bool relative)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	PlaylistParser::saveM3UPlaylist(filename, pl->tracks(), relative);
}


void Handler::resetPlaylist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	if(!pl->isBusy())
	{
		DBWrapper db_connector;
		CustomPlaylist cpl = db_connector.getPlaylistById(pl->id());

		clearPlaylist(pl_idx);
		createPlaylist(cpl);
	}
}

Util::SaveAsAnswer Handler::savePlaylist(int pl_idx)
{
	CHECK_IDX_RET(pl_idx, Util::SaveAsAnswer::OtherError)

	PlaylistPtr pl = m->playlists[pl_idx];

	DB::Connector::instance()->transaction();
	Util::SaveAsAnswer ret = pl->save();
	DB::Connector::instance()->commit();

	if(ret == Util::SaveAsAnswer::Success)
	{
		PlaylistChangeNotifier::instance()->addPlaylist(pl->id(), pl->name());
	}

	return ret;
}


Util::SaveAsAnswer Handler::savePlaylistAs(int pl_idx, const QString& new_name, bool force_override)
{
	CHECK_IDX_RET(pl_idx, Util::SaveAsAnswer::OtherError)

	PlaylistPtr pl = m->playlists[pl_idx];
	Util::SaveAsAnswer ret = pl->saveAs(new_name, force_override);
	if(ret != Util::SaveAsAnswer::Success) {
		return ret;
	}

	{ // fetch id of new playlist
		auto db_connector = std::make_unique<DBWrapper>();
		CustomPlaylist pl_new = db_connector->getPlaylistByName(new_name);
		if(pl_new.id() >= 0)
		{
			PlaylistChangeNotifier::instance()->addPlaylist(pl_new.id(), new_name);
		}
	}

	emit sigPlaylistNameChanged(pl_idx);

	return Util::SaveAsAnswer::Success;
}


Util::SaveAsAnswer Handler::renamePlaylist(int pl_idx, const QString& new_name)
{
	CHECK_IDX_RET(pl_idx, Util::SaveAsAnswer::OtherError)

	// get playlist we want to save
	PlaylistPtr pl = m->playlists[pl_idx];
	QString old_name = pl->name();

	Util::SaveAsAnswer ret = pl->rename(new_name);
	if(ret == Util::SaveAsAnswer::Success)
	{
		PlaylistChangeNotifier::instance()->renamePlaylist(pl->id(), old_name, new_name);
	}

	return ret;
}

void Handler::playlistRenamed(int id, const QString& old_name, const QString& new_name)
{
	Q_UNUSED(old_name)

	auto it = Algorithm::find(m->playlists, [&id](auto playlist) {
		return (playlist->id() == id);
	});

	if(it == m->playlists.end()) {
		return;
	}

	PlaylistPtr pl = *it;
	pl->setName(new_name);

	emit sigPlaylistNameChanged(pl->index());
}


void Handler::deletePlaylist(int pl_idx)
{
	CHECK_IDX_VOID(pl_idx)

	PlaylistPtr pl = m->playlists[pl_idx];
	int id = pl->id();

	bool success = pl->removeFromDatabase();
	if(success){
		PlaylistChangeNotifier::instance()->deletePlaylist(id);
	}
}

void Handler::playlistDeleted(int id)
{
	auto it = Algorithm::find(m->playlists, [&id](auto playlist){
		return (playlist->id() == id);
	});

	if(it == m->playlists.end()){
		return;
	}

	PlaylistPtr pl = *it;
	pl->setTemporary(true);
}


void Handler::deleteTracks(int pl_idx, const IndexSet& rows, Library::TrackDeletionMode deletion_mode)
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

	emit sigTrackDeletionRequested(v_md, deletion_mode);
}

void Handler::wwwTrackFinished(const MetaData& md)
{
	PlaylistPtr active_pl = activePlaylist();

	if(GetSetting(Set::Stream_ShowHistory))
	{
		active_pl->insertTracks(MetaDataList{md}, active_pl->currentTrackIndex());
	}
}

struct MetaDataScannerData
{
	int playlist_id;
	int target_row_index;
};

void Handler::createFilescanner(int playlist_index, const QStringList& paths, int target_row_idx)
{
	CHECK_IDX_VOID(playlist_index)

	PlaylistPtr playlist = m->playlists.at(playlist_index);
	if(playlist->isBusy()){
		return;
	}

	playlist->setBusy(true);

	using Directory::MetaDataScanner;

	auto* t = new QThread();
	auto* worker = new MetaDataScanner(paths, true, nullptr);
	auto* data = new MetaDataScannerData{playlist->id(), target_row_idx};

	worker->setData(data);

	connect(t, &QThread::started, worker, &MetaDataScanner::start);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(worker, &MetaDataScanner::sigFinished, this, &Handler::filesScanned);
	connect(worker, &MetaDataScanner::sigCurrentProcessedPathChanged, this, &Handler::filescannerProgressChanged);
	connect(worker, &MetaDataScanner::sigFinished, t, &QThread::quit);

	worker->moveToThread(t);
	t->start();
}

void Handler::filesScanned()
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());
	auto* data = static_cast<MetaDataScannerData*>(worker->data());

	for (PlaylistPtr pl : m->playlists)
	{
		if(pl->id() != data->playlist_id) {
			continue;
		}

		pl->setBusy(false);
		if(worker->metadata().isEmpty()){
			continue;
		}

		int target_row_index = data->target_row_index;
		if(target_row_index < 0)
		{
			pl->clear();
			insertTracks(worker->metadata(), 0, pl->index());
		}

		else if(target_row_index >= pl->count())
		{
			appendTracks(worker->metadata(), pl->index());
		}

		else
		{
			insertTracks(worker->metadata(), target_row_index, pl->index());
		}
	}

	worker->setData(nullptr);
	worker->deleteLater();
	delete data;
}

void Handler::filescannerProgressChanged(const QString& current_file)
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());
	auto* data = static_cast<MetaDataScannerData*>(worker->data());

	for (PlaylistPtr pl : m->playlists)
	{
		if(pl->id() != data->playlist_id)
		{
			continue;
		}

		pl->setCurrentScannedFile(current_file);
	}
}

void Playlist::Handler::applyPlaylistActionAfterDoubleClick()
{
	PlayState playState = m->playManager->playstate();
	::Playlist::Mode plm = GetSetting(Set::PL_Mode);

	bool append = (plm.append() == ::Playlist::Mode::State::On);

	if(GetSetting(Set::Lib_DC_DoNothing))
	{
		return;
	}

	else if(GetSetting(Set::Lib_DC_PlayIfStopped))
	{
		if(playState != PlayState::Playing)
		{
			this->changeTrack(0, this->current_index());
		}
	}

	else if(GetSetting(Set::Lib_DC_PlayImmediately) && !append)
	{
		this->changeTrack(0, this->current_index());
	}
}
