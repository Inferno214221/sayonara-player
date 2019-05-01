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
#include "Components/DBus/org_mpris_media_player2_adaptor.h"
#include "Components/DBus/org_mpris_media_player2_player_adaptor.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/AbstractPlaylist.h"

#include "Utils/RandomGenerator.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QMainWindow>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>
#include <QUrl>

struct DBusAdaptor::Private
{
	QString		object_path;
	QString		service_name;
	QString		dbus_service;
	QString		dbus_interface;

	PlayManagerPtr play_manager=nullptr;

	Private(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface) :
		object_path(object_path),
		service_name(service_name),
		dbus_service(dbus_service),
		dbus_interface(dbus_interface)
	{
		play_manager = PlayManager::instance();
	}
};

DBusAdaptor::DBusAdaptor(QStrRef object_path, QStrRef service_name, QStrRef dbus_service, QStrRef dbus_interface, QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(object_path, service_name, dbus_service, dbus_interface);
}

DBusAdaptor::~DBusAdaptor() {}

void DBusAdaptor::create_message(QString name, QVariant val)
{
	QDBusMessage sig;
	QVariantMap map;
	QVariantList args;

	map.insert(name, val);
	args << m->dbus_service << map << QStringList();

	// path, interface, name
	sig = QDBusMessage::createSignal(m->object_path, m->dbus_interface, "PropertiesChanged");
	sig.setArguments(args);

	QDBusConnection::sessionBus().send(sig);
}

QString DBusAdaptor::object_path() const
{
	return m->object_path;
}

QString DBusAdaptor::service_name() const
{
	return m->service_name;
}

QString DBusAdaptor::dbus_service() const
{
	return m->dbus_service;
}

QString DBusAdaptor::dbus_interface() const
{
	return m->dbus_interface;
}

struct DBusMPRIS::MediaPlayer2::Private
{
	Cover::Location cl;

	QStringList     supported_uri_schemes;
	QStringList     supported_mime_types;

	QString			playback_status;
	MetaData		md;
	MicroSeconds	pos;

	QMainWindow*	player=nullptr;
	PlayManagerPtr  play_manager=nullptr;

	double			volume;

	int				playlist_track_count;
	int				cur_idx;

	bool			can_previous;
	bool			can_next;

	bool			initialized;

	Private(QMainWindow* player) :
		playback_status("Stopped"),
		pos(0),
		player(player),
		volume(1.0),
		playlist_track_count(0),
		cur_idx(-1),
		can_previous(false),
		can_next(false),
		initialized(false)
	{

		const Playlist::Handler* plh = Playlist::Handler::instance();
		auto playlist = plh->active_playlist();

		play_manager = PlayManager::instance();
		volume = GetSetting(Set::Engine_Vol) / 100.0;

		if(playlist)
		{
			cur_idx = playlist->current_track_index();
			can_previous = (playlist->current_track_index() > 0);
			can_next = ((playlist->current_track_index() < playlist->count() - 1) && (cur_idx >= 0));
			playlist_track_count = playlist->count();
		}


		pos = (play_manager->current_position_ms() * 1000);
	}
};

DBusMPRIS::MediaPlayer2::MediaPlayer2(QMainWindow* player, QObject *parent) :
	DBusAdaptor("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.sayonara","org.mpris.MediaPlayer2.Player", "org.freedesktop.DBus.Properties", parent)
{
	m = Pimpl::make<Private>(player);

	connect(m->play_manager, &PlayManager::sig_playlist_changed,
			this, &DBusMPRIS::MediaPlayer2::playlist_len_changed);
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
	m->player->close();
}

void DBusMPRIS::MediaPlayer2::Raise()
{
	sp_log(Log::Debug, this) << "Raise";

	m->player->show();
	m->player->raise();
	m->player->show();
	m->player->raise();
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


QVariantMap DBusMPRIS::MediaPlayer2::Metadata()
{
	QVariantMap map;
	QVariant v_object_path, v_length;

	TrackID id = m->md.id;
	if(id == -1){
		id = RandomGenerator::get_random_number(5000, 10000);
	}
	QDBusObjectPath object_path(QString("/org/sayonara/track") + QString::number(id));

	v_object_path.setValue<QDBusObjectPath>(object_path);
	v_length.setValue<qlonglong>(m->md.length_ms * 1000);

	m->cl = Cover::Location::cover_location(m->md);
	QString cover_path = m->cl.preferred_path();

	QString title = m->md.title();
	if(title.isEmpty()){
		title = tr("None");
	}
	QString album = m->md.album();
	if(album.isEmpty()){
		album = tr("None");
	}
	QString artist = m->md.artist();
	if(artist.isEmpty()){
		artist = tr("None");
	}

	map["mpris:trackid"] = v_object_path;
	map["mpris:length"] = v_length;
	map["xesam:title"] = title;
	map["xesam:album"] = album;
	map["xesam:artist"] = QStringList({artist});
	map["mpris:artUrl"] = QUrl::fromLocalFile(cover_path).toString();

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
	return m->can_next;
}

bool DBusMPRIS::MediaPlayer2::CanGoPrevious()
{
	return m->can_previous;
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
	return true;
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

void DBusMPRIS::MediaPlayer2::SetShuffle(bool shuffle)
{
	Q_UNUSED(shuffle)
}

void DBusMPRIS::MediaPlayer2::SetVolume(double volume)
{
	m->play_manager->set_volume((int) (volume * 100));
	m->volume = volume;
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
	if(!m->initialized){
		init();
	}

	m->can_previous = (idx > 0);
	m->can_next = (idx < m->playlist_track_count - 1);

	create_message("CanGoNext", m->can_next);
	create_message("CanGoPrevious", m->can_previous);

	m->cur_idx = idx;
}

void DBusMPRIS::MediaPlayer2::playlist_len_changed(int track_count)
{
	if(!m->initialized){
		init();
	}

	m->can_next = (m->cur_idx < track_count - 1 && m->cur_idx >= 0);

	create_message("CanGoNext", m->can_next);

	m->playlist_track_count = track_count;
}

void DBusMPRIS::MediaPlayer2::track_changed(const MetaData& md)
{
	m->md = md;
	if(!m->initialized){
		init();
	}

	QVariantMap map = Metadata();
	create_message("Metadata", map);
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
