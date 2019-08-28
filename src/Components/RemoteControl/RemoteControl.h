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
 * @ingroup Components
 *
 * The current implemented commands are:\n
 *
 *
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
 * <B>state</B> \t request state: every answer except playlists are returned \n
 * \n\n
 * The current implemented answers are: (multiple parameters are separated by the tab character.
 * And answer always ends with th four bytes combination 0x00 0x01 0x00 0x01)
 * \n
 * <B>curPos:<int></B> \t current position in seconds \n
 * <B>vol:<int></B> \t current volume \n
 * <B>curIdx:<int></B> \t current track index in active playlist \n
 * <B>title:<string></B> \t current track title \n
 * <B>artist:<string></B> \t current track artist \n
 * <B>album:<string></B> \t current track album \n
 * <B>totalPos:<string></B> \t length of track in seconds \n
 * <B>playstate:[playing|paused|stopped]</B> \t current playback state \n
 * <B>playlist:<stringlist></B> \t active playlist \n
 * <B>broadcast:<int,int></B> \t broadcast settings: <active, broadcast port> \n
 * <B>coverinfo:<width,height,format></B> \t width, height, qt image format.\n
 * \tfollowed by 0x00,0x01,0x00,0x01<cover data>0x00,0x01,0x00,0x01
 * @ingroup Components
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

	void json_playstate(QJsonObject& o);
	void write_playstate();

	void json_broadcast_info(QJsonObject& o);
	void write_broadcast_info();

	void json_current_track(QJsonObject& o);
	void write_current_track();

	void json_volume(QJsonObject& o) const;
	void write_volume();

	void json_current_position(QJsonObject& o) const;
	void write_current_position();

	void json_playlist(QJsonArray& o) const;
	void write_playlist();

	void search_cover();
	void json_cover(QJsonObject& o, const QPixmap& pm) const;

	void write(const QByteArray& arr);

	void active_changed();
};



#endif // REMOTECONTROL_H
