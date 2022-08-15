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
	constexpr const auto PropertyCanGoNext = "CanGoNext";
	constexpr const auto PropertyCanGoPrevious = "CanGoPrevious";
	constexpr const auto PropertyCanPause = "CanPause";
	constexpr const auto PropertyCanPlay = "CanPlay";
	constexpr const auto PropertyCanSeek = "CanSeek";
	constexpr const auto PropertyDesktopEntry = "DesktopEntry";
	constexpr const auto PropertyLoopStatus = "LoopStatus";
	constexpr const auto PropertyMetadata = "Metadata";
	constexpr const auto PropertyPlaybackStatus = "PlaybackStatus";
	constexpr const auto PropertyShuffle = "Shuffle";
	constexpr const auto PropertyVolume = "Volume";

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

	Private(QMainWindow* player, PlayManager* playManager, PlaylistAccessor* playlistAccessor) :
		playManager(playManager),
		player(player),
		playlistAccessor(playlistAccessor),
		pos(playManager->currentPositionMs() * 1000) {} // NOLINT(readability-magic-numbers)
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

	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &DBusMPRIS::MediaPlayer2::playstateChanged);
	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &DBusMPRIS::MediaPlayer2::trackChanged);
	connect(m->playManager, &PlayManager::sigTrackIndexChanged, this, &DBusMPRIS::MediaPlayer2::trackIndexChanged);
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &DBusMPRIS::MediaPlayer2::positionChanged);
	connect(m->playManager, &PlayManager::sigVolumeChanged, this, &DBusMPRIS::MediaPlayer2::volumeChanged);
	connect(m->playManager, &PlayManager::sigCurrentMetadataChanged, this, [&]() {
		trackChanged(m->playManager->currentTrack());
	});

	trackChanged(m->playManager->currentTrack());
}

DBusMPRIS::MediaPlayer2::~MediaPlayer2()
{
	QDBusConnection::sessionBus().unregisterObject(objectPath());
	QDBusConnection::sessionBus().unregisterService(serviceName());
}

void DBusMPRIS::MediaPlayer2::init()
{
	static auto isInitialized = false;
	if(isInitialized)
	{
		return;
	}

	new OrgMprisMediaPlayer2Adaptor(this);
	new OrgMprisMediaPlayer2PlayerAdaptor(this);

	if(!QDBusConnection::sessionBus().registerService(serviceName()))
	{
		spLog(Log::Warning, this) << "Failed to register " << serviceName() << " on the session bus";
	}

	else
	{
		spLog(Log::Info, this) << serviceName() << " registered";

		QDBusConnection::sessionBus().registerObject(objectPath(), this);
		createMessage(PropertyDesktopEntry, QString("sayonara"));
	}

	isInitialized = true;
}

bool DBusMPRIS::MediaPlayer2::CanQuit() const { return true; } // NOLINT(readability-convert-member-functions-to-static)

bool DBusMPRIS::MediaPlayer2::CanRaise() { return true; } // NOLINT(readability-convert-member-functions-to-static)

bool DBusMPRIS::MediaPlayer2::HasTrackList() { return false; } // NOLINT(readability-convert-member-functions-to-static)

QString
DBusMPRIS::MediaPlayer2::Identity() { return "Sayonara Player"; } // NOLINT(readability-convert-member-functions-to-static)

QString
DBusMPRIS::MediaPlayer2::DesktopEntry() { return "com.sayonara-player.Sayonara"; } // NOLINT(readability-convert-member-functions-to-static)

QStringList DBusMPRIS::MediaPlayer2::SupportedUriSchemes() // NOLINT(readability-convert-member-functions-to-static)
{
	return {
		"file",
		"http",
		"cdda",
		"smb",
		"sftp"
	};
}

QStringList DBusMPRIS::MediaPlayer2::SupportedMimeTypes() // NOLINT(readability-convert-member-functions-to-static)
{
	return {
		"audio/mpeg"
		"audio/ogg"
	};
}

bool
DBusMPRIS::MediaPlayer2::CanSetFullscreen() { return true; } // NOLINT(readability-convert-member-functions-to-static)

bool
DBusMPRIS::MediaPlayer2::Fullscreen() { return GetSetting(Set::Player_Fullscreen); } // NOLINT(readability-convert-member-functions-to-static)

void DBusMPRIS::MediaPlayer2::SetFullscreen(bool b)// NOLINT(readability-convert-member-functions-to-static)
{
	SetSetting(Set::Player_Fullscreen, b);
}

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::Quit()
{
	m->player->setProperty("shutdown", true);
	m->player->close();
}

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::Raise()
{
	spLog(Log::Debug, this) << "Raise";

	constexpr const auto Timeout = 200;

	const auto geometry = GetSetting(Set::Player_Geometry);
	if(m->player->isMinimized())
	{
		QTimer::singleShot(Timeout, [=]() {
			m->player->showNormal();
		});
	}
	else
	{
		QTimer::singleShot(Timeout, [=]() {
			m->player->restoreGeometry(geometry);
			m->player->showNormal();
		});
	}
}

/*** mpris.mediaplayer2.player ***/

bool DBusMPRIS::MediaPlayer2::CanControl() { return true; } // NOLINT(readability-convert-member-functions-to-static)

bool DBusMPRIS::MediaPlayer2::CanGoNext()
{
	const auto playlist = m->playlistAccessor->playlist(m->playlistAccessor->currentIndex());
	if(!playlist)
	{
		return false;
	}

	const auto playlistMode = playlist->mode();
	const auto isShuffleOrRepeat = PlaylistMode::isActiveAndEnabled(playlistMode.shuffle()) ||
	                               PlaylistMode::isActiveAndEnabled(playlistMode.repAll());

	return (isShuffleOrRepeat && playlist->count() > 0) ||
	       (playlist->currentTrackIndex() < playlist->count() - 1);
}

bool DBusMPRIS::MediaPlayer2::CanGoPrevious()
{
	const auto playlist = m->playlistAccessor->playlist(m->playlistAccessor->currentIndex());
	return (playlist != nullptr)
	       ? (playlist->currentTrackIndex() > 0) && (playlist->count() > 1)
	       : false;
}

bool DBusMPRIS::MediaPlayer2::CanPause() { return true; } // NOLINT(readability-convert-member-functions-to-static)

bool DBusMPRIS::MediaPlayer2::CanPlay() { return true; } // NOLINT(readability-convert-member-functions-to-static)

bool DBusMPRIS::MediaPlayer2::CanSeek()
{
	const auto& track = m->playManager->currentTrack();
	return track.durationMs() > 0;
}

void DBusMPRIS::MediaPlayer2::Next() { m->playManager->next(); }

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::Previous() { m->playManager->previous(); }

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::Pause() { m->playManager->pause(); }

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::PlayPause() { m->playManager->playPause(); }

void DBusMPRIS::MediaPlayer2::Stop() { m->playManager->stop(); }

void DBusMPRIS::MediaPlayer2::Play() { m->playManager->play(); }

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::Seek(const qlonglong offset)
{
	m->playManager->seekRelativeMs(offset / 1000); // NOLINT(readability-magic-numbers)
}

qlonglong DBusMPRIS::MediaPlayer2::Position() { return m->pos; }

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::SetPosition(const QDBusObjectPath& /*trackId*/, const qlonglong position)
{
	m->playManager->seekAbsoluteMs(position / 1000); // NOLINT(readability-magic-numbers)
}

void DBusMPRIS::MediaPlayer2::positionChanged(const MilliSeconds pos)
{
	init();

	const auto newPosition = static_cast<MicroSeconds>(pos * 1000);
	const auto difference = newPosition - m->pos;

	constexpr const auto OneSecond = 1'000'000;
	if(difference < 0 || difference > OneSecond)
	{
		emit Seeked(newPosition);
	}

	m->pos = newPosition;
}

void DBusMPRIS::MediaPlayer2::trackIndexChanged(const int /*idx*/)
{
	init();

	createMessage(PropertyCanGoNext, CanGoNext());
	createMessage(PropertyCanGoPrevious, CanGoPrevious());
}

QVariantMap DBusMPRIS::MediaPlayer2::Metadata()
{
	auto map = QVariantMap {};

	map["mpris:artUrl"] = QUrl::fromLocalFile(m->coverPath).toString();
	map["mpris:length"] = QVariant::fromValue<qlonglong>(
		m->track.durationMs() * 1000); // NOLINT(readability-magic-numbers)
	map["mpris:trackid"] = QVariant::fromValue(createObjectPath(m->track.id()));

	map["xesam:album"] = checkString(m->track.album(), Lang::UnknownAlbum);
	map["xesam:albumArtist"] = checkString(m->track.albumArtist(), Lang::UnknownArtist);
	map["xesam:artist"] = QStringList({checkString(m->track.artist(), Lang::UnknownArtist)});

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
	map["xesam:title"] = checkString(m->track.title(), Lang::UnknownTitle);
	map["xesam:userRating"] = (static_cast<int>(m->track.rating()) / 5.0); // NOLINT(readability-magic-numbers)

	map["sayonara:year"] = static_cast<int>(m->track.year());
	map["sayonara:bitrate"] = static_cast<int>(m->track.bitrate());
	map["sayonara:filesize"] = QVariant::fromValue<int>(static_cast<int>(m->track.filesize()));

	return map;
}

void DBusMPRIS::MediaPlayer2::trackChanged(const MetaData& track)
{
	m->track = track;
	m->coverPath = Util::Filepath(Cover::Location::coverLocation(track).preferredPath()).fileystemPath();

	init();

	createMessage(PropertyMetadata, Metadata());
	createMessage(PropertyCanSeek, CanSeek());
}

QString DBusMPRIS::MediaPlayer2::LoopStatus() // NOLINT(readability-convert-member-functions-to-static)
{
	return playlistModeToLoopStatus(GetSetting(Set::PL_Mode));
}

void DBusMPRIS::MediaPlayer2::SetLoopStatus(const QString loopStatus) // NOLINT(performance-unnecessary-value-param)
{
	const auto playlistMode = loopStatusToPlaylistMode(loopStatus);
	SetSetting(Set::PL_Mode, playlistMode);
	createMessage(PropertyLoopStatus, playlistModeToLoopStatus(playlistMode));
}

QString DBusMPRIS::MediaPlayer2::PlaybackStatus() { return getPlaybackStatusString(m->playManager->playstate()); }

void DBusMPRIS::MediaPlayer2::playstateChanged(const PlayState state)
{
	init();

	const auto playlist = m->playlistAccessor->playlist(m->playlistAccessor->currentIndex());
	const auto hasTracks = (playlist != nullptr)
	                       ? playlist->count() > 0
	                       : false;

	createMessage(PropertyCanPlay, hasTracks && (state != PlayState::Playing));
	createMessage(PropertyCanPause, (state == PlayState::Playing));
	createMessage(PropertyPlaybackStatus, PlaybackStatus());
}

bool DBusMPRIS::MediaPlayer2::Shuffle() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto playlistMode = GetSetting(Set::PL_Mode);
	return PlaylistMode::isActiveAndEnabled(playlistMode.shuffle());
}

void DBusMPRIS::MediaPlayer2::SetShuffle(const bool shuffle) // NOLINT(readability-convert-member-functions-to-static)
{
	auto playlistMode = GetSetting(Set::PL_Mode);
	playlistMode.setShuffle(shuffle);
	SetSetting(Set::PL_Mode, playlistMode);

	createMessage(PropertyShuffle, PlaylistMode::isActiveAndEnabled(playlistMode.shuffle()));
}

double DBusMPRIS::MediaPlayer2::Volume() { return m->volume; }

void DBusMPRIS::MediaPlayer2::SetVolume(const double volume)
{
	m->playManager->setVolume(static_cast<int>(volume * 100)); // NOLINT(readability-magic-numbers)
	m->volume = volume;
}

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::IncreaseVolume() { m->playManager->volumeUp(); }

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::DecreaseVolume() { m->playManager->volumeDown(); }

void DBusMPRIS::MediaPlayer2::volumeChanged(const int volume)
{
	init();

	m->volume = (volume / 100.0);
	createMessage(PropertyVolume, m->volume);
}

int DBusMPRIS::MediaPlayer2::Rating() { return static_cast<int>(m->track.rating()); }

double DBusMPRIS::MediaPlayer2::MinimumRate() { return 1.0; } // NOLINT(readability-convert-member-functions-to-static)

double DBusMPRIS::MediaPlayer2::MaximumRate() { return 1.0; } // NOLINT(readability-convert-member-functions-to-static)

double DBusMPRIS::MediaPlayer2::Rate() { return 1.0; } // NOLINT(readability-convert-member-functions-to-static)

void DBusMPRIS::MediaPlayer2::SetRate(const double /*rate*/) {}

[[maybe_unused]] void DBusMPRIS::MediaPlayer2::OpenUri(const QString& /*uri*/) {}
