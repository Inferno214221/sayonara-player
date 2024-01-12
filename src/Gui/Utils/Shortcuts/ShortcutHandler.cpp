/* ShortcutHandler.cpp */

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

namespace Algorithm = Util::Algorithm;

struct ShortcutEntry
{
	ShortcutIdentifier identifier;
	QString databaseKey;
	QStringList defaultShortcuts;

	ShortcutEntry(ShortcutIdentifier identifier, const QString& databaseKey, const QString& defaultShortcut) :
		identifier(identifier),
		databaseKey(databaseKey),
		defaultShortcuts({defaultShortcut})
	{
		if(defaultShortcut.contains("enter", Qt::CaseInsensitive))
		{
			QString defaultShortcutCopy(defaultShortcut);
			defaultShortcutCopy.replace("enter", "Return", Qt::CaseInsensitive);
			defaultShortcuts << defaultShortcutCopy;
		}
	}
};

struct ShortcutHandler::Private
{
	Shortcut invalidShortcut;
	QList<ShortcutEntry> shortcutEntries;
	QList<Shortcut> shortcuts;

	Private()
	{
		invalidShortcut = Shortcut::getInvalid();

		shortcutEntries << ShortcutEntry(ShortcutIdentifier::PlayPause, "play_pause", QString("Space"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::Stop, "stop", QString("Ctrl+Space"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::Next, "next", QString("Ctrl+Right"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::Prev, "prev", QString("Ctrl+Left"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::VolDown, "vol_down", QString("Ctrl+-"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::VolUp, "vol_up", QString("Ctrl++"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::SeekFwd, "seek_fwd", QString("Alt+Right"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::SeekBwd, "seek_bwd", QString("Alt+Left"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::SeekFwdFast, "seek_fwd_fast", QString("Shift+Alt+Right"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::SeekBwdFast, "seek_bwd_fast", QString("Shift+Alt+Left"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::PlayNewTab, "play_new_tab", QString("Ctrl+Enter"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::PlayNext, "play_next", QString("Alt+Enter"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::Append, "append", QString("Shift+Enter"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::CoverView, "cover_view", QString("Ctrl+Shift+C"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::AlbumArtists, "album_artists", QString("Ctrl+Shift+A"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::ViewLibrary, "view_library", QString("Ctrl+L"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::AddTab, "add_tab", QString("Ctrl+T"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::CloseTab, "close_tab", QString("Ctrl+W"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::ClosePlugin, "close_plugin", QString("Ctrl+Esc"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::Minimize, "minimize", QString("Ctrl+M"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::Quit, "quit", QString("Ctrl+Q"));
		shortcutEntries << ShortcutEntry(ShortcutIdentifier::ReloadLibrary, "reload_library", QString("Ctrl+F5"));
		shortcutEntries
			<< ShortcutEntry(ShortcutIdentifier::TogglePlaylistLock, "lock_playlist", QString("Ctrl+Shift+L"));
	}
};

ShortcutHandler::ShortcutHandler() :
	QObject()
{
	m = Pimpl::make<Private>();

	DB::Shortcuts* db = DB::Connector::instance()->shortcutConnector();
	RawShortcutMap rsm = db->getAllShortcuts();

	const auto& entries = Algorithm::AsConst(m->shortcutEntries);

	for(const ShortcutEntry& se: entries)
	{
		const QStringList& shortcuts = rsm[se.databaseKey];
		if(shortcuts.isEmpty())
		{
			m->shortcuts << Shortcut(se.identifier, se.defaultShortcuts);
		}

		else
		{
			m->shortcuts << Shortcut(se.identifier, shortcuts);
		}
	}
}

ShortcutHandler::~ShortcutHandler() = default;

Shortcut ShortcutHandler::shortcut(ShortcutIdentifier identifier) const
{
	auto it = Util::Algorithm::find(m->shortcuts, [identifier](const Shortcut& s) {
		return (s.identifier() == identifier);
	});

	if(it != m->shortcuts.end())
	{
		return *it;
	}

	return m->invalidShortcut;
}

void ShortcutHandler::setShortcut(ShortcutIdentifier identifier, const QStringList& shortcuts)
{
	RawShortcutMap rsm;

	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier)
		{
			it->changeShortcut(shortcuts);
			emit sigShortcutChanged(identifier);
		}

		rsm[it->databaseKey()] = it->shortcuts();
	}

	DB::Shortcuts* db = DB::Connector::instance()->shortcutConnector();
	db->setShortcuts(this->databaseKey(identifier), shortcuts);
}

void ShortcutHandler::qtShortcutsAdded(ShortcutIdentifier identifier, const QList<QShortcut*>& qtShortcuts)
{
	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		if(it->identifier() == identifier)
		{
			it->setQtShortcuts(qtShortcuts);
		}
	}

	for(auto qit = qtShortcuts.begin(); qit != qtShortcuts.end(); qit++)
	{
		QShortcut* qsc = *qit;
		connect(qsc, &QObject::destroyed, this, &ShortcutHandler::qtShortcutDestroyed);
	}
}

void ShortcutHandler::qtShortcutDestroyed()
{
	auto* sc = static_cast<QShortcut*>(sender());

	for(auto it = m->shortcuts.begin(); it != m->shortcuts.end(); it++)
	{
		it->removeQtShortcut(sc);
	}
}

QList<ShortcutIdentifier> ShortcutHandler::allIdentifiers() const
{
	QList<ShortcutIdentifier> identifiers;

	for(const ShortcutEntry& sme: Algorithm::AsConst(m->shortcutEntries))
	{
		identifiers << sme.identifier;
	}

	return identifiers;
}

QString ShortcutHandler::databaseKey(ShortcutIdentifier identifier) const
{
	auto it = Algorithm::find(m->shortcutEntries, [identifier](const ShortcutEntry& entry) {
		return (entry.identifier == identifier);
	});

	if(it == m->shortcutEntries.end())
	{
		return QString();
	}

	return it->databaseKey;
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
			return Lang::get(Lang::LibraryView);
		case ShortcutIdentifier::TogglePlaylistLock:
			return tr("Toggle playlist lock");
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

