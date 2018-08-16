/* ShortcutHandler.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "ShortcutHandler.h"
#include "Shortcut.h"

#include "Database/Connector.h"
#include "Database/Shortcuts.h"

#include "Utils/Utils.h"
#include "Utils/RawShortcutMap.h"

#include <functional>
#include <QStringList>


struct ShortcutHandler::Private
{
	QList<Shortcut> shortcuts;

	QMap<ShortcutHandler::Identifier, QString> map;

	Private()
	{
		map =
		{
			{ShortcutHandler::PlayPause, "play_pause"},
			{ShortcutHandler::Stop, "stop"},
			{ShortcutHandler::Next, "next"},
			{ShortcutHandler::Prev, "prev"},
			{ShortcutHandler::VolDown, "vol_down"},
			{ShortcutHandler::VolUp, "vol_up"},
			{ShortcutHandler::SeekFwd, "seek_fwd"},
			{ShortcutHandler::SeekBwd, "seek_bwd"},
			{ShortcutHandler::SeekFwdFast, "seek_fwd_fast"},
			{ShortcutHandler::SeekBwdFast, "seek_bwd_fast"},
			{ShortcutHandler::PlayNewTab, "play_new_tab"},
			{ShortcutHandler::PlayNext, "play_next"},
			{ShortcutHandler::Append, "append"},
			{ShortcutHandler::CoverView, "cover_view"},
			{ShortcutHandler::AlbumArtists, "album_artists"},
			{ShortcutHandler::Quit, "quit"},
			{ShortcutHandler::Minimize, "minimize"},
			{ShortcutHandler::ViewLibrary, "view_library"},
			{ShortcutHandler::AddTab, "add_tab"},
			{ShortcutHandler::CloseTab, "close_tab"},
			{ShortcutHandler::ClosePlugin, "close_plugin"}
		};
	}
};

ShortcutHandler::ShortcutHandler() :
	SayonaraClass()
{
	m = Pimpl::make<Private>();
}

ShortcutHandler::~ShortcutHandler() {}

Shortcut ShortcutHandler::get_shortcut(const QString& identifier) const
{
	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier){
			return *it;
		}
	}

	return Shortcut::getInvalid();
}

Shortcut ShortcutHandler::get_shortcut(const ShortcutHandler::Identifier& identifier) const
{
	QString id = this->identifier(identifier);
	return get_shortcut(id);
}

void ShortcutHandler::set_shortcut(const QString& identifier, const QStringList& shortcuts)
{
	RawShortcutMap rsm;
	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier)
		{
			it->change_shortcut(shortcuts);
			emit sig_shortcut_changed(identifier);
		}

		rsm[it->identifier()] = it->shortcuts();
	}

	DB::Shortcuts* db = DB::Connector::instance()->shortcut_connector();
	db->setShortcuts(identifier, shortcuts);
}


Shortcut ShortcutHandler::add(ShortcutWidget* parent, ShortcutHandler::Identifier id, const QString& name, const QString& default_shortcut)
{
	QString identifier = this->identifier(id);
	for(const Shortcut& sc : m->shortcuts)
	{
		if(identifier == sc.identifier()){
			return sc;
		}
	}

	Shortcut shortcut(parent, identifier, name, default_shortcut);

	if(!shortcut.is_valid()){
		Shortcut::getInvalid();
	}

	m->shortcuts << shortcut;
	return shortcut;
}

QStringList ShortcutHandler::get_shortcuts() const
{
	QStringList ret;

	for(auto sc : Util::AsConst(m->shortcuts)){
		ret << sc.identifier();
	}

	return ret;
}

void ShortcutHandler::set_parent_deleted(ShortcutWidget* parent)
{
	QMutableListIterator<Shortcut> it(m->shortcuts);
	while(it.hasNext()){
		Shortcut sc = it.next();
		if(sc.parent() == parent){
			it.remove();
		}
	}
}

QString ShortcutHandler::identifier(ShortcutHandler::Identifier id) const
{
	return m->map.value(id);
}
