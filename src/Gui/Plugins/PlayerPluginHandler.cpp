/* PlayerPluginHandler.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "PlayerPluginBase.h"
#include "PlayerPluginHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QAction>

using PlayerPlugin::Handler;
using PlayerPlugin::Base;

namespace Algorithm = Util::Algorithm;

struct Handler::Private
{
	QList<Base*> plugins;
	Base* currentPlugin = nullptr;
	bool isShutdown {false};
};

Handler::Handler() :
	QObject()
{
	m = Pimpl::make<Private>();

	ListenSetting(Set::Player_Language, Handler::languageChanged);
}

Handler::~Handler() = default;

void Handler::shutdown()
{
	m->isShutdown = true;

	const auto currentPluginName = m->currentPlugin
	                               ? m->currentPlugin->name()
	                               : QString();

	SetSetting(Set::Player_ShownPlugin, currentPluginName);

	for(auto* plugin: m->plugins)
	{
		plugin->close();
		plugin->deleteLater();
	}

	m->plugins.clear();
}

Base* Handler::findPlugin(const QString& name)
{
	const auto it = Util::Algorithm::find(m->plugins, [&](const auto* plugin) {
		return (plugin->name() == name);
	});

	return (it == m->plugins.end())
	       ? nullptr
	       : *it;
}

void Handler::addPlugin(Base* plugin)
{
	if(!plugin)
	{
		return;
	}

	m->plugins.push_back(plugin);

	connect(plugin, &Base::sigActionTriggered, this, &Handler::pluginActionTriggered);

	const auto lastPlugin = GetSetting(Set::Player_ShownPlugin);
	if(plugin->name() == lastPlugin)
	{
		m->currentPlugin = plugin;
		plugin->pluginAction()->setChecked(true);
	}

	emit sigPluginAdded(plugin);
}

void Handler::showPlugin(const QString& name)
{
	if(auto* plugin = findPlugin(name); plugin)
	{
		m->currentPlugin = plugin;
		m->currentPlugin->pluginAction()->trigger();
	}
}

void Handler::pluginActionTriggered(const bool b)
{
	if(m->isShutdown)
	{
		return;
	}

	auto* plugin = dynamic_cast<Base*>(sender());

	if(b)
	{
		if(plugin)
		{
			m->currentPlugin = plugin;
		}
	}

	else
	{
		if(m->currentPlugin == plugin)
		{
			m->currentPlugin = nullptr;
		}
	}

	emit sigPluginActionTriggered(b);
}

void Handler::languageChanged()
{
	for(auto* plugins: m->plugins)
	{
		plugins->languageChanged();
		plugins->pluginAction()->setText(plugins->displayName());
	}
}

QList<Base*> Handler::allPlugins() const { return m->plugins; }

Base* Handler::currentPlugin() const { return m->currentPlugin; }
