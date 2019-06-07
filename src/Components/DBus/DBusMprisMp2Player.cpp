#include "DBusMprisMp2Player.h"
#include "Components/DBus/org_mpris_media_player2_player_adaptor.h"

#include "Components/PlayManager/PlayManager.h"
#include "Utils/Settings/Settings.h"
#include "Components/Covers/CoverLocation.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Logger/Logger.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>

struct DBusMPRIS::MediaPlayer2Player::Private
{
	QStringList     supported_uri_schemes;
	QStringList     supported_mime_types;

	Cover::Location cl;
	MetaData		md;
	QString			playback_status;
	PlayManager*	play_manager;
	MicroSeconds	pos;
	double			volume;
	bool			initialized;

	Private() :
		playback_status("Stopped"),
		pos(0),
		initialized(false)
	{
		volume = GetSetting(Set::Engine_Vol) / 100.0;
		play_manager = PlayManager::instance();
		pos = (play_manager->current_position_ms() * 1000);
	}
};


DBusMPRIS::MediaPlayer2Player::MediaPlayer2Player(QObject* parent) :
	DBusAdaptor("/org/mpris/MediaPlayer2/Player", "org.mpris.MediaPlayer2.Player.sayonara","org.mpris.MediaPlayer2.Player", "org.freedesktop.DBus.Properties", parent)
{
	m = Pimpl::make<Private>();

	connect(m->play_manager, &PlayManager::sig_playstate_changed,
			this, &DBusMPRIS::MediaPlayer2Player::playstate_changed);
	connect(m->play_manager, &PlayManager::sig_track_changed,
			this, &DBusMPRIS::MediaPlayer2Player::track_changed);
	connect(m->play_manager, &PlayManager::sig_track_idx_changed,
			this, &DBusMPRIS::MediaPlayer2Player::track_idx_changed);
	connect(m->play_manager, &PlayManager::sig_position_changed_ms,
			this, &DBusMPRIS::MediaPlayer2Player::position_changed);
	connect(m->play_manager, &PlayManager::sig_volume_changed,
			this, &DBusMPRIS::MediaPlayer2Player::volume_changed);

		track_changed(m->play_manager->current_track());
}

DBusMPRIS::MediaPlayer2Player::~MediaPlayer2Player() {}

void DBusMPRIS::MediaPlayer2Player::init()
{
	if(m->initialized){
		return;
	}

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
	m->initialized = true;
}

double DBusMPRIS::MediaPlayer2Player::Volume()
{
	return m->volume;
}

qlonglong DBusMPRIS::MediaPlayer2Player::Position()
{
	return m->pos;
}


double DBusMPRIS::MediaPlayer2Player::MinimumRate()
{
	return 1.0;
}

double DBusMPRIS::MediaPlayer2Player::MaximumRate()
{
	return 1.0;
}

bool DBusMPRIS::MediaPlayer2Player::CanGoNext()
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

bool DBusMPRIS::MediaPlayer2Player::CanGoPrevious()
{
	Playlist::Handler* handler = Playlist::Handler::instance();
	PlaylistConstPtr pl = handler->playlist(handler->current_index());
	if(!pl){
		return false;
	}

	return (pl->current_track_index() > 0 && pl->count() > 1);
}

bool DBusMPRIS::MediaPlayer2Player::CanPlay()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2Player::CanPause()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2Player::CanSeek()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2Player::CanControl()
{
	return true;
}


void DBusMPRIS::MediaPlayer2Player::Next()
{
	m->play_manager->next();
}


void DBusMPRIS::MediaPlayer2Player::Previous()
{
	m->play_manager->previous();
}

void DBusMPRIS::MediaPlayer2Player::Pause()
{
	m->play_manager->pause();
}


void DBusMPRIS::MediaPlayer2Player::PlayPause()
{
	m->play_manager->play_pause();
}

void DBusMPRIS::MediaPlayer2Player::Stop()
{
	m->play_manager->stop();
}

void DBusMPRIS::MediaPlayer2Player::Play()
{
	m->playback_status = "Playing";
	m->play_manager->play();
}

void DBusMPRIS::MediaPlayer2Player::Seek(qlonglong offset)
{
	m->play_manager->seek_rel_ms(offset / 1000);
}

void DBusMPRIS::MediaPlayer2Player::SetPosition(const QDBusObjectPath& track_id, qlonglong position)
{
	Q_UNUSED(track_id)
	m->play_manager->seek_abs_ms(position / 1000);
}

void DBusMPRIS::MediaPlayer2Player::OpenUri(const QString& uri)
{
	Q_UNUSED(uri)
}

void DBusMPRIS::MediaPlayer2Player::SetLoopStatus(QString status)
{
	Q_UNUSED(status)
}

void DBusMPRIS::MediaPlayer2Player::SetRate(double rate)
{
	Q_UNUSED(rate)
}

void DBusMPRIS::MediaPlayer2Player::SetShuffle(bool shuffle)
{
	Q_UNUSED(shuffle)
}

void DBusMPRIS::MediaPlayer2Player::SetVolume(double volume)
{
	m->play_manager->set_volume((int) (volume * 100));
	m->volume = volume;
}


void DBusMPRIS::MediaPlayer2Player::volume_changed(int volume)
{
	if(!m->initialized){
		init();
	}

	m->volume = (volume / 100.0);

	create_message("Volume", volume / 100.0);
}


void DBusMPRIS::MediaPlayer2Player::position_changed(MilliSeconds pos)
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


void DBusMPRIS::MediaPlayer2Player::track_idx_changed(int idx)
{
	Q_UNUSED(idx)

	if(!m->initialized){
		init();
	}

	create_message("CanGoNext", CanGoNext());
	create_message("CanGoPrevious", CanGoPrevious());
}


void DBusMPRIS::MediaPlayer2Player::track_changed(const MetaData& md)
{
	m->md = md;
	if(!m->initialized){
		init();
	}

	QVariantMap map = Metadata();
	create_message("Metadata", map);
}

void DBusMPRIS::MediaPlayer2Player::playstate_changed(PlayState state)
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


QString DBusMPRIS::MediaPlayer2Player::PlaybackStatus()
{
	return m->playback_status;
}

QString DBusMPRIS::MediaPlayer2Player::LoopStatus()
{
	return "None";
}

double DBusMPRIS::MediaPlayer2Player::Rate()
{
	return 1.0;
}

bool DBusMPRIS::MediaPlayer2Player::Shuffle()
{
	return false;
}

QVariantMap DBusMPRIS::MediaPlayer2Player::Metadata()
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
