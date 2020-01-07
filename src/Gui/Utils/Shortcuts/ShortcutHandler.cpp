/* ShortcutHandler.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/RawShortcutMap.h"
#include "Utils/Logger/Logger.h"

#include <functional>
#include <QStringList>

namespace Algorithm=Util::Algorithm;

struct ShortcutEntry
{
	ShortcutIdentifier	identifier;
	QString				db_key;
	QString				default_shortcut;

	ShortcutEntry(ShortcutIdentifier identifier, const QString& db_key, const QString& default_shortcut) :
		identifier(identifier),
		db_key(db_key),
		default_shortcut(default_shortcut)
	{}
};

struct ShortcutHandler::Private
{
	Shortcut					invalid_shortcut;
	QList<ShortcutEntry>		shortcut_entries;
	QList<Shortcut>				shortcuts;

	Private()
	{
		invalid_shortcut = Shortcut::getInvalid();

		shortcut_entries << ShortcutEntry(ShortcutIdentifier::PlayPause, "play_pause", QString("Space"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::Stop, "stop", QString("Ctrl+Space"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::Next, "next", QString("Ctrl+Right"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::Prev, "prev", QString("Ctrl+Left"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::VolDown, "vol_down", QString("Ctrl+-"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::VolUp, "vol_up", QString("Ctrl++"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::SeekFwd, "seek_fwd", QString("Alt+Right"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::SeekBwd, "seek_bwd", QString("Alt+Left"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::SeekFwdFast, "seek_fwd_fast", QString("Shift+Alt+Right"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::SeekBwdFast, "seek_bwd_fast", QString("Shift+Alt+Left"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::PlayNewTab, "play_new_tab", QString("Ctrl+Enter"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::PlayNext, "play_next", QString("Alt+Enter"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::Append, "append", QString("Shift+Enter"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::CoverView, "cover_view", QString("Ctrl+Shift+C"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::AlbumArtists, "album_artists", QString("Ctrl+Shift+A"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::ViewLibrary, "view_library", QString("Ctrl+L"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::AddTab, "add_tab", QString("Ctrl+T"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::CloseTab, "close_tab", QString("Ctrl+W"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::ClosePlugin, "close_plugin", QString("Ctrl+Esc"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::Minimize, "minimize", QString("Ctrl+M"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::Quit, "quit", QString("Ctrl+Q"));
		shortcut_entries << ShortcutEntry(ShortcutIdentifier::ReloadLibrary, "reload_library", QString("Ctrl+F5"));
	}
};


ShortcutHandler::ShortcutHandler() :
	QObject()
{
	m = Pimpl::make<Private>();

	DB::Shortcuts* db = DB::Connector::instance()->shortcut_connector();
	RawShortcutMap rsm = db->getAllShortcuts();

	for(const ShortcutEntry& se : Algorithm::AsConst(m->shortcut_entries))
	{
		QStringList shortcuts = rsm[se.db_key];
		if(shortcuts.isEmpty())
		{
			 m->shortcuts << Shortcut(se.identifier, se.default_shortcut);
		}

		else {
			m->shortcuts << Shortcut(se.identifier, shortcuts);
		}
	}
}

ShortcutHandler::~ShortcutHandler() = default;

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

		rsm[it->db_key()] = it->shortcuts();
	}

	DB::Shortcuts* db = DB::Connector::instance()->shortcut_connector();
	db->setShortcuts(this->db_key(identifier), shortcuts);
}


void ShortcutHandler::qt_shortcuts_added(ShortcutIdentifier identifier, const QList<QShortcut*>& qt_shortcuts)
{
	for(auto it=m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier)
		{
			it->set_qt_shortcuts(qt_shortcuts);
		}
	}

	for(auto qit=qt_shortcuts.begin(); qit != qt_shortcuts.end(); qit++)
	{
		QShortcut* qsc = *qit;
		connect(qsc, &QObject::destroyed, this, &ShortcutHandler::qt_shortcut_destroyed);
	}
}


void ShortcutHandler::qt_shortcut_destroyed()
{
	auto* sc = static_cast<QShortcut*>(sender());

	for(auto it=m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		it->remove_qt_shortcut(sc);
	}
}


QList<ShortcutIdentifier> ShortcutHandler::all_identifiers() const
{
	QList<ShortcutIdentifier> identifiers;

	for(const ShortcutEntry& sme : Algorithm::AsConst(m->shortcut_entries))
	{
		identifiers << sme.identifier;
	}

	return identifiers;
}


QString ShortcutHandler::db_key(ShortcutIdentifier identifier) const
{
	auto it = Algorithm::find(m->shortcut_entries, [identifier](const ShortcutEntry& entry)
	{
		return (entry.identifier == identifier);
	});

	if(it == m->shortcut_entries.end()){
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

