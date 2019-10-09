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
	plugin_closed();

	for(Base* plugin : m->plugins)
	{
		plugin->close();
		plugin->deleteLater();
	}

	m->plugins.clear();
}


Base* Handler::find_plugin(const QString& name)
{
	sp_log(Log::Debug, this) << "Search for plugin " << name;

	for(Base* p : Algorithm::AsConst(m->plugins))
	{
		if(p->get_name().compare(name) == 0)
		{
			return p;
		}
	}

	return nullptr;
}


void Handler::add_plugin(Base* p)
{
	if(!p){
		return;
	}

	m->plugins.push_back(p);

	connect(p, SIGNAL(sig_closed()), this, SLOT(plugin_closed()));
	connect(p, SIGNAL(sig_opened()), this, SLOT(plugin_opened()));
	connect(p, SIGNAL(sig_action_triggered(bool)), this, SLOT(plugin_action_triggered(bool)));

	QString last_plugin = GetSetting(Set::Player_ShownPlugin);
	if(p->get_name() == last_plugin)
	{
		m->current_plugin = p;
		p->get_action()->setChecked(true);
	}

	emit sig_plugin_added(p);
}

void Handler::show_plugin(const QString& name)
{
	Base* plugin = find_plugin(name);
	if(!plugin){
		return;
	}

	sig_plugin_action_triggered(false);
	m->current_plugin = plugin;
	sig_plugin_action_triggered(true);
}


void Handler::plugin_action_triggered(bool b)
{
	auto* plugin = static_cast<Base*>(sender());

	if(b){
		m->current_plugin = plugin;
	}

	else {
		m->current_plugin = nullptr;
	}

	emit sig_plugin_action_triggered(b);
}

void Handler::plugin_opened(Base* p)
{
	if(p){
		SetSetting(Set::Player_ShownPlugin, p->get_name());
	}
}

void Handler::plugin_opened()
{
	auto* p = static_cast<Base*>(sender());
	plugin_opened(p);
}

void Handler::plugin_closed()
{
	m->current_plugin = nullptr;
	SetSetting(Set::Player_ShownPlugin, QString());

	emit sig_plugin_closed();
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
