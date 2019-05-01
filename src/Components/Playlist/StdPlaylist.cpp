/* StdPlaylist.cpp */

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

#include "StdPlaylist.h"

#include "Utils/globals.h"
#include "Utils/FileUtils.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/RandomGenerator.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"

#include <algorithm>

using Playlist::Standard;

struct Standard::Private
{
	Private() {}
};

Standard::Standard(int idx, const QString& name) :
	Base(idx, name)
{
	m = Pimpl::make<Private>();
	set_storable(true);
}

Standard::~Standard() {}

Playlist::Type Standard::type() const
{
	return Playlist::Type::Std;
}

void Standard::set_changed(bool b)
{
	Base::set_changed(b);
}

int Standard::create_playlist(const MetaDataList& v_md)
{
	if(Playlist::Mode::isActiveAndEnabled(mode().append())){
		metadata() << v_md;
	}

	else{
		metadata() = v_md;
	}

	set_changed(true);

	return metadata().count();
}



bool Standard::change_track(int idx)
{
	set_track_idx_before_stop(-1);

	bool success = Base::change_track(idx);
	if(!success){
		return false;
	}

	metadata(idx).played = true;

	if( !Util::File::check_file(metadata(idx).filepath()) )
	{
		sp_log(Log::Warning, this) << "Track not available on file system: ";
		sp_log(Log::Warning, this) << metadata(idx).filepath();
		metadata(idx).is_disabled = true;

		return change_track(idx + 1);
	}

	return true;
}

bool Standard::wake_up()
{
	int idx = track_idx_before_stop();
	bool success = between(idx, count());

	if(success) {
		success = change_track(idx);
	}

	return success;
}


void Standard::play()
{
	if( metadata().isEmpty() )
	{
		stop();
		set_track_idx_before_stop(-1);
		return;
	}

	if(metadata().current_track() >= 0){
		return;
	}

	int new_track_idx = 0;
	metadata().set_current_track(new_track_idx);
}

void Standard::pause() {}

void Standard::stop()
{
	int cur_track_idx = metadata().current_track();
	if(cur_track_idx >= 0)
	{
		set_track_idx_before_stop(metadata().current_track());
	}

	metadata().set_current_track(-1);

	for(MetaData& md : metadata()){
		md.played = false;
	}
}


void Standard::fwd()
{
	Playlist::Mode cur_mode = mode();
	Playlist::Mode mode_bak = cur_mode;

	cur_mode.setRep1(false);
	set_mode(cur_mode);

	next();

	set_mode(mode_bak);
}


void Standard::bwd()
{
	int cur_idx = metadata().current_track();
	change_track( cur_idx - 1 );
}


void Standard::next()
{
	int cur_track = metadata().current_track();
	int track_num = -1;

	// no track
	if(metadata().isEmpty() )
	{
		stop();
		set_track_idx_before_stop(-1);
		return;
	}

	// stopped
	if(cur_track == -1){
		track_num = 0;
	}

	else if(Playlist::Mode::isActiveAndEnabled(mode().rep1())){
		track_num = cur_track;
	}

	// shuffle mode
	else if(Playlist::Mode::isActiveAndEnabled(mode().shuffle()))
	{
		track_num = calc_shuffle_track();
		if(track_num == -1)
		{
			stop();
			set_track_idx_before_stop(-1);
			return;
		}
	}

	// normal track
	else
	{
		// last track
		if(cur_track == metadata().count() - 1)
		{
			if(Playlist::Mode::isActiveAndEnabled(mode().repAll())){
				track_num = 0;
			}

			else
			{
				stop();
				set_track_idx_before_stop(-1);
				return;
			}
		}

		else{
			track_num = cur_track + 1;
		}
	}

	change_track(track_num);
}


int Standard::calc_shuffle_track()
{
	if(metadata().size() <= 1){
		return -1;
	}

	QList<int> left_tracks;
	int i, left_tracks_idx;
	RandomGenerator rnd;

	// check all tracks played
	i=0;
	for(MetaData& md : metadata())
	{
		if(!md.played){
			left_tracks << i;
		}

		i++;
	}

	// no random track to play
	if(left_tracks.isEmpty()){
		if(Playlist::Mode::isActiveAndEnabled(mode().repAll())){
			return rnd.get_number(0, metadata().count() - 1);
		}

		else{
			return -1;
		}
	}

	left_tracks_idx = rnd.get_number(0, left_tracks.size() - 1);

	return left_tracks[left_tracks_idx];
}


void Standard::metadata_deleted(const MetaDataList& v_md_deleted)
{
	IndexSet indexes;

	int i=0;
	for(const MetaData& md : metadata())
	{
		for(auto it=v_md_deleted.begin(); it != v_md_deleted.end(); it++)
		{
			if(it->is_equal(md))
			{
				indexes.insert(i);
				break;
			}
		}

		i++;
	}

	metadata().remove_tracks(indexes);
	emit sig_items_changed( index() );
}


void Standard::metadata_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	Q_UNUSED(v_md_old)

	for(auto it=metadata().begin(); it !=metadata().end(); it++){
		auto lambda = [it](const MetaData& md) -> bool{
			return it->is_equal(md);
		};

		// find the value in old values
		auto tmp_it = std::find_if(v_md_new.begin(), v_md_new.end(), lambda);
		if(tmp_it != v_md_new.end()){
			// found, overwrite
			*it = *tmp_it;
		}
	}

	emit sig_items_changed( index() );
}

void Standard::duration_changed(MilliSeconds ms)
{
	MetaDataList& v_md = metadata();

	int cur_track = v_md.current_track();
	if(!between(cur_track, v_md.count())){
		return;
	}

	IdxList idx_list = find_tracks(v_md[cur_track].filepath());

	for(int i : idx_list)
	{
		MetaData changed_md(v_md[i]);
		changed_md.length_ms = std::max<MilliSeconds>(0, ms);

		replace_track(i, changed_md);
	}
}

void Standard::metadata_changed_single(const MetaData& md)
{
	IdxList idx_list = find_tracks(md.filepath());

	for(int i : idx_list){
		replace_track(i, md);
	}
}

