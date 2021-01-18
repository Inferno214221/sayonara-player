/* RemoteControl.cpp */

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

#include "RemoteControl.h"
#include "RemoteControl/UDPSocket.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Covers/CoverLookup.h"

#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

#include <QPixmap>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QByteArray>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

#include <functional>
#include <mutex>

using RemoteFunction = std::function<void()>;
using RemoteFunctionInt = std::function<void(int)>;

namespace
{
	int extractParameterInt(const QByteArray& data, int len)
	{
		return data.right(data.size() - len - 1).toInt();
	}

	void insertJsonSayonaraIdAndName(QJsonObject& jsonObject)
	{
		jsonObject.insert("sayonara-id", QString(GetSetting(Set::Player_PublicId)));
		jsonObject.insert("sayonara-name", QHostInfo::localHostName());
	}

	void insertJsonBroadcastInfo(QJsonObject& jsonObject)
	{
		jsonObject.insert("broadcast-active", GetSetting(Set::Broadcast_Active));
		jsonObject.insert("broadcast-port", GetSetting(Set::Broadcast_Port));
	}

	void insertJsonCurrentTrack(QJsonObject& jsonObject, Playlist::Handler* playlistHandler, PlayManager* playManager)
	{
		const auto playlist = playlistHandler->playlist(playlistHandler->activeIndex());
		if(!playlist)
		{
			return;
		}

		const auto& currentTrack = playManager->currentTrack();
		const auto currentTrackIdx = playlist->currentTrackIndex();

		spLog(Log::Debug, "RemoteControl") << "Send cur track idx: " << currentTrackIdx;

		jsonObject.insert("playlist-current-index", currentTrackIdx);
		jsonObject.insert("track-title", currentTrack.title());
		jsonObject.insert("track-artist", currentTrack.artist());
		jsonObject.insert("track-album", currentTrack.album());
		jsonObject.insert("track-total-time", QJsonValue::fromVariant(
			QVariant::fromValue<Seconds>(Seconds(currentTrack.durationMs() / 1000)))
		);
	}

	void insertJsonVolume(QJsonObject& jsonObject, PlayManager* playManager)
	{
		jsonObject.insert("volume", playManager->volume());
	}

	void insertJsonCurrentPosition(QJsonObject& jsonObject, PlayManager* playManager)
	{
		const auto positionMs = playManager->currentPositionMs();
		const auto positionSec = static_cast<Seconds>(positionMs / 1000);

		jsonObject.insert("track-current-position", QJsonValue::fromVariant(
			QVariant::fromValue<Seconds>(positionSec))
		);
	}

	void insertJsonPlaylist(QJsonArray& jsonArray, Playlist::Handler* playlistHandler)
	{
		const auto playlist = playlistHandler->playlist(playlistHandler->activeIndex());
		if(playlist)
		{
			for(const auto& track : playlist->tracks())
			{
				QJsonObject obj;

				obj.insert("pl-track-title", track.title());
				obj.insert("pl-track-artist", track.artist());
				obj.insert("pl-track-album", track.album());
				obj.insert("pl-track-total-time", QJsonValue::fromVariant(
					QVariant::fromValue<Seconds>(Seconds(track.durationMs() / 1000)))
				);

				jsonArray.append(obj);
			}
		}
	}

	void insertJsonPlaystate(QJsonObject& jsonObject, PlayManager* playManager)
	{
		switch(playManager->playstate())
		{
			case PlayState::Playing:
				jsonObject.insert("playstate", "playing");
				return;
			case PlayState::Paused:
				jsonObject.insert("playstate", "paused");
				return;
			case PlayState::Stopped:
				jsonObject.insert("playstate", "stopped");
				return;
			default:
				return;
		}
	}

	void insertJsonCover(QJsonObject& jsonObject, const QPixmap& pixmap)
	{
		if(pixmap.isNull())
		{
			return;
		}

		const auto pixmapScaled = pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		jsonObject.insert("cover-width", pixmapScaled.width());
		jsonObject.insert("cover-height", pixmapScaled.height());

		const auto imageData = Util::convertPixmapToByteArray(pixmapScaled);
		const auto imageData8Bit = QString::fromLocal8Bit(imageData.toBase64());

		spLog(Log::Debug, "RemoteControl") << "Send " << imageData8Bit.size() << " bytes cover info";
		jsonObject.insert("cover-data", imageData8Bit);
	}

	void insertState(QJsonObject& jsonObject, Playlist::Handler* playlistHandler, PlayManager* playManager)
	{
		insertJsonVolume(jsonObject, playManager);
		insertJsonCurrentPosition(jsonObject, playManager);
		insertJsonCurrentTrack(jsonObject, playlistHandler, playManager);
		insertJsonPlaystate(jsonObject, playManager);
		insertJsonBroadcastInfo(jsonObject);
		insertJsonSayonaraIdAndName(jsonObject);
	}

	static std::mutex mtx;

	void write(const QJsonObject& jsonObject, QTcpSocket* socket)
	{
		if(socket && socket->isOpen())
		{
			std::lock_guard<std::mutex> lock(mtx);
			Q_UNUSED(lock);

			QJsonDocument jsonDocument;
			jsonDocument.setObject(jsonObject);

			const auto data = jsonDocument.toBinaryData();
			socket->write(data + "ENDMESSAGE");
			socket->flush();
		}
	}
}

struct RemoteControl::Private
{
	QMap<QByteArray, RemoteFunction> functionCallMap;
	QMap<QByteArray, RemoteFunctionInt> functionIntCallMap;

	QTcpServer* server = nullptr;
	QTcpSocket* socket = nullptr;

	Playlist::Handler* playlistHandler;
	PlayManager* playManager;
	QTimer* volumeTimer;
	RemoteUDPSocket* udp;
	bool initialized{false};

	Private(RemoteControl* parent, Playlist::Handler* playlistHandler, PlayManager* playManager) :
		playlistHandler(playlistHandler),
		playManager(playManager),
		volumeTimer(new QTimer {}),
		udp(new RemoteUDPSocket {parent})
	{
		volumeTimer->setInterval(100);
		volumeTimer->setSingleShot(true);
	}
};

RemoteControl::RemoteControl(Playlist::Handler* playlistHandler, PlayManager* playManager, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this, playlistHandler, playManager);

	connect(m->volumeTimer, &QTimer::timeout, this, [=]() { writeVolume(); });

	ListenSetting(Set::Remote_Active, RemoteControl::activeChanged);
}

RemoteControl::~RemoteControl() = default;

void RemoteControl::activeChanged()
{
	m->server = new QTcpServer(this);

	if(GetSetting(Set::Remote_Active))
	{
		const auto port = GetSetting(Set::Remote_Port);
		const auto success = m->server->listen(QHostAddress::AnyIPv4, static_cast<quint16>(port));
		if(!success)
		{
			spLog(Log::Warning, this) << "Cannot listen on port " << port << ": " << m->server->errorString();
			return;
		}
	}

	connect(m->server, &QTcpServer::newConnection, this, &RemoteControl::newConnection);
}

void RemoteControl::init()
{
	m->functionCallMap["play"] = std::bind(&PlayManager::play, m->playManager);
	m->functionCallMap["pause"] = std::bind(&PlayManager::pause, m->playManager);
	m->functionCallMap["prev"] = std::bind(&PlayManager::previous, m->playManager);
	m->functionCallMap["next"] = std::bind(&PlayManager::next, m->playManager);
	m->functionCallMap["playpause"] = std::bind(&PlayManager::playPause, m->playManager);
	m->functionCallMap["stop"] = std::bind(&PlayManager::stop, m->playManager);
	m->functionCallMap["volup"] = std::bind(&PlayManager::volumeUp, m->playManager);
	m->functionCallMap["voldown"] = std::bind(&PlayManager::volumeDown, m->playManager);
	m->functionCallMap["state"] = std::bind(&RemoteControl::requestState, this);
	m->functionCallMap["pl"] = std::bind(&RemoteControl::writePlaylist, this);
	m->functionCallMap["curSong"] = std::bind(&RemoteControl::writeCurrentTrack, this);
	m->functionCallMap["idAndName"] = std::bind(&RemoteControl::writeSayonaraIdAndName, this);
	m->functionCallMap["help"] = std::bind(&RemoteControl::showApi, this);

	m->functionIntCallMap["setvol"] = std::bind(&RemoteControl::setVolume, this, std::placeholders::_1);
	m->functionIntCallMap["seekrel"] = std::bind(&RemoteControl::seekRelative, this, std::placeholders::_1);
	m->functionIntCallMap["seekrelms"] = std::bind(&RemoteControl::seekRelativeMs, this, std::placeholders::_1);
	m->functionIntCallMap["seekabsms"] = std::bind(&RemoteControl::seekAbsoluteMs, this, std::placeholders::_1);
	m->functionIntCallMap["chtrk"] = std::bind(&RemoteControl::changeTrack, this, std::placeholders::_1);

	ListenSettingNoCall(Set::Remote_Active, RemoteControl::remoteActiveChanged);
	ListenSettingNoCall(Set::Remote_Port, RemoteControl::remotePortChanged);
	ListenSettingNoCall(Set::Broadcast_Port, RemoteControl::broadcastChanged);
	ListenSettingNoCall(Set::Broadcast_Active, RemoteControl::broadcastChanged);

	m->initialized = true;
}

bool RemoteControl::isConnected() const
{
	return (
		GetSetting(Set::Remote_Active) &&
		m->socket &&
		m->socket->isValid() &&
		m->socket->isOpen() &&
		m->socket->isWritable()
	);
}

void RemoteControl::newConnection()
{
	if(!m->initialized)
	{
		init();
	}

	m->socket = m->server->nextPendingConnection();
	if(!m->socket)
	{
		return;
	}

	spLog(Log::Debug, this) << "Got new connection";

	connect(m->socket, &QTcpSocket::readyRead, this, &RemoteControl::newRequest);
	connect(m->socket, &QTcpSocket::disconnected, this, &RemoteControl::socketDisconnected);

	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &RemoteControl::currentPositionChangedMs);
	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &RemoteControl::currentTrackChanged);
	connect(m->playManager, &PlayManager::sigVolumeChanged, this, &RemoteControl::volumeChanged);
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &RemoteControl::playstateChanged);
	connect(m->playlistHandler,
	        &Playlist::Handler::sigActivePlaylistChanged,
	        this,
	        &RemoteControl::activePlaylistChanged);

	activePlaylistChanged(m->playlistHandler->activeIndex());
}

void RemoteControl::socketDisconnected()
{
	disconnect(m->playManager, &PlayManager::sigPositionChangedMs, this, &RemoteControl::currentPositionChangedMs);
	disconnect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &RemoteControl::currentTrackChanged);
	disconnect(m->playManager, &PlayManager::sigVolumeChanged, this, &RemoteControl::volumeChanged);
	disconnect(m->playManager, &PlayManager::sigPlaystateChanged, this, &RemoteControl::playstateChanged);
	disconnect(m->playlistHandler,
	           &Playlist::Handler::sigActivePlaylistChanged,
	           this,
	           &RemoteControl::activePlaylistChanged);
}

void RemoteControl::newRequest()
{
	auto socketData = m->socket->readAll();
	socketData = socketData.left(socketData.size() - 1);

	if(m->functionCallMap.contains(socketData))
	{
		m->functionCallMap[socketData]();
	}

	else
	{
		const auto index = socketData.indexOf(' ');
		if(index >= 0)
		{
			const auto cmd = socketData.left(index);
			if(m->functionIntCallMap.contains(cmd))
			{
				const auto val = extractParameterInt(socketData, cmd.size());
				m->functionIntCallMap[cmd](val);
			}
		}
	}
}

void RemoteControl::remoteActiveChanged()
{
	if(!GetSetting(Set::Remote_Active))
	{
		m->socket->disconnectFromHost();
		m->server->close();
	}

	else if(!m->server->isListening())
	{
		const auto port = GetSetting(Set::Remote_Port);
		m->server->listen(QHostAddress::Any, static_cast<quint16>(port));
	}
}

void RemoteControl::remotePortChanged()
{
	if(GetSetting(Set::Remote_Active))
	{
		const auto port = GetSetting(Set::Remote_Port);
		if(m->socket->localPort() != static_cast<quint16>(port))
		{
			m->socket->disconnectFromHost();
			m->server->close();
			m->server->listen(QHostAddress::Any, port);
		}
	}
}

void RemoteControl::broadcastChanged()
{
	if(isConnected())
	{
		writeBroadcastInfo();
	}
}

void RemoteControl::setVolume(int volume)
{
	m->playManager->setVolume(volume);
}

void RemoteControl::seekRelative(int percent)
{
	percent = std::min(percent, 100);
	percent = std::max(percent, 0);
	m->playManager->seekRelative(percent / 100.0);
}

void RemoteControl::seekRelativeMs(int positionMs)
{
	m->playManager->seekRelativeMs(positionMs);
}

void RemoteControl::seekAbsoluteMs(int positionMs)
{
	m->playManager->seekAbsoluteMs(positionMs);
}

void RemoteControl::changeTrack(int trackIndex)
{
	trackIndex--;

	auto playlist = m->playlistHandler->activePlaylist();
	if(trackIndex < playlist->count())
	{
		playlist->changeTrack(trackIndex);
	}
}

void RemoteControl::currentPositionChangedMs(MilliSeconds positionMs)
{
	static MilliSeconds p = 0;
	if(p / 1000 != positionMs / 1000)
	{
		p = positionMs;
		writeCurrentPosition();
	}
}

void RemoteControl::writeCurrentPosition()
{
	QJsonObject jsonObject;
	insertJsonCurrentPosition(jsonObject, m->playManager);
	write(jsonObject, m->socket);
}

void RemoteControl::volumeChanged(int /*volume*/)
{
	m->volumeTimer->start(100);
}

void RemoteControl::writeVolume()
{
	QJsonObject jsonObject;
	insertJsonVolume(jsonObject, m->playManager);
	write(jsonObject, m->socket);
}

void RemoteControl::currentTrackChanged(const MetaData& /*track*/)
{
	writeCurrentTrack();
}

void RemoteControl::writeCurrentTrack()
{
	const auto playstate = m->playManager->playstate();
	if(playstate == PlayState::Stopped)
	{
		writePlaystate();
		return;
	}

	QJsonObject jsonObject;
	insertJsonPlaystate(jsonObject, m->playManager);
	insertJsonCurrentTrack(jsonObject, m->playlistHandler, m->playManager);
	write(jsonObject, m->socket);

	searchCover();
}

void RemoteControl::playstateChanged(PlayState playstate)
{
	(playstate == PlayState::Playing) ?
	requestState() :
	writePlaystate();
}

void RemoteControl::writePlaystate()
{
	QJsonObject jsonObject;
	insertJsonPlaystate(jsonObject, m->playManager);
	write(jsonObject, m->socket);
}

void RemoteControl::activePlaylistChanged(int index)
{
	if(index >= 0 && index < m->playlistHandler->count())
	{
		const auto playlist = m->playlistHandler->playlist(index);
		if(playlist)
		{
			connect(playlist.get(),
			        &Playlist::Playlist::sigItemsChanged,
			        this,
			        &RemoteControl::activePlaylistContentChanged);
		}
	}

	writePlaylist();
}

void RemoteControl::activePlaylistContentChanged(int index)
{
	Q_UNUSED(index)
	writePlaylist();
}

void RemoteControl::searchCover()
{
	const auto& track = m->playManager->currentTrack();
	const auto coverLocation = Cover::Location::coverLocation(track);

	auto* coverLookup = new Cover::Lookup(coverLocation, 1, nullptr);
	connect(coverLookup, &Cover::Lookup::sigCoverFound, this, &RemoteControl::coverFound);
	connect(coverLookup, &Cover::Lookup::sigFinished, coverLookup, &QObject::deleteLater);

	coverLookup->start();
}

void RemoteControl::coverFound(const QPixmap& pixmap)
{
	QJsonObject jsonObject;
	insertJsonCover(jsonObject, pixmap);
	write(jsonObject, m->socket);
}

void RemoteControl::writePlaylist()
{
	QJsonObject jsonObject;

	const auto playlist = m->playlistHandler->playlist(m->playlistHandler->activeIndex());
	if(playlist)
	{
		const auto currentTrackIndex = playlist->currentTrackIndex();
		jsonObject.insert("playlist-current-index", currentTrackIndex);
	}

	QJsonArray jsonArray;
	insertJsonPlaylist(jsonArray, m->playlistHandler);
	if(!jsonArray.isEmpty())
	{
		jsonObject.insert("playlist", jsonArray);
		write(jsonObject, m->socket);
	}
}

void RemoteControl::writeBroadcastInfo()
{
	QJsonObject jsonObject;
	insertJsonBroadcastInfo(jsonObject);
	write(jsonObject, m->socket);
}

void RemoteControl::requestState()
{
	QJsonObject jsonObject;
	insertState(jsonObject, m->playlistHandler, m->playManager);
	write(jsonObject, m->socket);

	writePlaylist();
	searchCover();
}

void RemoteControl::writeSayonaraIdAndName()
{
	QJsonObject jsonObject;
	insertJsonSayonaraIdAndName(jsonObject);
	write(jsonObject, m->socket);
}

void RemoteControl::showApi()
{
	if(m->socket && m->socket->isOpen())
	{
		m->socket->write("\n");

		for(auto it = m->functionCallMap.cbegin(); it != m->functionCallMap.cend(); it++)
		{
			m->socket->write(it.key() + "\n");
		}

		m->socket->write("\n");
		for(auto it = m->functionIntCallMap.cbegin(); it != m->functionIntCallMap.cend(); it++)
		{
			m->socket->write(it.key() + "( value )\n");
		}

		m->socket->write("\n");
	}
}
