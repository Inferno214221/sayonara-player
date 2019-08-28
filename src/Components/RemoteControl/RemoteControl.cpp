/* RemoteControl.cpp */

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

#include "RemoteControl.h"
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

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <functional>
#include <algorithm>

using RemoteFunction=std::function<void()>;
using RemoteFunctionInt=std::function<void(int)>;


struct RemoteControl::Private
{
	bool initialized;

	QMap<QByteArray, RemoteFunction>    fn_call_map;
	QMap<QByteArray, RemoteFunctionInt> fn_int_call_map;

	QTcpServer*			server=nullptr;
	QTcpSocket*			socket=nullptr;

	Private() :
		initialized(false)
	{}
};

RemoteControl::RemoteControl(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	ListenSetting(Set::Remote_Active, RemoteControl::active_changed);
}

RemoteControl::~RemoteControl() = default;

void RemoteControl::active_changed()
{
	m->server = new QTcpServer(this);

	if(GetSetting(Set::Remote_Active))
	{
		auto port = quint16(GetSetting(Set::Remote_Port));
		bool success = m->server->listen(QHostAddress::AnyIPv4, port);
		if(!success){
			sp_log(Log::Warning, this) << "Cannot listen on port " << port << ": " << m->server->errorString();
		}
	}

	connect(m->server, &QTcpServer::newConnection, this, &RemoteControl::new_connection);
}

void RemoteControl::init()
{
	if(m->initialized){
		return;
	}

	auto* pm = PlayManager::instance();

	m->fn_call_map["play"] =	[pm]() {pm->play();};
	m->fn_call_map["pause"] =	[pm]() {pm->pause();};
	m->fn_call_map["prev"] =	[pm]() {pm->previous();};
	m->fn_call_map["next"] =	[pm]() {pm->next();};
	m->fn_call_map["playpause"]=[pm]() {pm->play_pause();};
	m->fn_call_map["stop"] =	[pm]() {pm->stop();};
	m->fn_call_map["volup"] =   [pm]() {pm->volume_up();};
	m->fn_call_map["voldown"] =	[pm]() {pm->volume_down();};
	m->fn_call_map["state"] =   std::bind(&RemoteControl::request_state, this);
	m->fn_call_map["pl"] =      std::bind(&RemoteControl::write_playlist, this);
	m->fn_call_map["curSong"] =	std::bind(&RemoteControl::write_current_track, this);
	m->fn_call_map["help"] =		std::bind(&RemoteControl::show_api, this);

	m->fn_int_call_map["setvol"] =  std::bind(&RemoteControl::set_volume, this, std::placeholders::_1);
	m->fn_int_call_map["seekrel"] = std::bind(&RemoteControl::seek_rel, this, std::placeholders::_1);
	m->fn_int_call_map["seekrels"] =std::bind(&RemoteControl::seek_rel_ms, this, std::placeholders::_1);
	m->fn_int_call_map["chtrk"] =   std::bind(&RemoteControl::change_track, this, std::placeholders::_1);

	ListenSettingNoCall(Set::Remote_Active, RemoteControl::_sl_active_changed);
	ListenSettingNoCall(Set::Remote_Port, RemoteControl::_sl_port_changed);
	ListenSettingNoCall(Set::Broadcast_Port, RemoteControl::_sl_broadcast_changed);
	ListenSettingNoCall(Set::Broadcast_Active, RemoteControl::_sl_broadcast_changed);

	m->initialized = true;
}

bool RemoteControl::is_connected() const
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

void RemoteControl::new_connection()
{
	if(!m->initialized){
		init();
	}

	m->socket = m->server->nextPendingConnection();
	if(!m->socket){
		return;
	}

	sp_log(Log::Debug, this) << "Got new connection";

	connect(m->socket, &QTcpSocket::readyRead, this, &RemoteControl::new_request);
	connect(m->socket, &QTcpSocket::disconnected, this, &RemoteControl::socket_disconnected);

	auto* pm = PlayManager::instance();
	auto* plh = Playlist::Handler::instance();

	connect(pm, &PlayManager::sig_position_changed_ms, this, &RemoteControl::pos_changed_ms);
	connect(pm, &PlayManager::sig_track_changed, this, &RemoteControl::track_changed);
	connect(pm, &PlayManager::sig_volume_changed, this, &RemoteControl::volume_changed);
	connect(pm, &PlayManager::sig_playstate_changed, this, &RemoteControl::playstate_changed);
	connect(plh, &Playlist::Handler::sig_active_playlist_changed, this, &RemoteControl::active_playlist_changed);

	active_playlist_changed(plh->active_index());
}

void RemoteControl::socket_disconnected()
{
	auto* pm = PlayManager::instance();
	auto* plh = Playlist::Handler::instance();

	disconnect(pm, &PlayManager::sig_position_changed_ms, this, &RemoteControl::pos_changed_ms);
	disconnect(pm, &PlayManager::sig_track_changed, this, &RemoteControl::track_changed);
	disconnect(pm, &PlayManager::sig_volume_changed, this, &RemoteControl::volume_changed);
	disconnect(pm, &PlayManager::sig_playstate_changed, this, &RemoteControl::playstate_changed);
	disconnect(plh, &Playlist::Handler::sig_active_playlist_changed, this, &RemoteControl::active_playlist_changed);
}


void RemoteControl::new_request()
{
	QByteArray arr = m->socket->readAll();
	arr = arr.left(arr.size() - 1);

	if(m->fn_call_map.contains(arr))
	{
		auto fn = m->fn_call_map[arr];
		fn();
		return;
	}

	int idx = arr.indexOf(' ');
	if(idx == -1){
		return;
	}

	QByteArray cmd = arr.left(idx);
	if(m->fn_int_call_map.contains(cmd))
	{
		int val = extract_parameter_int(arr, cmd.size());
		RemoteFunctionInt fn = m->fn_int_call_map[cmd];
		fn(val);
		return;
	}
}


int RemoteControl::extract_parameter_int(const QByteArray& data, int cmd_len)
{
	return data.right(data.size() - cmd_len - 1).toInt();
}


void RemoteControl::_sl_active_changed()
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

void RemoteControl::_sl_port_changed()
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

void RemoteControl::_sl_broadcast_changed()
{
	if(!is_connected()){
		return;
	}

	write_broadcast_info();
}

void RemoteControl::set_volume(int vol)
{
	PlayManager::instance()->set_volume(vol);
}

void RemoteControl::seek_rel(int percent)
{
	percent = std::min(percent, 100);
	percent = std::max(percent, 0);
	PlayManager::instance()->seek_rel( percent / 100.0 );
}

void RemoteControl::seek_rel_ms(int pos_ms)
{
	PlayManager::instance()->seek_rel_ms( pos_ms );
}

void RemoteControl::change_track(int idx)
{
	auto* plh = Playlist::Handler::instance();
	plh->change_track(idx - 1, plh->active_index());
}


void RemoteControl::pos_changed_ms(MilliSeconds pos)
{
	static MilliSeconds p = 0;
	if(p / 1000 == pos / 1000){
		return;
	}

	p = pos;

	write_current_position();
}

void RemoteControl::json_current_position(QJsonObject& obj) const
{
	MilliSeconds pos_ms = PlayManager::instance()->current_position_ms();
	Seconds pos_sec = Seconds(pos_ms / 1000);

	obj.insert("track-current-position", QJsonValue::fromVariant(
		QVariant::fromValue<Seconds>(pos_sec))
	);
}

void RemoteControl::write_current_position()
{
	QJsonDocument doc;

	QJsonObject obj;
	json_current_position(obj);

	doc.setObject(obj);
	write(doc.toBinaryData());
}


void RemoteControl::volume_changed(int vol)
{
	Q_UNUSED(vol)
	write_volume();
}

void RemoteControl::json_volume(QJsonObject& obj) const
{
	obj.insert("volume", PlayManager::instance()->volume());
}

void RemoteControl::write_volume()
{
	QJsonDocument doc;
	QJsonObject obj;
	json_volume(obj);
	doc.setObject(obj);
	write(doc.toBinaryData());
}


void RemoteControl::track_changed(const MetaData& md)
{
	Q_UNUSED(md)
	write_current_track();
}

void RemoteControl::json_current_track(QJsonObject& o)
{
	auto* plh = Playlist::Handler::instance();

	MetaData md = PlayManager::instance()->current_track();

	PlaylistConstPtr pl = plh->playlist(plh->active_index());
	if(!pl){
		return;
	}

	int cur_track_idx = pl->current_track_index();

	sp_log(Log::Debug, this) << "Send cur track idx: " << cur_track_idx;

	o.insert("playlist-current-index", cur_track_idx);
	o.insert("track-title", md.title());
	o.insert("track-artist", md.artist());
	o.insert("track-album", md.album());
	o.insert("track-total-time", QJsonValue::fromVariant(
		QVariant::fromValue<Seconds>(Seconds(md.duration_ms / 1000)))
	);
}

void RemoteControl::write_current_track()
{
	PlayState playstate = PlayManager::instance()->playstate();
	if(playstate == PlayState::Stopped)
	{
		write_playstate();
		return;
	}

	QJsonDocument doc;
	QJsonObject obj;

	json_playstate(obj);
	json_current_track(obj);

	doc.setObject(obj);

	write(doc.toBinaryData());

	search_cover();
}


void RemoteControl::json_cover(QJsonObject& o, const QPixmap& pm) const
{
	if(pm.isNull()){
		return;
	}

	QPixmap pm_scaled = pm.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	o.insert("cover-width", pm_scaled.width());
	o.insert("cover-height", pm_scaled.height());

	QByteArray img_data = Util::cvt_pixmap_to_bytearray(pm_scaled);
	QString data = QString::fromLocal8Bit(img_data.toBase64());

	sp_log(Log::Debug, this) << "Send " << data.size() << " bytes cover info";
	o.insert("cover-data", data);
}


void RemoteControl::playstate_changed(PlayState playstate)
{
	if(playstate == PlayState::Playing) {
		request_state();
	}

	else
	{
		write_playstate();
	}
}

void RemoteControl::json_playstate(QJsonObject& o)
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

void RemoteControl::write_playstate()
{
	QJsonDocument doc;
	QJsonObject o;
	json_playstate(o);
	doc.setObject(o);
	write(doc.toBinaryData());
}

void RemoteControl::active_playlist_changed(int index)
{
	auto* plh = Playlist::Handler::instance();
	if(index >= 0 && index < plh->count())
	{
		PlaylistConstPtr pl = plh->playlist(index);
		if(pl)
		{
			connect(pl.get(), &Playlist::Playlist::sig_items_changed, this, &RemoteControl::active_playlist_content_changed);
		}
	}

	write_playlist();
}

void RemoteControl::active_playlist_content_changed(int index)
{
	Q_UNUSED(index)
	write_playlist();
}

void RemoteControl::search_cover()
{
	MetaData md = PlayManager::instance()->current_track();
	Cover::Location cl = Cover::Location::cover_location(md);

	auto* cover_lookup = new Cover::Lookup(cl, 1, nullptr);
	connect(cover_lookup, &Cover::Lookup::sig_cover_found, this, &RemoteControl::cover_found);
	connect(cover_lookup, &Cover::Lookup::sig_finished, cover_lookup, &QObject::deleteLater);

	cover_lookup->start();
}

void RemoteControl::cover_found(const QPixmap& pm)
{
	auto* cover_lookup = static_cast<Cover::Lookup*>(sender());

	QJsonDocument doc;
	QJsonObject obj;
	json_cover(obj, pm);

	if(!obj.isEmpty())
	{
		doc.setObject(obj);
		write(doc.toBinaryData());
	}
}

void RemoteControl::json_playlist(QJsonArray& arr) const
{
	QByteArray data;

	auto* plh = Playlist::Handler::instance();
	PlaylistConstPtr pl = plh->playlist(plh->active_index());
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
				QVariant::fromValue<Seconds>(Seconds(md.duration_ms / 1000)))
			);

			arr.append(obj);

			i++;
		}
	}
}

void RemoteControl::write_playlist()
{
	QJsonDocument doc;
	QJsonObject obj;

	auto* plh = Playlist::Handler::instance();
	MetaData md = PlayManager::instance()->current_track();
	PlaylistConstPtr pl = plh->playlist(plh->active_index());
	if(pl)
	{
		int cur_track_idx = pl->current_track_index();
		obj.insert("playlist-current-index", cur_track_idx);
	}

	QJsonArray arr;
	json_playlist(arr);
	if(arr.isEmpty()){
		return;
	}

	obj.insert("playlist", arr);
	doc.setObject(obj);

	write(doc.toBinaryData());
}


void RemoteControl::json_broadcast_info(QJsonObject& obj)
{
	obj.insert("broadcast-active", GetSetting(Set::Broadcast_Active));
	obj.insert("broadcast-port", GetSetting(Set::Broadcast_Port));
}

void RemoteControl::write_broadcast_info()
{
	QJsonDocument doc;
	QJsonObject obj;
	json_broadcast_info(obj);

	doc.setObject(obj);
	write(doc.toBinaryData());
}


void RemoteControl::write(const QByteArray& data)
{
	if(!m->socket){
		return;
	}

	m->socket->write(data + "ENDMESSAGE");
	m->socket->flush();
}


void RemoteControl::request_state()
{
	sp_log(Log::Debug, this) << "Current state requested";

	QJsonDocument doc;
	QJsonObject obj;

	json_volume(obj);
	json_current_position(obj);
	json_current_track(obj);
	json_playstate(obj);
	json_broadcast_info(obj);

	doc.setObject(obj);
	sp_log(Log::Info, this) << QString::fromLocal8Bit(doc.toJson());

	write(doc.toBinaryData());

	write_playlist();
	search_cover();
}


void RemoteControl::show_api()
{
	if(!m->socket || !m->socket->isOpen()){
		return;
	}

	m->socket->write("\n");

	for(auto it=m->fn_call_map.cbegin(); it!=m->fn_call_map.cend(); it++)
	{
		m->socket->write(it.key() + "\n");
	}

	m->socket->write("\n");

	for(auto it=m->fn_call_map.cbegin(); it!=m->fn_call_map.cend(); it++)
	{
		m->socket->write(it.key() + "( value )\n");
	}

	m->socket->write("\n");
}
