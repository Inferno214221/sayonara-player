/* PlayerPluginHandler.cpp */

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

#include "PlayerPluginBase.h"
#include "PlayerPluginHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QAction>

using PlayerPlugin::Handler;
using PlayerPlugin::Base;

namespace Algorithm=Util::Algorithm;

struct Handler::Private
{
	QList<Base*>	plugins;
	Base*			current_plugin=nullptr;

	bool			is_shutdown;

	Private() :
		is_shutdown(false)
	{}
};

Handler::Handler() :
	QObject()
{
	m = Pimpl::make<Private>();

	ListenSetting(Set::Player_Language, Handler::language_changed);
}

Handler::~Handler() = default;

void Handler::shutdown()
{
	m->is_shutdown = true;

	if(m->current_plugin)
	{
		SetSetting(Set::Player_ShownPlugin, m->current_plugin->get_name());
	}

	else {
		SetSetting(Set::Player_ShownPlugin, QString());
	}

	for(Base* plugin : m->plugins)
	{
		plugin->close();
		plugin->deleteLater();
	}

	m->plugins.clear();
}


Base* Handler::find_plugin(const QString& name)
{
	for(Base* p : Algorithm::AsConst(m->plugins))
	{
		if(p->get_name().compare(name) == 0)
		{
			return p;
		}
	}

	return nullptr;
}


void Handler::add_plugin(Base* plugin)
{
	if(!plugin){
		return;
	}

	m->plugins.push_back(plugin);

	connect(plugin, &Base::sig_action_triggered, this, &Handler::plugin_action_triggered);

	QString last_plugin = GetSetting(Set::Player_ShownPlugin);
	if(plugin->get_name() == last_plugin)
	{
		m->current_plugin = plugin;
		plugin->get_action()->setChecked(true);
	}

	emit sig_plugin_added(plugin);
}

void Handler::show_plugin(const QString& name)
{
	Base* plugin = find_plugin(name);
	if(!plugin)
	{
		return;
	}

	m->current_plugin = plugin;
	plugin->get_action()->setChecked(true);
}

void Handler::plugin_action_triggered(bool b)
{
	if(m->is_shutdown)
	{
		return;
	}

	Base* plugin = static_cast<Base*>(sender());
	if(b && plugin)
	{
		m->current_plugin = plugin;
	}

	else if(m->current_plugin == plugin)
	{
		m->current_plugin = nullptr;
	}

	emit sig_plugin_action_triggered(b);
}

void Handler::language_changed()
{
	for(Base* p : Algorithm::AsConst(m->plugins))
	{
		p->language_changed();
		p->get_action()->setText(p->get_display_name());
	}
}

QList<Base*> Handler::all_plugins() const
{
	return m->plugins;
}

Base* Handler::current_plugin() const
{
	return m->current_plugin;
}
