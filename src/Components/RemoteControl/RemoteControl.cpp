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

#include <QBuffer>
#include <QFile>
#include <QImage>
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
#include <algorithm>
#include <mutex>

using RemoteFunction=std::function<void()>;
using RemoteFunctionInt=std::function<void(int)>;

static QString getInstanceName()
{
	return QHostInfo::localHostName();
}

struct RemoteControl::Private
{
	bool initialized;

	QMap<QByteArray, RemoteFunction>    functionCallMap;
	QMap<QByteArray, RemoteFunctionInt> functionIntCallMap;

	QTimer*				volumeTimer=nullptr;

	QTcpServer*			server=nullptr;
	QTcpSocket*			socket=nullptr;

	RemoteUDPSocket*	udp=nullptr;

	Private() :
		initialized(false)
	{
		volumeTimer = new QTimer();
		volumeTimer->setInterval(100);
		volumeTimer->setSingleShot(true);
	}
};

RemoteControl::RemoteControl(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	m->udp = new RemoteUDPSocket(this);

	connect(m->volumeTimer, &QTimer::timeout, this, &RemoteControl::volumeTimerTimeout);

	ListenSetting(Set::Remote_Active, RemoteControl::activeChanged);
}

RemoteControl::~RemoteControl() = default;

void RemoteControl::activeChanged()
{
	m->server = new QTcpServer(this);

	if(GetSetting(Set::Remote_Active))
	{
		auto port = quint16(GetSetting(Set::Remote_Port));
		bool success = m->server->listen(QHostAddress::AnyIPv4, port);
		if(!success){
			spLog(Log::Warning, this) << "Cannot listen on port " << port << ": " << m->server->errorString();
		}
	}

	connect(m->server, &QTcpServer::newConnection, this, &RemoteControl::newConnection);
}

void RemoteControl::init()
{
	if(m->initialized){
		return;
	}

	auto* pm = PlayManager::instance();

	m->functionCallMap["play"] =		std::bind(&PlayManager::play, pm);
	m->functionCallMap["pause"] =		std::bind(&PlayManager::pause, pm);
	m->functionCallMap["prev"] =		std::bind(&PlayManager::previous, pm);
	m->functionCallMap["next"] =		std::bind(&PlayManager::next, pm);
	m->functionCallMap["playpause"] =	std::bind(&PlayManager::playPause, pm);
	m->functionCallMap["stop"] =		std::bind(&PlayManager::stop, pm);
	m->functionCallMap["volup"] =		std::bind(&PlayManager::volumeUp, pm);
	m->functionCallMap["voldown"] =		std::bind(&PlayManager::volumeDown, pm);
	m->functionCallMap["state"] =		std::bind(&RemoteControl::requestState, this);
	m->functionCallMap["pl"] =			std::bind(&RemoteControl::writePlaylist, this);
	m->functionCallMap["curSong"] =		std::bind(&RemoteControl::writeCurrentTrack, this);
	m->functionCallMap["idAndName"] =	std::bind(&RemoteControl::writeSayonaraIdAndName, this);
	m->functionCallMap["help"] =		std::bind(&RemoteControl::showApi, this);

	m->functionIntCallMap["setvol"] =  std::bind(&RemoteControl::setVolume, this, std::placeholders::_1);
	m->functionIntCallMap["seekrel"] = std::bind(&RemoteControl::seekRelative, this, std::placeholders::_1);
	m->functionIntCallMap["seekrelms"] =std::bind(&RemoteControl::seekRelativeMs, this, std::placeholders::_1);
	m->functionIntCallMap["seekabsms"] =std::bind(&RemoteControl::seekAbsoluteMs, this, std::placeholders::_1);
	m->functionIntCallMap["chtrk"] =   std::bind(&RemoteControl::changeTrack, this, std::placeholders::_1);

	ListenSettingNoCall(Set::Remote_Active, RemoteControl::remoteActiveChanged);
	ListenSettingNoCall(Set::Remote_Port, RemoteControl::remotePortChanged);
	ListenSettingNoCall(Set::Broadcast_Port, RemoteControl::broadcastChanged);
	ListenSettingNoCall(Set::Broadcast_Active, RemoteControl::broadcastChanged);

	m->initialized = true;
}

bool RemoteControl::isConnected() const
{
	if(!GetSetting(Set::Remote_Active)){
		return false;
	}

	if(!m->socket){
		return false;
	}

	if( !m->socket->isOpen() ||
		!m->socket->isValid() ||
		!m->socket->isWritable())
	{
		return false;
	}

	return true;
}

void RemoteControl::newConnection()
{
	if(!m->initialized){
		init();
	}

	m->socket = m->server->nextPendingConnection();
	if(!m->socket){
		return;
	}

	spLog(Log::Debug, this) << "Got new connection";

	connect(m->socket, &QTcpSocket::readyRead, this, &RemoteControl::newRequest);
	connect(m->socket, &QTcpSocket::disconnected, this, &RemoteControl::socketDisconnected);

	auto* pm = PlayManager::instance();
	auto* plh = Playlist::Handler::instance();

	connect(pm, &PlayManager::sigPositionChangedMs, this, &RemoteControl::currentPositionChangedMs);
	connect(pm, &PlayManager::sigCurrentTrackChanged, this, &RemoteControl::currentTrackChanged);
	connect(pm, &PlayManager::sigVolumeChanged, this, &RemoteControl::volumeChanged);
	connect(pm, &PlayManager::sigPlaystateChanged, this, &RemoteControl::playstateChanged);
	connect(plh, &Playlist::Handler::sigActivePlaylistChanged, this, &RemoteControl::activePlaylistChanged);

	activePlaylistChanged(plh->activeIndex());
}

void RemoteControl::socketDisconnected()
{
	auto* pm = PlayManager::instance();
	auto* plh = Playlist::Handler::instance();

	disconnect(pm, &PlayManager::sigPositionChangedMs, this, &RemoteControl::currentPositionChangedMs);
	disconnect(pm, &PlayManager::sigCurrentTrackChanged, this, &RemoteControl::currentTrackChanged);
	disconnect(pm, &PlayManager::sigVolumeChanged, this, &RemoteControl::volumeChanged);
	disconnect(pm, &PlayManager::sigPlaystateChanged, this, &RemoteControl::playstateChanged);
	disconnect(plh, &Playlist::Handler::sigActivePlaylistChanged, this, &RemoteControl::activePlaylistChanged);
}


void RemoteControl::newRequest()
{
	QByteArray arr = m->socket->readAll();
	arr = arr.left(arr.size() - 1);

	if(m->functionCallMap.contains(arr))
	{
		auto fn = m->functionCallMap[arr];
		fn();
		return;
	}

	int idx = arr.indexOf(' ');
	if(idx == -1){
		return;
	}

	QByteArray cmd = arr.left(idx);
	if(m->functionIntCallMap.contains(cmd))
	{
		int val = extractParameterInt(arr, cmd.size());
		RemoteFunctionInt fn = m->functionIntCallMap[cmd];
		fn(val);
	}
}


int RemoteControl::extractParameterInt(const QByteArray& data, int cmd_len)
{
	return data.right(data.size() - cmd_len - 1).toInt();
}


void RemoteControl::remoteActiveChanged()
{
	bool active = GetSetting(Set::Remote_Active);

	if(!active){
		m->socket->disconnectFromHost();
		m->server->close();
	}

	else if(m->server->isListening()) {
		return;
	}

	else {
		auto port = quint16(GetSetting(Set::Remote_Port));
		m->server->listen(QHostAddress::Any, port);
	}
}

void RemoteControl::remotePortChanged()
{
	auto port = quint16(GetSetting(Set::Remote_Port));
	bool active = GetSetting(Set::Remote_Active);

	if(!active){
		return;
	}

	if(port != m->socket->localPort())
	{
		m->socket->disconnectFromHost();
		m->server->close();
		m->server->listen(QHostAddress::Any, port);
	}
}

void RemoteControl::broadcastChanged()
{
	if(!isConnected()){
		return;
	}

	writeBroadcastInfo();
}

void RemoteControl::setVolume(int vol)
{
	PlayManager::instance()->setVolume(vol);
}

void RemoteControl::seekRelative(int percent)
{
	percent = std::min(percent, 100);
	percent = std::max(percent, 0);
	PlayManager::instance()->seekRelative( percent / 100.0 );
}

void RemoteControl::seekRelativeMs(int pos_ms)
{
	PlayManager::instance()->seekRelativeMs( pos_ms );
}

void RemoteControl::seekAbsoluteMs(int pos_ms)
{
	PlayManager::instance()->seekAbsoluteMs( pos_ms );
}

void RemoteControl::changeTrack(int idx)
{
	auto* plh = Playlist::Handler::instance();
	plh->changeTrack(idx - 1, plh->activeIndex());
}


void RemoteControl::currentPositionChangedMs(MilliSeconds pos)
{
	static MilliSeconds p = 0;
	if(p / 1000 == pos / 1000){
		return;
	}

	p = pos;

	writeCurrentPosition();
}

void RemoteControl::insertJsonCurrentPosition(QJsonObject& obj) const
{
	MilliSeconds pos_ms = PlayManager::instance()->currentPositionMs();
	Seconds pos_sec = Seconds(pos_ms / 1000);

	obj.insert("track-current-position", QJsonValue::fromVariant(
		QVariant::fromValue<Seconds>(pos_sec))
	);
}

void RemoteControl::writeCurrentPosition()
{
	QJsonDocument doc;

	QJsonObject obj;
	insertJsonCurrentPosition(obj);

	doc.setObject(obj);
	write(doc.toBinaryData());
}


void RemoteControl::volumeChanged(int vol)
{
	Q_UNUSED(vol)
	m->volumeTimer->start(100);
}

void RemoteControl::volumeTimerTimeout()
{
	spLog(Log::Debug, this) << "Volume timer timeout";
	writeVolume();
}

void RemoteControl::insertJsonVolume(QJsonObject& obj) const
{
	obj.insert("volume", PlayManager::instance()->volume());
}

void RemoteControl::writeVolume()
{
	QJsonDocument doc;
	QJsonObject obj;
	insertJsonVolume(obj);
	doc.setObject(obj);
	write(doc.toBinaryData());
}


void RemoteControl::currentTrackChanged(const MetaData& md)
{
	Q_UNUSED(md)
	writeCurrentTrack();
}

void RemoteControl::insertJsonCurrentTrack(QJsonObject& o)
{
	auto* plh = Playlist::Handler::instance();

	MetaData md = PlayManager::instance()->currentTrack();

	PlaylistConstPtr pl = plh->playlist(plh->activeIndex());
	if(!pl){
		return;
	}

	int currentTrackIdx = pl->currentTrackIndex();

	spLog(Log::Debug, this) << "Send cur track idx: " << currentTrackIdx;

	o.insert("playlist-current-index", currentTrackIdx);
	o.insert("track-title", md.title());
	o.insert("track-artist", md.artist());
	o.insert("track-album", md.album());
	o.insert("track-total-time", QJsonValue::fromVariant(
		QVariant::fromValue<Seconds>(Seconds(md.durationMs() / 1000)))
	);
}

void RemoteControl::writeCurrentTrack()
{
	PlayState playstate = PlayManager::instance()->playstate();
	if(playstate == PlayState::Stopped)
	{
		writePlaystate();
		return;
	}

	QJsonDocument doc;
	QJsonObject obj;

	insertJsonPlaystate(obj);
	insertJsonCurrentTrack(obj);

	doc.setObject(obj);

	write(doc.toBinaryData());

	searchCover();
}

void RemoteControl::insertJsonSayonaraIdAndName(QJsonObject &obj) const
{
	obj.insert("sayonara-id", QString(GetSetting(Set::Player_PublicId)));
	obj.insert("sayonara-name", getInstanceName());
}

void RemoteControl::writeSayonaraIdAndName()
{
	QJsonDocument doc;
	QJsonObject obj;
	insertJsonSayonaraIdAndName(obj);
	doc.setObject(obj);
	write(doc.toBinaryData());
}

void RemoteControl::jsonCover(QJsonObject& o, const QPixmap& pm) const
{
	if(pm.isNull()){
		return;
	}

	QPixmap pm_scaled = pm.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	o.insert("cover-width", pm_scaled.width());
	o.insert("cover-height", pm_scaled.height());

	QByteArray img_data = Util::convertPixmapToByteArray(pm_scaled);
	QString data = QString::fromLocal8Bit(img_data.toBase64());

	spLog(Log::Debug, this) << "Send " << data.size() << " bytes cover info";
	o.insert("cover-data", data);
}

void RemoteControl::playstateChanged(PlayState playstate)
{
	if(playstate == PlayState::Playing) {
		requestState();
	}

	else
	{
		writePlaystate();
	}
}

void RemoteControl::insertJsonPlaystate(QJsonObject& o)
{
	PlayState playstate = PlayManager::instance()->playstate();

	if(playstate == PlayState::Playing){
		o.insert("playstate", "playing");
	}

	else if(playstate == PlayState::Paused){
		o.insert("playstate", "paused");
	}

	else if(playstate == PlayState::Stopped){
		o.insert("playstate", "stopped");
	}
}

void RemoteControl::writePlaystate()
{
	QJsonDocument doc;
	QJsonObject o;
	insertJsonPlaystate(o);
	doc.setObject(o);
	write(doc.toBinaryData());
}

void RemoteControl::activePlaylistChanged(int index)
{
	auto* plh = Playlist::Handler::instance();
	if(index >= 0 && index < plh->count())
	{
		PlaylistConstPtr pl = plh->playlist(index);
		if(pl)
		{
			connect(pl.get(), &Playlist::Playlist::sigItemsChanged, this, &RemoteControl::activePlaylistContentChanged);
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
	MetaData md = PlayManager::instance()->currentTrack();
	Cover::Location cl = Cover::Location::coverLocation(md);

	auto* cover_lookup = new Cover::Lookup(cl, 1, nullptr);
	connect(cover_lookup, &Cover::Lookup::sigCoverFound, this, &RemoteControl::coverFound);
	connect(cover_lookup, &Cover::Lookup::sigFinished, cover_lookup, &QObject::deleteLater);

	cover_lookup->start();
}

void RemoteControl::coverFound(const QPixmap& pm)
{
	QJsonDocument doc;
	QJsonObject obj;
	jsonCover(obj, pm);

	if(!obj.isEmpty())
	{
		doc.setObject(obj);
		write(doc.toBinaryData());
	}
}

void RemoteControl::insertJsonPlaylist(QJsonArray& arr) const
{
	QByteArray data;

	auto* plh = Playlist::Handler::instance();
	PlaylistConstPtr pl = plh->playlist(plh->activeIndex());
	if(pl)
	{
		int i=1;
		for(const MetaData& md : pl->tracks())
		{
			QJsonObject obj;

			obj.insert("pl-track-title", md.title());
			obj.insert("pl-track-artist", md.artist());
			obj.insert("pl-track-album", md.album());
			obj.insert("pl-track-total-time", QJsonValue::fromVariant(
				QVariant::fromValue<Seconds>(Seconds(md.durationMs() / 1000)))
			);

			arr.append(obj);

			i++;
		}
	}
}

void RemoteControl::writePlaylist()
{
	QJsonDocument doc;
	QJsonObject obj;

	auto* plh = Playlist::Handler::instance();
	MetaData md = PlayManager::instance()->currentTrack();
	PlaylistConstPtr pl = plh->playlist(plh->activeIndex());
	if(pl)
	{
		int cur_trackIdx = pl->currentTrackIndex();
		obj.insert("playlist-current-index", cur_trackIdx);
	}

	QJsonArray arr;
	insertJsonPlaylist(arr);
	if(arr.isEmpty()){
		return;
	}

	obj.insert("playlist", arr);
	doc.setObject(obj);

	write(doc.toBinaryData());
}


void RemoteControl::insertJsonBroadcastInfo(QJsonObject& obj)
{
	obj.insert("broadcast-active", GetSetting(Set::Broadcast_Active));
	obj.insert("broadcast-port", GetSetting(Set::Broadcast_Port));
}

void RemoteControl::writeBroadcastInfo()
{
	QJsonDocument doc;
	QJsonObject obj;
	insertJsonBroadcastInfo(obj);

	doc.setObject(obj);
	write(doc.toBinaryData());
}

static std::mutex mtx;


void RemoteControl::write(const QByteArray& data)
{
	if(m->socket && m->socket->isOpen())
	{
		std::lock_guard<std::mutex> lock(mtx); Q_UNUSED(lock);
		m->socket->write(data + "ENDMESSAGE");
		m->socket->flush();
	}
}


void RemoteControl::requestState()
{
	spLog(Log::Debug, this) << "Current state requested";

	QJsonDocument doc;
	QJsonObject obj;

	insertJsonVolume(obj);
	insertJsonCurrentPosition(obj);
	insertJsonCurrentTrack(obj);
	insertJsonPlaystate(obj);
	insertJsonBroadcastInfo(obj);
	insertJsonSayonaraIdAndName(obj);

	doc.setObject(obj);
	spLog(Log::Info, this) << QString::fromLocal8Bit(doc.toJson());

	write(doc.toBinaryData());

	writePlaylist();
	searchCover();
}

void RemoteControl::showApi()
{
	if(!m->socket || !m->socket->isOpen()){
		return;
	}

	m->socket->write("\n");

	for(auto it=m->functionCallMap.cbegin(); it!=m->functionCallMap.cend(); it++)
	{
		m->socket->write(it.key() + "\n");
	}

	m->socket->write("\n");

	for(auto it=m->functionCallMap.cbegin(); it!=m->functionCallMap.cend(); it++)
	{
		m->socket->write(it.key() + "( value )\n");
	}

	m->socket->write("\n");
}
