/* Playlist.cpp */

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

#include "Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Logger/Logger.h"

namespace File=Util::File;
namespace Algorithm=Util::Algorithm;

using PlaylistImpl=::Playlist::Playlist;

struct PlaylistImpl::Private
{
	MetaDataList    v_md;
	QList<uint64_t>	shuffle_history;
	UniqueId		playing_id;
	PlaylistMode	playlist_mode;
	int				playlist_idx;
	PlaylistType	type;
	bool			playlist_changed;
	bool			busy;

	Private(int playlist_idx, PlaylistMode playlist_mode, PlaylistType type) :
		playlist_mode(playlist_mode),
		playlist_idx(playlist_idx),
		type(type),
		playlist_changed(false),
		busy(false)
	{}
};


PlaylistImpl::Playlist(int idx, PlaylistType type, const QString& name) :
	Playlist::DBInterface(name)
{
	m = Pimpl::make<::Playlist::Playlist::Private>(idx,  GetSetting(Set::PL_Mode), type);

	Tagging::ChangeNotifier* md_change_notifier = Tagging::ChangeNotifier::instance();
	connect(md_change_notifier, &Tagging::ChangeNotifier::sig_metadata_changed, this, &Playlist::metadata_changed);
	connect(md_change_notifier, &Tagging::ChangeNotifier::sig_metadata_deleted, this, &Playlist::metadata_deleted);

	PlayManager* play_manager = PlayManager::instance();
	connect(play_manager, &PlayManager::sig_track_metadata_changed, this, &Playlist::metadata_changed_single);
	connect(play_manager, &PlayManager::sig_duration_changed, this, &Playlist::duration_changed);

	ListenSetting(Set::PL_Mode, Playlist::setting_playlist_mode_changed);
}

PlaylistImpl::~Playlist() = default;

void PlaylistImpl::clear()
{
	if(!m->v_md.isEmpty())
	{
		m->v_md.clear();
		set_changed(true);
	}
}

IndexSet PlaylistImpl::move_tracks(const IndexSet& indexes, int tgt_row)
{
	m->v_md.move_tracks(indexes, tgt_row);

	int n_lines_before_tgt = Algorithm::count_if(indexes, [&tgt_row](int sel){
		return (sel < tgt_row);
	});

	IndexSet new_track_positions;
	for(int i=tgt_row; i<tgt_row + indexes.count(); i++) {
		new_track_positions.insert(i - n_lines_before_tgt);
	}

	set_changed(true);

	return new_track_positions;
}

IndexSet PlaylistImpl::copy_tracks(const IndexSet& indexes, int tgt)
{
	m->v_md.copy_tracks(indexes, tgt);

	set_changed(true);

	IndexSet new_track_positions;
	for(int i=0; i<indexes.count(); i++)
	{
		new_track_positions << tgt + i;
	}

	set_changed(true);

	return new_track_positions;
}

void Playlist::Playlist::find_track(int idx)
{
	if(Util::between(idx, m->v_md)){
		emit sig_find_track(m->v_md[idx].id);
	}
}

void PlaylistImpl::remove_tracks(const IndexSet& indexes)
{
	m->v_md.remove_tracks(indexes);

	set_changed(true);
}


void PlaylistImpl::insert_tracks(const MetaDataList& lst, int tgt)
{
	m->v_md.insert_tracks(lst, tgt);
	set_changed(true);
}

void PlaylistImpl::append_tracks(const MetaDataList& lst)
{
	int old_size = m->v_md.count();

	m->v_md << lst;

	for(auto it=m->v_md.begin() + old_size; it != m->v_md.end(); it++)
	{
		it->is_disabled = !(File::check_file(it->filepath()));
	}

	set_changed(true);
}

bool PlaylistImpl::change_track(int idx)
{
	set_track_idx_before_stop(-1);
	m->v_md.set_current_track(idx);

	if( !Util::between(idx, m->v_md) )
	{
		stop();
		set_track_idx_before_stop(-1);
		return false;
	}

	emit sig_current_track_changed(idx);

	m->shuffle_history << m->v_md[idx].unique_id();
	sp_log(Log::Info, this) << m->shuffle_history;

	if( !Util::File::check_file(m->v_md[idx].filepath()) )
	{
		sp_log(Log::Warning, this) << QString("Track %1 not available on file system: ").arg(m->v_md[idx].filepath());
		m->v_md[idx].is_disabled = true;

		return change_track(idx + 1);
	}

	return true;
}

void PlaylistImpl::metadata_deleted()
{
	IndexSet indexes;
	auto* mdcn = Tagging::ChangeNotifier::instance();
	MetaDataList v_md_deleted = mdcn->deleted_metadata();

	int i=0;
	for(const MetaData& md : m->v_md)
	{
		bool contains = Algorithm::contains(v_md_deleted, [&md](const MetaData& md_tmp){
			return (md.is_equal(md_tmp));
		});

		if(contains){
			indexes.insert(i);
		}

		i++;
	}

	m->v_md.remove_tracks(indexes);
	emit sig_items_changed( index() );
}

void PlaylistImpl::metadata_changed()
{
	auto* mdcn = Tagging::ChangeNotifier::instance();
	auto changed_metadata = mdcn->changed_metadata();

	const MetaDataList& v_md_new = changed_metadata.second;

	for(auto it=m->v_md.begin(); it !=m->v_md.end(); it++)
	{
		auto tmp_it = Algorithm::find(v_md_new, [it](const MetaData& md){
			return it->is_equal(md);
		});

		if(tmp_it != v_md_new.end()) {
			*it = *tmp_it;
		}
	}

	emit sig_items_changed( index() );
}

void PlaylistImpl::metadata_changed_single()
{
	MetaData md = PlayManager::instance()->current_track();
	IdxList idx_list = m->v_md.findTracks(md.filepath());

	for(int i : idx_list) {
		replace_track(i, md);
	}
}

void PlaylistImpl::duration_changed()
{
	PlayManager* pm = PlayManager::instance();
	MilliSeconds duration = pm->duration_ms();

	MetaDataList& v_md = m->v_md;

	int cur_track = v_md.current_track();
	if(!Util::between(cur_track, v_md.count())){
		return;
	}

	IdxList idx_list = m->v_md.findTracks(v_md[cur_track].filepath());

	for(int i : idx_list)
	{
		MetaData changed_md(v_md[i]);
		changed_md.set_duration_ms(std::max<MilliSeconds>(0, duration));

		replace_track(i, changed_md);
	}
}


void PlaylistImpl::replace_track(int idx, const MetaData& md)
{
	if( !Util::between(idx, m->v_md) ) {
		return;
	}

	bool is_playing = m->v_md[idx].pl_playing;
//	if(m->playing_id == m->v_md[idx].unique_id())
//	{
//		m->playing_id = md.unique_id();
//	}

	m->v_md[idx] = md;
	m->v_md[idx].is_disabled = !(File::check_file(md.filepath()));
	m->v_md[idx].pl_playing = is_playing;

	emit sig_items_changed( index() );
}

void PlaylistImpl::play()
{
	if(m->v_md.current_track() < 0){
		change_track(0);
	}
}

void PlaylistImpl::stop()
{
	m->shuffle_history.clear();

	if(current_track_index() >= 0)
	{
		set_track_idx_before_stop(current_track_index());
		m->v_md.set_current_track(-1);
	}

	emit sig_stopped();
}

void PlaylistImpl::fwd()
{
	PlaylistMode cur_mode = m->playlist_mode;
	PlaylistMode mode_bak = m->playlist_mode;

	cur_mode.setRep1(false);
	set_mode(cur_mode);

	next();

	set_mode(mode_bak);
}

void PlaylistImpl::bwd()
{
	if(PlaylistMode::isActiveAndEnabled(m->playlist_mode.shuffle()))
	{
		for(int history_index = m->shuffle_history.size() - 2; history_index >= 0; history_index--)
		{
			UniqueId id = m->shuffle_history[history_index];

			int idx = Util::Algorithm::indexOf(m->v_md, [id](const MetaData& md){
				return (id == md.unique_id());
			});

			if(Util::between(idx, m->v_md))
			{
				m->shuffle_history.erase(m->shuffle_history.begin() + history_index, m->shuffle_history.end());

				change_track(idx);
				return;
			}
		}
	}

	m->shuffle_history.clear();
	change_track( current_track_index() - 1 );
}

void PlaylistImpl::next()
{
	// no track
	if(m->v_md.isEmpty() )
	{
		stop();
		set_track_idx_before_stop(-1);
		return;
	}

	// stopped
	int cur_track = m->v_md.current_track();
	int track_num = -1;

	if(cur_track == -1){
		track_num = 0;
	}

	// play it again
	else if(PlaylistMode::isActiveAndEnabled(m->playlist_mode.rep1())){
		track_num = cur_track;
	}

	// shuffle mode
	else if(PlaylistMode::isActiveAndEnabled(m->playlist_mode.shuffle()))
	{
		track_num = calc_shuffle_track();
	}

	// normal track
	else
	{
		// last track
		if(cur_track == m->v_md.count() - 1)
		{
			track_num = -1;

			if(PlaylistMode::isActiveAndEnabled(m->playlist_mode.repAll())){
				track_num = 0;
			}
		}

		else {
			track_num = cur_track + 1;
		}
	}

	change_track(track_num);
}

#include "Utils/Utils.h"
int PlaylistImpl::calc_shuffle_track()
{
	if(m->v_md.size() <= 1){
		return -1;
	}

	// check all tracks played
	int i=0;
	QList<int> unplayed_tracks;
	for(MetaData& md : m->v_md)
	{
		UniqueId unique_id = md.unique_id();
		if(!m->shuffle_history.contains(unique_id)) {
			unplayed_tracks << i;
		}

		i++;
	}

	// no random track to play
	if(unplayed_tracks.isEmpty())
	{
		if(PlaylistMode::isActiveAndEnabled(m->playlist_mode.repAll()) == false)
		{
			return -1;
		}

		m->shuffle_history.clear();
		return Util::random_number(0, int(m->v_md.size() - 1));
	}

	else
	{
		RandomGenerator rnd;
		int left_tracks_idx = rnd.get_number(0, unplayed_tracks.size() - 1);

		return unplayed_tracks[left_tracks_idx];
	}
}


bool PlaylistImpl::wake_up()
{
	int idx = track_idx_before_stop();

	if(Util::between(idx, count()))
	{
		return change_track(idx);
	}

	return false;
}

void Playlist::Playlist::set_busy(bool busy)
{
	m->busy = busy;

	emit sig_busy_changed(busy);
}

bool Playlist::Playlist::is_busy() const
{
	return m->busy;
}

void Playlist::Playlist::set_current_scanned_file(const QString& current_scanned_file)
{
	emit sig_current_scanned_file_changed(current_scanned_file);
}

void PlaylistImpl::enable_all()
{
	for(MetaData& md : m->v_md)
	{
		md.is_disabled = false;
	}

	set_changed(true);
}

int PlaylistImpl::create_playlist(const MetaDataList& v_md)
{
	if(PlaylistMode::isActiveAndEnabled(m->playlist_mode.append()) == false)
	{
		m->v_md.clear();
		m->shuffle_history.clear();
	}

	m->v_md << v_md;

	set_changed(true);

	return m->v_md.count();
}

int PlaylistImpl::index() const
{
	return m->playlist_idx;
}

void PlaylistImpl::set_index(int idx)
{
	m->playlist_idx = idx;
}

void PlaylistImpl::set_mode(const PlaylistMode& mode)
{
	if( m->playlist_mode.shuffle() != mode.shuffle())
	{
		m->shuffle_history.clear();
	}

	m->playlist_mode = mode;
}

PlaylistMode PlaylistImpl::mode() const
{
	return m->playlist_mode;
}

MilliSeconds PlaylistImpl::running_time() const
{
	MilliSeconds dur_ms  = std::accumulate(m->v_md.begin(), m->v_md.end(), 0, [](MilliSeconds time, const MetaData& md){
		return time + md.duration_ms();
	});

	return dur_ms;
}

int PlaylistImpl::current_track_index() const
{
	return m->v_md.current_track();
}

bool PlaylistImpl::current_track(MetaData& md) const
{
	int cur_play_idx = m->v_md.current_track();
	if(cur_play_idx < 0){
		return false;
	}

	md = m->v_md[cur_play_idx];
	return true;
}

int PlaylistImpl::count() const
{
	return m->v_md.count();
}

void PlaylistImpl::set_changed(bool b)
{
	restore_track_before_stop();

	m->playlist_changed = b;

	emit sig_items_changed(m->playlist_idx);
}

bool PlaylistImpl::was_changed() const
{
	return m->playlist_changed;
}

bool PlaylistImpl::is_storable() const
{
	return (m->type == PlaylistType::Std);
}

void PlaylistImpl::setting_playlist_mode_changed()
{
	set_mode( GetSetting(Set::PL_Mode) );
}

MetaDataList PlaylistImpl::tracks() const
{
	return m->v_md;
}

MetaData PlaylistImpl::track(int idx) const
{
	if(idx >= 0 && idx < m->v_md.count())
	{
		return m->v_md[idx];
	}

	return MetaData();
}
