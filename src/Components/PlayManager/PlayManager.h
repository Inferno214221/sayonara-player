/* PlayManager.h */

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

#ifndef PLAY_MANAGER_H
#define PLAY_MANAGER_H

#include <QObject>
#include "PlayState.h"
#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

/**
 * @brief Global handler for current playback state (Singleton)
 * @ingroup Components
 */
class PlayManager :
		public QObject
{
	Q_OBJECT

	SINGLETON_QOBJECT(PlayManager)
	PIMPL(PlayManager)

signals:

	/**
	 * @brief emitted when a streamed track has finished
	 * @param old_md the last played track
	 */
	void sig_www_track_finished(const MetaData& old_md);

	/**
	 * @brief emitted, when PlayState was changed
	 */
	void sig_playstate_changed(PlayState);

	/**
	 * @brief next track was triggered
	 */
	void sig_next();

	/**
	 * @brief This signal is sent when the playstate changed
	 * from stopped to play
	 */
	void sig_wake_up();

	/**
	 * @brief previous track was triggered
	 */
	void sig_previous();

	/**
	 * @brief stop was triggered
	 */
	void sig_stopped();

	/**
	 * @brief relative seeking was triggered
	 * @param percent relative position in track
	 */
	void sig_seeked_rel(double percent);

	/**
	 * @brief relative seeking was triggered
	 * @param ms relative position to current position in milliseconds
	 */
	void sig_seeked_rel_ms(MilliSeconds ms);

	/**
	 * @brief absolute seeking was triggered
	 * @param ms absolute position in milliseconds
	 */
	void sig_seeked_abs_ms(MilliSeconds ms);

	/**
	 * @brief position in track has changed
	 * @param ms absolute position in milliseconds
	 */
	void sig_position_changed_ms(MilliSeconds ms);

	/**
	 * @brief track has changed
	 * @param md new MetaData
	 */
	void sig_track_changed(const MetaData& md);

	void sig_track_metadata_changed();

	/**
	 * @brief track has changed
	 * @param idx index in playlist
	 */
	void sig_track_idx_changed(int idx);

	/**
	 * @brief duration of track has changed
	 * @param ms duration of track in milliseconds
	 */
	void sig_duration_changed();

	void sig_bitrate_changed();

	/**
	 * @brief playlist has finished
	 */
	void sig_playlist_finished();

	/**
	 * @brief recording is requested
	 * @param b
	 *  true, when a new recording session should begin,
	 *  false if a recording session should stop
	 */
	void sig_record(bool b);

	/**
	 * @brief emitted when currently in buffering state
	 * @param b true if buffering, false else
	 */
	void sig_buffer(int b);

	/**
	 * @brief emitted when volume has changed
	 * @param vol value between 0 and 100
	 */
	void sig_volume_changed(int vol);


	/**
	 * @brief emitted when mute state has changed
	 * @param b true if muted, false else
	 */
	void sig_mute_changed(bool b);

	void sig_error(const QString& message);


public slots:
	/**
	 * @brief Start playing if there's a track
	 */
	void play();

	/**
	 * @brief Emit wake up signal after stopping state
	 */
	void wake_up();

	/**
	 * @brief toggle play/pause
	 */
	void play_pause();

	/**
	 * @brief pause track, if currently playing
	 */
	void pause();

	/**
	 * @brief change to previous track
	 */
	void previous();

	/**
	 * @brief change to next track
	 */
	void next();

	/**
	 * @brief stop playback
	 */
	void stop();

	/**
	 * @brief request recording (see also sig_record(bool b))
	 * @param b
	 *  true, when a new recording session should begin,
	 *  false if a recording session should stop
	 */
	void record(bool b);

	/**
	 * @brief seek relative
	 * @param percent relative position within track
	 */
	void seek_rel(double percent);

	/**
	 * @brief seek absolute
	 * @param ms absolute position in milliseconds
	 */
	void seek_abs_ms(MilliSeconds ms);

	/**
	 * @brief seek_rel_ms
	 * @param ms relative position to current position in milliseconds
	 */
	void seek_rel_ms(MilliSeconds ms);

	/**
	 * @brief set current position of track
	 * This method does not seek.
	 * Just tells the playmanager where the current position is
	 * @param ms position in milliseconds.
	 */
	void set_position_ms(MilliSeconds ms);

	/**
	 * @brief change current track
	 * @param md new MetaData object
	 */
	void change_track(const MetaData& md, int track_idx);

	/**
	 * @brief change_track
	 * @param md
	 */
	void change_track_metadata(const MetaData& md);


	/**
	 * @brief notify, that track is ready for playback
	 */
	void set_track_ready();
	void set_track_finished();

	/**
	 * @brief notifiy, that track is in buffering state currently
	 * @param progress
	 */
	void buffering(int progress);

	/**
	 * @brief increase volume by 5
	 */
	void volume_up();

	/**
	 * @brief decrease volume by 5
	 */
	void volume_down();

	/**
	 * @brief set volume
	 * @param vol value between [0,100], will be cropped if not within boundaries
	 */
	void set_volume(int vol);

	/**
	 * @brief mute/unmute
	 * @param b
	 */
	void set_muted(bool b);

	/**
	 * @brief If already muted, then unmute. If unmuted, then mute it
	 */
	void toggle_mute();


	/**
	 * @brief Change the duration. This is usually called when
	 * the Engine sends a duration changed signal. You should
	 * not use this
	 * @param ms
	 */
	void change_duration(MilliSeconds ms);

	void change_bitrate(Bitrate br);

	/**
	 * @brief Some playback error occured
	 * @param message
	 */
	void error(const QString& message);

public:
	/**
	 * @brief get current play state
	 * @return PlayState enum
	 */
	PlayState	playstate() const;

	/**
	 * @brief get current position in milliseconds
	 * @return current position in milliseconds
	 */
	MilliSeconds		current_position_ms() const;

	/**
	 * @brief get position in milliseconds where track will start
	 * @return position in milliseconds where track will start
	 */
	MilliSeconds		initial_position_ms() const;

	/**
	 * @brief get duration of track
	 * @return duration in milliseconds
	 */
	MilliSeconds		duration_ms() const;


	Bitrate				bitrate() const;

	/**
	 * @brief get current track
	 * @return MetaData object of current track
	 */
	const MetaData& current_track() const;

	/**
	 * @brief get current volume
	 * @return value between 0 and 100
	 */
	int			volume() const;


	/**
	 * @brief query mute status
	 * @return true if muted, false else
	 */
	bool		is_muted() const;


	/**
	 * @brief Shutdown the computer
	 */
	void		shutdown();
};

using PlayManagerPtr=PlayManager*;

#endif


