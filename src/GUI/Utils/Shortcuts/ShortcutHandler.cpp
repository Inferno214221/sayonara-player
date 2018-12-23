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
#include "Utils/Language.h"
#include "Utils/RawShortcutMap.h"
#include "Utils/Logger/Logger.h"

#include <functional>
#include <QStringList>

struct ShortcutMapEntry
{
	ShortcutIdentifier identifier;
	QString db_key;
	QString default_shortcut;


	ShortcutMapEntry(ShortcutIdentifier identifier, const QString& db_key, const QString& default_shortcut) :
		identifier(identifier),
		db_key(db_key),
		default_shortcut(default_shortcut)
	{}
};

struct ShortcutHandler::Private
{
	Shortcut invalid_shortcut;
	QList<ShortcutMapEntry> shortcut_map;
	QList<Shortcut> shortcuts;

	Private()
	{
		invalid_shortcut = Shortcut::getInvalid();

		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::PlayPause, "play_pause", QString("Space"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::Stop, "stop", QString("Ctrl+Space"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::Next, "next", QString("Ctrl+Right"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::Prev, "prev", QString("Ctrl+Left"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::VolDown, "vol_down", QString("Ctrl+-"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::VolUp, "vol_up", QString("Ctrl++"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::SeekFwd, "seek_fwd", QString("Alt+Right"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::SeekBwd, "seek_bwd", QString("Alt+Left"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::SeekFwdFast, "seek_fwd_fast", QString("Shift+Alt+Right"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::SeekBwdFast, "seek_bwd_fast", QString("Shift+Alt+Left"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::PlayNewTab, "play_new_tab", QString("Ctrl+Enter"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::PlayNext, "play_next", QString("Alt+Enter"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::Append, "append", QString("Shift+Enter"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::CoverView, "cover_view", QString("Ctrl+Shift+C"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::AlbumArtists, "album_artists", QString("Ctrl+Shift+A"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::ViewLibrary, "view_library", QString("Ctrl+L"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::AddTab, "add_tab", QString("Ctrl+T"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::CloseTab, "close_tab", QString("Ctrl+W"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::ClosePlugin, "close_plugin", QString("Ctrl+Esc"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::Minimize, "minimize", QString("Ctrl+M"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::Quit, "quit", QString("Ctrl+Q"));
		shortcut_map << ShortcutMapEntry(ShortcutIdentifier::ReloadLibrary, "reload_library", QString("Ctrl+F5"));
	}
};

ShortcutHandler::ShortcutHandler() :
	SayonaraClass()
{
	m = Pimpl::make<Private>();

	DB::Shortcuts* db = DB::Connector::instance()->shortcut_connector();
	RawShortcutMap rsm = db->getAllShortcuts();

	for(const ShortcutMapEntry& sme : Util::AsConst(m->shortcut_map))
	{
		QStringList shortcuts = rsm[sme.db_key];
		if(shortcuts.isEmpty())
		{
			 m->shortcuts << Shortcut(sme.identifier, sme.default_shortcut);
		}

		else {
			m->shortcuts << Shortcut(sme.identifier, shortcuts);
		}
	}
}

ShortcutHandler::~ShortcutHandler() {}

Shortcut ShortcutHandler::shortcut(ShortcutIdentifier identifier) const
{
	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier){
			return *it;
		}
	}

	return m->invalid_shortcut;
}

void ShortcutHandler::set_shortcut(ShortcutIdentifier identifier, const QStringList& shortcuts)
{
	RawShortcutMap rsm;
	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier)
		{
			it->change_shortcut(shortcuts);
			emit sig_shortcut_changed(identifier);
		}

		rsm[it->identifier_string()] = it->shortcuts();
	}

	DB::Shortcuts* db = DB::Connector::instance()->shortcut_connector();
	db->setShortcuts(this->identifier(identifier), shortcuts);
}


void ShortcutHandler::qt_shortcuts_added(ShortcutIdentifier identifier, const QList<QShortcut*>& shortcuts)
{
	for(auto it=m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier)
		{
			it->add_qt_shortcuts(shortcuts);
		}
	}
}


QList<ShortcutIdentifier> ShortcutHandler::shortcuts_ids() const
{
	QList<ShortcutIdentifier> ids;

	for(const ShortcutMapEntry& sme : ::Util::AsConst(m->shortcut_map))
	{
		ids << sme.identifier;
	}

	return ids;
}

QString ShortcutHandler::identifier(ShortcutIdentifier id) const
{
	auto it = ::Util::find(m->shortcut_map, [id](const ShortcutMapEntry& entry){
		return (entry.identifier == id);
	});

	if(it == m->shortcut_map.end()){
		return QString();
	}

	return it->db_key;
}

QString ShortcutHandler::shortcut_text(ShortcutIdentifier identifier) const
{
	switch(identifier)
	{
		case ShortcutIdentifier::AddTab:
			return Lang::get(Lang::AddTab);
		case ShortcutIdentifier::AlbumArtists:
			return Lang::get(Lang::ShowAlbumArtists);
		case ShortcutIdentifier::Append:
			return Lang::get(Lang::Tracks) + ": " + Lang::get(Lang::Append);
		case ShortcutIdentifier::ClosePlugin:
			return Lang::get(Lang::Plugin) + ": " + Lang::get(Lang::Close);
		case ShortcutIdentifier::CloseTab:
			return Lang::get(Lang::CloseTab);
		case ShortcutIdentifier::CoverView:
			return Lang::get(Lang::ShowCovers);
		case ShortcutIdentifier::Minimize:
			return Lang::get(Lang::Application) + ": " + Lang::get(Lang::Minimize);
		case ShortcutIdentifier::Next:
			return Lang::get(Lang::NextTrack);
		case ShortcutIdentifier::PlayNewTab:
			return Lang::get(Lang::Tracks) + ": " + Lang::get(Lang::PlayInNewTab);
		case ShortcutIdentifier::PlayNext:
			return Lang::get(Lang::Tracks) + ": " + Lang::get(Lang::PlayNext);
		case ShortcutIdentifier::PlayPause:
			return Lang::get(Lang::PlayPause);
		case ShortcutIdentifier::Prev:
			return Lang::get(Lang::PreviousTrack);
		case ShortcutIdentifier::Quit:
			return Lang::get(Lang::Application) + ": " + Lang::get(Lang::Quit);
		case ShortcutIdentifier::SeekBwd:
			return Lang::get(Lang::SeekBackward);
		case ShortcutIdentifier::SeekBwdFast:
			return Lang::get(Lang::SeekBackward).space() + "(" + Lang::get(Lang::Fast) + ")";
		case ShortcutIdentifier::SeekFwd:
			return Lang::get(Lang::SeekForward);
		case ShortcutIdentifier::SeekFwdFast:
			return Lang::get(Lang::SeekForward).space() + "(" + Lang::get(Lang::Fast) + ")";
		case ShortcutIdentifier::Stop:
			return Lang::get(Lang::Stop);
		case ShortcutIdentifier::ViewLibrary:
			return Lang::get(Lang::ShowLibrary);
		case ShortcutIdentifier::VolDown:
			return Lang::get(Lang::VolumeDown);
		case ShortcutIdentifier::VolUp:
			return Lang::get(Lang::VolumeUp);
		case ShortcutIdentifier::ReloadLibrary:
			return Lang::get(Lang::ReloadLibrary);
		default:
			return QString();
	}
}

