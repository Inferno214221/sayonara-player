/* DBusMPRIS.cpp */

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

#include "DBusMPRIS.h"
#include "DBus/org_mpris_media_player2_adaptor.h"
#include "DBus/org_mpris_media_player2_player_adaptor.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"

#include "Utils/Filepath.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QMainWindow>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>
#include <QUrl>
#include <QTimer>

struct DBusMPRIS::MediaPlayer2::Private
{
	QString			cover_path;

	QStringList     supported_uri_schemes;
	QStringList     supported_mime_types;

	QString			playback_status;
	MetaData		md;
	MicroSeconds	pos;

	QMainWindow*	player=nullptr;
	PlayManager*	play_manager=nullptr;

	double			volume;

	bool			initialized;

	Private(QMainWindow* player) :
		playback_status("Stopped"),
		pos(0),
		player(player),
		volume(1.0),
		initialized(false)
	{
		cover_path = Util::Filepath(Cover::Location::invalid_path()).filesystem_path();
		play_manager = PlayManager::instance();
		volume = GetSetting(Set::Engine_Vol) / 100.0;

		pos = (play_manager->current_position_ms() * 1000);
	}
};

DBusMPRIS::MediaPlayer2::MediaPlayer2(QMainWindow* player, QObject *parent) :
	DBusAdaptor("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.sayonara","org.mpris.MediaPlayer2.Player", "org.freedesktop.DBus.Properties", parent)
{
	m = Pimpl::make<Private>(player);

	connect(m->play_manager, &PlayManager::sig_playstate_changed,
			this, &DBusMPRIS::MediaPlayer2::playstate_changed);
	connect(m->play_manager, &PlayManager::sig_track_changed,
			this, &DBusMPRIS::MediaPlayer2::track_changed);
	connect(m->play_manager, &PlayManager::sig_track_idx_changed,
			this, &DBusMPRIS::MediaPlayer2::track_idx_changed);
	connect(m->play_manager, &PlayManager::sig_position_changed_ms,
			this, &DBusMPRIS::MediaPlayer2::position_changed);
	connect(m->play_manager, &PlayManager::sig_volume_changed,
			this, &DBusMPRIS::MediaPlayer2::volume_changed);
	connect(m->play_manager, &PlayManager::sig_track_metadata_changed,
			this, &DBusMPRIS::MediaPlayer2::track_metadata_changed);

	track_changed(m->play_manager->current_track());
}


DBusMPRIS::MediaPlayer2::~MediaPlayer2()
{
	QDBusConnection::sessionBus().unregisterObject(object_path());
	QDBusConnection::sessionBus().unregisterService(service_name());
}

void DBusMPRIS::MediaPlayer2::init()
{
	if(m->initialized){
		return;
	}

	new OrgMprisMediaPlayer2Adaptor(this);
	new OrgMprisMediaPlayer2PlayerAdaptor(this);

	if (!QDBusConnection::sessionBus().registerService(service_name())) {
		sp_log(Log::Error, this)	<< "Failed to register "
							<< service_name()
							<< " on the session bus";

		m->initialized = true;

		return;
	}

	sp_log(Log::Info, this) << service_name() << " registered";

	QDBusConnection::sessionBus().registerObject(object_path(), this);
	create_message("DesktopEntry", QString("sayonara"));

	m->initialized = true;
}


bool DBusMPRIS::MediaPlayer2::CanQuit() const
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::CanRaise()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::HasTrackList()
{
	return false;
}

QString DBusMPRIS::MediaPlayer2::Identity()
{
	return QString("Sayonara Player");
}

QString DBusMPRIS::MediaPlayer2::DesktopEntry()
{
	return QString("sayonara");
}

QStringList DBusMPRIS::MediaPlayer2::SupportedUriSchemes()
{
	QStringList uri_schemes;
	uri_schemes << "file"
				<< "http"
				<< "cdda"
				<< "smb"
				<< "sftp";

	return uri_schemes;
}

QStringList DBusMPRIS::MediaPlayer2::SupportedMimeTypes()
{
	QStringList mimetypes;
	mimetypes   << "audio/mpeg"
				<< "audio/ogg";

	return mimetypes;
}

bool DBusMPRIS::MediaPlayer2::CanSetFullscreen()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::Fullscreen()
{
	return GetSetting(Set::Player_Fullscreen);
}


void DBusMPRIS::MediaPlayer2::SetFullscreen(bool b)
{
	SetSetting(Set::Player_Fullscreen, b);
}


void DBusMPRIS::MediaPlayer2::Quit()
{
	SetSetting(SetNoDB::Player_Quit, true);
	m->player->close();
}


void DBusMPRIS::MediaPlayer2::Raise()
{
	sp_log(Log::Debug, this) << "Raise";

	QByteArray geometry = GetSetting(Set::Player_Geometry);

	if(m->player->isMinimized()){

		QTimer::singleShot(200, [=]()
		{
			m->player->showNormal();
		});
	}
	else
	{
		QTimer::singleShot(200, [=]()
		{
			m->player->restoreGeometry(geometry);
			m->player->showNormal();
		});
	}
}

/*** mpris.mediaplayer2.player ***/

QString DBusMPRIS::MediaPlayer2::PlaybackStatus()
{
	return m->playback_status;
}

QString DBusMPRIS::MediaPlayer2::LoopStatus()
{
	return "None";
}

double DBusMPRIS::MediaPlayer2::Rate()
{
	return 1.0;
}

bool DBusMPRIS::MediaPlayer2::Shuffle()
{
	return false;
}

#include "Utils/Set.h"

QVariantMap DBusMPRIS::MediaPlayer2::Metadata()
{
	QVariantMap map;
	QVariant v_object_path, v_length;

	TrackID id = m->md.id();
	if(id == -1){
		id = RandomGenerator::get_random_number(5000, 10000);
	}
	QDBusObjectPath object_path(QString("/org/sayonara/track") + QString::number(id));

	v_object_path.setValue<QDBusObjectPath>(object_path);
	v_length.setValue<qlonglong>(m->md.duration_ms() * 1000);

	QString title = m->md.title();
	if(title.isEmpty()){
		title = Lang::get(Lang::UnknownTitle);
	}
	QString album = m->md.album();
	if(album.isEmpty()){
		album = Lang::get(Lang::UnknownAlbum);
	}
	QString artist = m->md.artist();
	if(artist.isEmpty()){
		artist = Lang::get(Lang::UnknownArtist);
	}
	QString album_artist = m->md.album_artist();
	if(album_artist.isEmpty()){
		album_artist = Lang::get(Lang::UnknownArtist);
	}

	map["mpris:artUrl"] = QUrl::fromLocalFile(m->cover_path).toString();
	map["mpris:length"] = v_length;
	map["mpris:trackid"] = v_object_path;

	map["xesam:album"] = album;
	map["xesam:albumArtist"] = album_artist;
	map["xesam:artist"] = QStringList({artist});

	if(!m->md.comment().isEmpty()) {
		map["xesam:comment"] = m->md.comment();
	}

	if(m->md.createdate_datetime().isValid()) {
		map["contentCreated"] = m->md.createdate_datetime().toString(Qt::ISODate);
	}

	map["xesam:discNumber"] = int(m->md.discnumber());

	if(!m->md.genres().isEmpty()) {
		map["xesam:genre"] = m->md.genres_to_list().join(", ");
	}

	map["xesam:trackNumber"] = int(m->md.track_number());
	map["xesam:title"] = title;
	map["xesam:userRating"] = (int(m->md.rating()) / 5.0);

	map["sayonara:year"] = int(m->md.year());
	map["sayonara:bitrate"] = int(m->md.bitrate());
	map["sayonara:filesize"] = QVariant::fromValue<int>(int(m->md.filesize()));

	return map;
}

double DBusMPRIS::MediaPlayer2::Volume()
{
	return m->volume;
}

qlonglong DBusMPRIS::MediaPlayer2::Position()
{
	return m->pos;
}


double DBusMPRIS::MediaPlayer2::MinimumRate()
{
	return 1.0;
}

double DBusMPRIS::MediaPlayer2::MaximumRate()
{
	return 1.0;
}

bool DBusMPRIS::MediaPlayer2::CanGoNext()
{
	Playlist::Handler* handler = Playlist::Handler::instance();
	PlaylistConstPtr pl = handler->playlist(handler->current_index());
	if(!pl){
		return false;
	}

	Playlist::Mode mode = pl->mode();
	bool b =	Playlist::Mode::isActiveAndEnabled(mode.shuffle()) ||
				Playlist::Mode::isActiveAndEnabled(mode.repAll());

	return ((b && pl->count() > 0) || (pl->current_track_index() < pl->count() - 1));
}

bool DBusMPRIS::MediaPlayer2::CanGoPrevious()
{
	Playlist::Handler* handler = Playlist::Handler::instance();
	PlaylistConstPtr pl = handler->playlist(handler->current_index());
	if(!pl){
		return false;
	}

	return (pl->current_track_index() > 0 && pl->count() > 1);
}

bool DBusMPRIS::MediaPlayer2::CanPlay()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::CanPause()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::CanSeek()
{
	return (m->play_manager->current_track().duration_ms() > 0);
}

bool DBusMPRIS::MediaPlayer2::CanControl()
{
	return true;
}


void DBusMPRIS::MediaPlayer2::Next()
{
	m->play_manager->next();
}


void DBusMPRIS::MediaPlayer2::Previous()
{
	m->play_manager->previous();
}

void DBusMPRIS::MediaPlayer2::Pause()
{
	m->play_manager->pause();
}


void DBusMPRIS::MediaPlayer2::PlayPause()
{
	m->play_manager->play_pause();
}

void DBusMPRIS::MediaPlayer2::Stop()
{
	m->play_manager->stop();
}

void DBusMPRIS::MediaPlayer2::Play()
{
	m->playback_status = "Playing";
	m->play_manager->play();
}

void DBusMPRIS::MediaPlayer2::Seek(qlonglong offset)
{
	m->play_manager->seek_rel_ms(offset / 1000);
}

void DBusMPRIS::MediaPlayer2::SetPosition(const QDBusObjectPath& track_id, qlonglong position)
{
	Q_UNUSED(track_id)
	m->play_manager->seek_abs_ms(position / 1000);
}

void DBusMPRIS::MediaPlayer2::OpenUri(const QString& uri)
{
	Q_UNUSED(uri)
}

void DBusMPRIS::MediaPlayer2::SetLoopStatus(QString status)
{
	Q_UNUSED(status)
}

void DBusMPRIS::MediaPlayer2::SetRate(double rate)
{
	Q_UNUSED(rate)
}

int DBusMPRIS::MediaPlayer2::Rating()
{
	return int(m->md.rating());
}

void DBusMPRIS::MediaPlayer2::SetShuffle(bool shuffle)
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);
	plm.setShuffle(shuffle);
	SetSetting(Set::PL_Mode, plm);
}

void DBusMPRIS::MediaPlayer2::SetVolume(double volume)
{
	m->play_manager->set_volume(int(volume * 100));
	m->volume = volume;
}

void DBusMPRIS::MediaPlayer2::IncreaseVolume()
{
	m->play_manager->volume_up();
}

void DBusMPRIS::MediaPlayer2::DecreaseVolume()
{
	m->play_manager->volume_down();
}

void DBusMPRIS::MediaPlayer2::volume_changed(int volume)
{
	if(!m->initialized){
		init();
	}

	m->volume = (volume / 100.0);

	create_message("Volume", volume / 100.0);
}


void DBusMPRIS::MediaPlayer2::position_changed(MilliSeconds pos)
{
	if(!m->initialized){
		init();
	}

	MicroSeconds new_pos = pos * 1000;
	MicroSeconds difference = new_pos - m->pos;

	if(difference < 0 || difference > 1000000){
		emit Seeked(new_pos);
	}

	m->pos = new_pos;
}


void DBusMPRIS::MediaPlayer2::track_idx_changed(int idx)
{
	Q_UNUSED(idx)

	if(!m->initialized){
		init();
	}

	create_message("CanGoNext", CanGoNext());
	create_message("CanGoPrevious", CanGoPrevious());
}


void DBusMPRIS::MediaPlayer2::track_changed(const MetaData& md)
{
	m->md = md;
	m->cover_path = Util::Filepath(Cover::Location::cover_location(md).preferred_path()).filesystem_path();

	if(!m->initialized){
		init();
	}

	QVariantMap map = Metadata();
	create_message("Metadata", map);
}


void DBusMPRIS::MediaPlayer2::track_metadata_changed()
{
	track_changed(m->play_manager->current_track());
}


void DBusMPRIS::MediaPlayer2::playstate_changed(PlayState state)
{
	QString playback_status;
	if(!m->initialized){
		init();
	}

	switch(state){
		case PlayState::Stopped:
			playback_status = "Stopped";
			break;
		case PlayState::Playing:
			playback_status = "Playing";
			break;
		case PlayState::Paused:
			playback_status = "Paused";
			break;
		default:
			playback_status = "Stopped";
			break;
	}

	m->playback_status = playback_status;

	create_message("PlaybackStatus", playback_status);
}
