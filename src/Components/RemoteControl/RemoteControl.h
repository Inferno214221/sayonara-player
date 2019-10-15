/* RemoteControl.h */

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

#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include "Components/PlayManager/PlayState.h"
#include "Utils/Playlist/PlaylistFwd.h"

#include <QObject>

#include "Utils/Pimpl.h"

class QPixmap;

/**
 * @brief Remote control allows to control Sayonara from an external application via network.
 * Various commands are implemented. Sayonara also delivers information about state changes,
 * @ingroup RemoteControl
 *
 * The current implemented commands are:
 * \n\n
 * <B>play</B> \t start playing \n
 * <B>pause</B> \t pause playing \n
 * <B>prev</B> \t previous song \n
 * <B>next</B> \t next song \n
 * <B>playpause</B> \t toggle play/pause \n
 * <B>stop</B> \t stop playing \n
 * <B>volup</B> \t increase volume \n
 * <B>voldown</B> \t decrease volume \n
 * <B>setvol <int></B>\t change volume \n
 * <B>pl</B> \t fetch the active playlist \n
 * <B>curSong</B> \t fetch the current song index \n
 * <B>seekrel <int></B> \t seek within song in percent \n
 * <B>seekrelms <int></B> \t seek within song in relative to current position in seconds \n
 * <B>chtrk <int></B> \t change track \n
 * <B>state</B> \t request state: every answer except playlists are returned
 * \n\n
 * Answers are sent in JSON format. Each answer is terminated with 10 bytes long ENDMESSAGE.\n
  * The list of attributes is:
 * \n\n
 * <B>volume<int></B> \t current volume value between 0 and 100
 * \n\n
 * <B>Current track</B>\n
 * <B>track-title<string></B> \t current track title \n
 * <B>track-artist<string></B> \t current track artist \n
 * <B>track-album<string></B> \t current track album \n
 * <B>track-total-time<int></B> \t current track total time in seconds \n
 * <B>track-current-position<int></B> \t current track position in seconds
 * \n\n
 * <B>Broadcasting</B>\n
 * <B>broadcast-active<bool></B> \t is broadcast active? \n
 * <B>broadcast-port<int></B> \t port where broadcasts can be received from
 * \n\n
 * <B>Cover</B> \n
 * <B>cover-data<string></B> \t Base64 encoded JPG file\n
 * <B>cover-width<int></B> \t width of cover pixmap\n
 * <B>cover-height<int></B> \t height of cover pixmap\n
 * <B>playstate<string></B> \t one of the values "playing", "paused" or "stopped"
 * \n\n
 * <B>Playlist</B>\n
 * <B>playlist-current-index<int>\t current playing track index \n
 * <B>playlist<array></B> \t array of tracks\n
 * <B>pl-track-title<int></B> \t title of track \n
 * <B>pl-track-album<string></B> \t album of track \n
 * <B>pl-track-artist<string></B> \t artist of track \n
 * <B>pl-track-total-time<int></B> \t length of track in seconds \n
 */
class RemoteControl :
		public QObject
{
	Q_OBJECT
	PIMPL(RemoteControl)

public:
	explicit RemoteControl(QObject *parent=nullptr);
	~RemoteControl() override;

	bool is_connected() const;

private slots:
	void new_connection();
	void socket_disconnected();
	void new_request();

	void pos_changed_ms(MilliSeconds pos);
	void track_changed(const MetaData& md);
	void volume_changed(int vol);
	void playstate_changed(PlayState playstate);
	void active_playlist_changed(int index);
	void active_playlist_content_changed(int index);

	void cover_found(const QPixmap& pm);

	void _sl_active_changed();
	void _sl_port_changed();
	void _sl_broadcast_changed();


private:
	void init();

	void set_volume(int vol);
	void seek_rel(int pos_percent);
	void seek_rel_ms(int pos_ms);
	void change_track(int idx);

	void show_api();
	void request_state();

	int extract_parameter_int(const QByteArray& arr, int cmd_len);

	void insert_json_playstate(QJsonObject& o);
	void write_playstate();

	void insert_json_broadcast_info(QJsonObject& o);
	void write_broadcast_info();

	void insert_json_current_track(QJsonObject& o);
	void write_current_track();

	void insert_json_volume(QJsonObject& o) const;
	void write_volume();

	void insert_json_current_position(QJsonObject& o) const;
	void write_current_position();

	void insert_json_playlist(QJsonArray& o) const;
	void write_playlist();

	void search_cover();
	void json_cover(QJsonObject& o, const QPixmap& pm) const;

	void write(const QByteArray& arr);

	void active_changed();
};



#endif // REMOTECONTROL_H
