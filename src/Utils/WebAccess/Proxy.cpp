/* Proxy.cpp */

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

#include "Proxy.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Utils.h"
#include "Utils/Crypt.h"
#include "Utils/Logger/Logger.h"

#include <QNetworkProxy>

void Proxy::init()
{
	Proxy::set_proxy();
}

void Proxy::set_proxy()
{
	QNetworkProxy proxy;

	if(active())
	{
		proxy.setType(QNetworkProxy::HttpProxy);
		proxy.setHostName(Proxy::hostname());
		proxy.setPort(Proxy::port());

		if(has_username()){
			proxy.setUser(username());
			proxy.setPassword(password());
		}

		QString url = full_url();

		Util::set_environment("http_proxy", url.toLocal8Bit().data());
		Util::set_environment("https_proxy", url.toLocal8Bit().data());
		Util::set_environment("HTTP_PROXY", url.toLocal8Bit().data());
		Util::set_environment("HTTPS_PROXY", url.toLocal8Bit().data());
	}

	QNetworkProxy::setApplicationProxy(proxy);
}

void Proxy::unset_proxy()
{
	QNetworkProxy proxy;
	proxy.setType(QNetworkProxy::NoProxy);
	QNetworkProxy::setApplicationProxy(proxy);
}


QString Proxy::hostname()
{
	return GetSetting(Set::Proxy_Hostname);
}

uint16_t Proxy::port()
{
	return static_cast<uint16_t>(GetSetting(Set::Proxy_Port));
}

QString Proxy::username()
{
	return GetSetting(Set::Proxy_Username);
}

QString Proxy::password()
{
	return Util::Crypt::decrypt(GetSetting(Set::Proxy_Password));
}

bool Proxy::active()
{
	return GetSetting(Set::Proxy_Active);
}

bool Proxy::has_username()
{
	return ((username() + password()).size() > 0);
}

QString Proxy::full_url()
{
	if(!active()){
		return QString();
	}

	QString host_name = hostname();
	if(!host_name.startsWith("http")){
		host_name.prepend("http://");
	}

	return host_name + ":" + QString::number(port());
}

QString Proxy::env_hostname()
{
	QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
	for(const QNetworkProxy& proxy : proxies)
	{
		QString hostname = proxy.hostName();
		if(!hostname.isEmpty()){
			return hostname;
		}
	}

	return QString();
}


int Proxy::env_port()
{
	QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
	for(const QNetworkProxy& proxy : proxies)
	{
		int port = proxy.port();
		if(port > 0){
			return port;
		}
	}

	return 3128;
}
