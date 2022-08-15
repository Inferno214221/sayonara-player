/* DBusMPRIS.cpp */

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
 */

#include "DBusMPRIS.h"
#include "DBus/org_mpris_media_player2_adaptor.h"
#include "DBus/org_mpris_media_player2_player_adaptor.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Playlist/Playlist.h"
#include "Interfaces/PlayManager.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/Filepath.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <QMainWindow>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>
#include <QUrl>
#include <QTimer>

namespace
{
	QString checkString(const QString& str, const Lang::Term fallback)
	{
		return str.isEmpty() ? Lang::get(fallback) : str;
	}

	QString getPlaybackStatusString(const PlayState playState)
	{
		switch(playState)
		{
			case PlayState::Stopped:
				return "Stopped";
			case PlayState::Playing:
				return "Playing";
			case PlayState::Paused:
				return "Paused";
			default:
				return "Stopped";
		}
	}

	QDBusObjectPath createObjectPath(Id trackId)
	{
		if(trackId < 0)
		{
			trackId = RandomGenerator::getRandomNumber(5000, 10000); // NOLINT(readability-magic-numbers);
		}

		return QDBusObjectPath {QString("/org/sayonara/track%1").arg(trackId)};
	}

	constexpr const auto LoopStatusNone = "None";
	constexpr const auto LoopStatusTrack = "Track";
	constexpr const auto LoopStatusPlaylist = "Playlist";

	PlaylistMode loopStatusToPlaylistMode(const QString& loopStatus)
	{
		auto playlistMode = GetSetting(Set::PL_Mode);
		if(loopStatus == LoopStatusNone)
		{
			playlistMode.setRep1(false);
			playlistMode.setRepAll(false);
		}

		else if(loopStatus == LoopStatusTrack)
		{
			playlistMode.setRepAll(false);
			playlistMode.setRep1(true);
		}

		else if(loopStatus == LoopStatusPlaylist)
		{
			playlistMode.setRep1(false);
			playlistMode.setRepAll(true);
		}

		return playlistMode;
	}

	QString playlistModeToLoopStatus(const PlaylistMode& playlistMode)
	{
		if(PlaylistMode::isActiveAndEnabled(playlistMode.rep1()))
		{
			return LoopStatusTrack;
		}

		if(PlaylistMode::isActiveAndEnabled(playlistMode.repAll()))
		{
			return LoopStatusPlaylist;
		}

		return LoopStatusNone;
	}
}

struct DBusMPRIS::MediaPlayer2::Private
{
	PlayManager* playManager;
	QMainWindow* player;
	PlaylistAccessor* playlistAccessor;

	QString coverPath {Util::Filepath(Cover::Location::invalidPath()).fileystemPath()};

	MetaData track;
	MicroSeconds pos;
	double volume {GetSetting(Set::Engine_Vol) / 100.0};
	bool initialized {false};

	Private(QMainWindow* player, PlayManager* playManager, PlaylistAccessor* playlistAccessor) :
		playManager(playManager),
		player(player),
		playlistAccessor(playlistAccessor),
		pos(playManager->currentPositionMs() * 1000) {}
};

DBusMPRIS::MediaPlayer2::MediaPlayer2(QMainWindow* player, PlayManager* playManager,
                                      PlaylistAccessor* playlistAccessor,
                                      QObject* parent) :
	DBusAdaptor("/org/mpris/MediaPlayer2",
	            "org.mpris.MediaPlayer2.sayonara",
	            "org.mpris.MediaPlayer2.Player",
	            "org.freedesktop.DBus.Properties",
	            parent)
{
	m = Pimpl::make<Private>(player, playManager, playlistAccessor);

	connect(m->playManager, &PlayManager::sigPlaystateChanged,
	        this, &DBusMPRIS::MediaPlayer2::playstateChanged);
	connect(m->playManager, &PlayManager::sigCurrentTrackChanged,
	        this, &DBusMPRIS::MediaPlayer2::trackChanged);
	connect(m->playManager, &PlayManager::sigTrackIndexChanged,
	        this, &DBusMPRIS::MediaPlayer2::trackIndexChanged);
	connect(m->playManager, &PlayManager::sigPositionChangedMs,
	        this, &DBusMPRIS::MediaPlayer2::positionChanged);
	connect(m->playManager, &PlayManager::sigVolumeChanged,
	        this, &DBusMPRIS::MediaPlayer2::volumeChanged);
	connect(m->playManager, &PlayManager::sigCurrentMetadataChanged,
	        this, &DBusMPRIS::MediaPlayer2::trackMetadataChanged);

	trackChanged(m->playManager->currentTrack());
}

DBusMPRIS::MediaPlayer2::~MediaPlayer2()
{
	QDBusConnection::sessionBus().unregisterObject(objectPath());
	QDBusConnection::sessionBus().unregisterService(serviceName());
}

void DBusMPRIS::MediaPlayer2::init()
{
	if(m->initialized)
	{
		return;
	}

	new OrgMprisMediaPlayer2Adaptor(this);
	new OrgMprisMediaPlayer2PlayerAdaptor(this);

	if(!QDBusConnection::sessionBus().registerService(serviceName()))
	{
		spLog(Log::Error, this) << "Failed to register "
		                        << serviceName()
		                        << " on the session bus";

		m->initialized = true;

		return;
	}

	spLog(Log::Info, this) << serviceName() << " registered";

	QDBusConnection::sessionBus().registerObject(objectPath(), this);
	createMessage("DesktopEntry", QString("sayonara"));

	m->initialized = true;
}

bool DBusMPRIS::MediaPlayer2::CanQuit() const { return true; }

bool DBusMPRIS::MediaPlayer2::CanRaise() { return true; }

bool DBusMPRIS::MediaPlayer2::HasTrackList() { return false; }

QString DBusMPRIS::MediaPlayer2::Identity() { return QString("Sayonara Player"); }

QString DBusMPRIS::MediaPlayer2::DesktopEntry() { return QString("sayonara"); }

QStringList DBusMPRIS::MediaPlayer2::SupportedUriSchemes()
{
	return QStringList {
		"file",
		"http",
		"cdda",
		"smb",
		"sftp"
	};
}

QStringList DBusMPRIS::MediaPlayer2::SupportedMimeTypes()
{
	return QStringList {
		"audio/mpeg"
		"audio/ogg"
	};
}

bool DBusMPRIS::MediaPlayer2::CanSetFullscreen() { return true; }

bool DBusMPRIS::MediaPlayer2::Fullscreen() { return GetSetting(Set::Player_Fullscreen); }

void DBusMPRIS::MediaPlayer2::SetFullscreen(bool b) { SetSetting(Set::Player_Fullscreen, b); }

void DBusMPRIS::MediaPlayer2::Quit()
{
	m->player->setProperty("shutdown", true);
	m->player->close();
}

void DBusMPRIS::MediaPlayer2::Raise()
{
	spLog(Log::Debug, this) << "Raise";

	QByteArray geometry = GetSetting(Set::Player_Geometry);

	if(m->player->isMinimized())
	{
		QTimer::singleShot(200, [=]() {
			m->player->showNormal();
		});
	}
	else
	{
		QTimer::singleShot(200, [=]() {
			m->player->restoreGeometry(geometry);
			m->player->showNormal();
		});
	}
}

/*** mpris.mediaplayer2.player ***/

QString DBusMPRIS::MediaPlayer2::PlaybackStatus() { return getPlaybackStatusString(m->playManager->playstate()); }

QString DBusMPRIS::MediaPlayer2::LoopStatus() // NOLINT(readability-convert-member-functions-to-static)
{
	return playlistModeToLoopStatus(GetSetting(Set::PL_Mode));
}

double DBusMPRIS::MediaPlayer2::Rate() { return 1.0; }

bool DBusMPRIS::MediaPlayer2::Shuffle() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto playlistMode = GetSetting(Set::PL_Mode);
	return PlaylistMode::isActiveAndEnabled(playlistMode.shuffle());
}

QVariantMap DBusMPRIS::MediaPlayer2::Metadata()
{
	QVariantMap map;
	QVariant objectPathVariant, lengthVariant;

	auto id = m->track.id();
	if(id == -1)
	{
		id = RandomGenerator::getRandomNumber(5000, 10000);
	}

	const auto objectPath = QDBusObjectPath(QString("/org/sayonara/track") + QString::number(id));

	objectPathVariant.setValue<QDBusObjectPath>(objectPath);
	lengthVariant.setValue<qlonglong>(m->track.durationMs() * 1000);

	auto title = m->track.title();
	if(title.isEmpty())
	{
		title = Lang::get(Lang::UnknownTitle);
	}

	auto album = m->track.album();
	if(album.isEmpty())
	{
		album = Lang::get(Lang::UnknownAlbum);
	}

	auto artist = m->track.artist();
	if(artist.isEmpty())
	{
		artist = Lang::get(Lang::UnknownArtist);
	}

	auto albumArtist = m->track.albumArtist();
	if(albumArtist.isEmpty())
	{
		albumArtist = Lang::get(Lang::UnknownArtist);
	}

	map["mpris:artUrl"] = QUrl::fromLocalFile(m->coverPath).toString();
	map["mpris:length"] = lengthVariant;
	map["mpris:trackid"] = objectPathVariant;

	map["xesam:album"] = album;
	map["xesam:albumArtist"] = albumArtist;
	map["xesam:artist"] = QStringList({artist});

	if(!m->track.comment().isEmpty())
	{
		map["xesam:comment"] = m->track.comment();
	}

	if(m->track.createdDateTime().isValid())
	{
		map["contentCreated"] = m->track.createdDateTime().toString(Qt::ISODate);
	}

	map["xesam:discNumber"] = static_cast<int>(m->track.discnumber());

	if(!m->track.genres().isEmpty())
	{
		map["xesam:genre"] = m->track.genresToList().join(", ");
	}

	map["xesam:trackNumber"] = static_cast<int>(m->track.trackNumber());
	map["xesam:title"] = title;
	map["xesam:userRating"] = (static_cast<int>(m->track.rating()) / 5.0);

	map["sayonara:year"] = static_cast<int>(m->track.year());
	map["sayonara:bitrate"] = static_cast<int>(m->track.bitrate());
	map["sayonara:filesize"] = QVariant::fromValue<int>(static_cast<int>(m->track.filesize()));

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
	auto pl = m->playlistAccessor->playlist(m->playlistAccessor->currentIndex());
	if(!pl)
	{
		return false;
	}

	const auto mode = pl->mode();
	const auto b = Playlist::Mode::isActiveAndEnabled(mode.shuffle()) ||
	               Playlist::Mode::isActiveAndEnabled(mode.repAll());

	return ((b && pl->count() > 0) || (pl->currentTrackIndex() < pl->count() - 1));
}

bool DBusMPRIS::MediaPlayer2::CanGoPrevious()
{
	auto pl = m->playlistAccessor->playlist(m->playlistAccessor->currentIndex());
	if(!pl)
	{
		return false;
	}

	return (pl->currentTrackIndex() > 0) && (pl->count() > 1);
}

bool DBusMPRIS::MediaPlayer2::CanPlay() { return true; }

bool DBusMPRIS::MediaPlayer2::CanPause() { return true; }

bool DBusMPRIS::MediaPlayer2::CanSeek()
{
	const auto& track = m->playManager->currentTrack();
	return track.durationMs() > 0;
}

bool DBusMPRIS::MediaPlayer2::CanControl() { return true; }

void DBusMPRIS::MediaPlayer2::Next() { m->playManager->next(); }

void DBusMPRIS::MediaPlayer2::Previous() { m->playManager->previous(); }

void DBusMPRIS::MediaPlayer2::Pause() { m->playManager->pause(); }

void DBusMPRIS::MediaPlayer2::PlayPause() { m->playManager->playPause(); }

void DBusMPRIS::MediaPlayer2::Stop() { m->playManager->stop(); }

void DBusMPRIS::MediaPlayer2::Play() { m->playManager->play(); }

void DBusMPRIS::MediaPlayer2::Seek(qlonglong offset)
{
	m->playManager->seekRelativeMs(offset / 1000);
}

void DBusMPRIS::MediaPlayer2::SetPosition(const QDBusObjectPath& /*trackId*/, qlonglong position)
{
	m->playManager->seekAbsoluteMs(position / 1000);
}

void DBusMPRIS::MediaPlayer2::OpenUri(const QString& /*uri*/) {}

void DBusMPRIS::MediaPlayer2::SetLoopStatus(const QString loopStatus) // NOLINT(performance-unnecessary-value-param)
{
	const auto playlistMode = loopStatusToPlaylistMode(loopStatus);
	SetSetting(Set::PL_Mode, playlistMode);
	createMessage("LoopStatus", playlistModeToLoopStatus(playlistMode));
}

void DBusMPRIS::MediaPlayer2::SetRate(double /*rate*/) {}

int DBusMPRIS::MediaPlayer2::Rating() { return int(m->track.rating()); }

void DBusMPRIS::MediaPlayer2::SetShuffle(bool shuffle)
{
	auto playlistMode = GetSetting(Set::PL_Mode);
	playlistMode.setShuffle(shuffle);
	SetSetting(Set::PL_Mode, playlistMode);

	createMessage("Shuffle", PlaylistMode::isActiveAndEnabled(playlistMode.shuffle()));
}

void DBusMPRIS::MediaPlayer2::SetVolume(double volume)
{
	m->playManager->setVolume(static_cast<int>(volume * 100));
	m->volume = volume;
}

void DBusMPRIS::MediaPlayer2::IncreaseVolume() { m->playManager->volumeUp(); }

void DBusMPRIS::MediaPlayer2::DecreaseVolume() { m->playManager->volumeDown(); }

void DBusMPRIS::MediaPlayer2::volumeChanged(int volume)
{
	if(!m->initialized)
	{
		init();
	}

	m->volume = (volume / 100.0);

	createMessage("Volume", volume / 100.0);
}

void DBusMPRIS::MediaPlayer2::positionChanged(MilliSeconds pos)
{
	if(!m->initialized)
	{
		init();
	}

	const auto newPosition = static_cast<MicroSeconds>(pos * 1000);
	const auto difference = newPosition - m->pos;

	if(difference < 0 || difference > 1000000)
	{
		emit Seeked(newPosition);
	}

	m->pos = newPosition;
}

void DBusMPRIS::MediaPlayer2::trackIndexChanged(int /*idx*/)
{
	if(!m->initialized)
	{
		init();
	}

	createMessage("CanGoNext", CanGoNext());
	createMessage("CanGoPrevious", CanGoPrevious());
}

void DBusMPRIS::MediaPlayer2::trackChanged(const MetaData& track)
{
	m->track = track;
	m->coverPath = Util::Filepath(Cover::Location::coverLocation(track).preferredPath()).fileystemPath();

	if(!m->initialized)
	{
		init();
	}

	const auto variantMap = Metadata();
	createMessage("Metadata", variantMap);
}

void DBusMPRIS::MediaPlayer2::trackMetadataChanged()
{
	trackChanged(m->playManager->currentTrack());
}

void DBusMPRIS::MediaPlayer2::playstateChanged(const PlayState state)
{
	auto playbackStatus = QString {};
	if(!m->initialized)
	{
		init();
	}

	const auto playlist = m->playlistAccessor->playlist(m->playlistAccessor->currentIndex());
	const auto hasTracks = (playlist != nullptr)
	                       ? playlist->count() > 0
	                       : false;

	createMessage("CanPlay", hasTracks && (state != PlayState::Playing));
	createMessage("CanPause", (state == PlayState::Playing));
	createMessage("PlaybackStatus", getPlaybackStatusString(state));
}
