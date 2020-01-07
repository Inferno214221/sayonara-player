/* LibraryContextMenu.cpp */

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

#include "LibraryContextMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Message/Message.h"

#include <QMap>
#include <QTimer>

using Library::ContextMenu;
namespace Algorithm=Util::Algorithm;

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, QAction*> entry_action_map;

	QMenu*		filetype_menu=nullptr;

	QAction*	info_action=nullptr;
	QAction*	lyrics_action=nullptr;
	QAction*	edit_action=nullptr;
	QAction*	remove_action=nullptr;
	QAction*	delete_action=nullptr;
	QAction*	play_action=nullptr;
	QAction*	play_new_tab_action=nullptr;
	QAction*	play_next_action=nullptr;
	QAction*	append_action=nullptr;
	QAction*	refresh_action=nullptr;
	QAction*	reload_library_action=nullptr;
	QAction*	clear_action=nullptr;
	QAction*	cover_view_action=nullptr;
	QAction*	filetype_action=nullptr;
	QAction*	show_filetype_bar_action=nullptr;
	QAction*	preference_separator=nullptr;

	bool has_preference_actions;

	Private() :
		has_preference_actions(false)
	{}
};

ContextMenu::ContextMenu(QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>();

	m->info_action = new QAction(this);
	m->lyrics_action  = new QAction(this);
	m->edit_action = new QAction(this);
	m->remove_action = new QAction(this);
	m->delete_action = new QAction(this);
	m->play_action = new QAction(this);
	m->play_new_tab_action = new QAction(this);
	m->play_next_action = new QAction(this);
	m->append_action = new QAction(this);
	m->refresh_action = new QAction(this);
	m->reload_library_action = new QAction(this);
	m->clear_action = new QAction(this);
	m->cover_view_action = new QAction(this);
	m->cover_view_action->setCheckable(true);
	m->filetype_menu = new QMenu(this);
	m->filetype_action = this->addMenu(m->filetype_menu);
	m->show_filetype_bar_action = new QAction(this);
	m->show_filetype_bar_action->setCheckable(true);

	ListenSetting(Set::Lib_ShowAlbumCovers, ContextMenu::show_cover_view_changed);
	ListenSetting(Set::Lib_ShowFilterExtBar, ContextMenu::show_filter_ext_bar_changed);

	ShortcutHandler* sch = ShortcutHandler::instance();
	connect(sch, &ShortcutHandler::sig_shortcut_changed, this, &ContextMenu::shortcut_changed);

	QList<QAction*> actions;
	actions << m->play_action
			<< m->play_new_tab_action
			<< m->play_next_action
			<< m->append_action
			<< addSeparator()

			<< m->info_action
			<< m->lyrics_action
			<< m->edit_action
			<< m->filetype_action
			<< addSeparator()

			<< m->reload_library_action
			<< m->refresh_action
			<< m->remove_action
			<< m->clear_action
			<< m->delete_action
			<< addSeparator()
			<< m->cover_view_action
	;

	this->addActions(actions);

	m->entry_action_map[EntryInfo] = m->info_action;
	m->entry_action_map[EntryEdit] = m->edit_action;
	m->entry_action_map[EntryLyrics] = m->lyrics_action;
	m->entry_action_map[EntryRemove] = m->remove_action;
	m->entry_action_map[EntryDelete] = m->delete_action;
	m->entry_action_map[EntryPlay] = m->play_action;
	m->entry_action_map[EntryPlayNewTab] = m->play_new_tab_action;
	m->entry_action_map[EntryPlayNext] = m->play_next_action;
	m->entry_action_map[EntryAppend] = m->append_action;
	m->entry_action_map[EntryRefresh] = m->refresh_action;
	m->entry_action_map[EntryReload] = m->reload_library_action;
	m->entry_action_map[EntryClear] = m->clear_action;
	m->entry_action_map[EntryCoverView] = m->cover_view_action;
	m->entry_action_map[EntryFilterExtension] = m->filetype_action;

	for(QAction* action : Algorithm::AsConst(actions))
	{
		action->setVisible(action->isSeparator());
	}

	connect(m->info_action, &QAction::triggered, this, &ContextMenu::sig_info_clicked);
	connect(m->lyrics_action, &QAction::triggered, this, &ContextMenu::sig_lyrics_clicked);
	connect(m->edit_action, &QAction::triggered, this, &ContextMenu::sig_edit_clicked);
	connect(m->remove_action, &QAction::triggered, this, &ContextMenu::sig_remove_clicked);
	connect(m->delete_action, &QAction::triggered, this, &ContextMenu::sig_delete_clicked);
	connect(m->play_action, &QAction::triggered, this, &ContextMenu::sig_play_clicked);
	connect(m->play_new_tab_action, &QAction::triggered, this, &ContextMenu::sig_play_new_tab_clicked);
	connect(m->play_next_action, &QAction::triggered, this, &ContextMenu::sig_play_next_clicked);
	connect(m->append_action, &QAction::triggered, this, &ContextMenu::sig_append_clicked);
	connect(m->refresh_action, &QAction::triggered, this, &ContextMenu::sig_refresh_clicked);
	connect(m->reload_library_action, &QAction::triggered, this, &ContextMenu::sig_reload_clicked);
	connect(m->clear_action, &QAction::triggered, this, &ContextMenu::sig_clear_clicked);
	connect(m->cover_view_action, &QAction::triggered, this, &ContextMenu::show_cover_triggered);
	connect(m->show_filetype_bar_action, &QAction::triggered, this, &ContextMenu::show_filter_extension_bar_triggered);
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::language_changed()
{
	m->info_action->setText(Lang::get(Lang::Info));
	m->lyrics_action->setText(Lang::get(Lang::Lyrics));
	m->edit_action->setText(Lang::get(Lang::Edit));
	m->remove_action->setText(Lang::get(Lang::Remove));
	m->delete_action->setText(Lang::get(Lang::Delete));
	m->play_action->setText(Lang::get(Lang::Play));
	m->play_new_tab_action->setText(tr("Play in new tab"));
	m->play_next_action->setText(Lang::get(Lang::PlayNext));
	m->append_action->setText(Lang::get(Lang::Append));
	m->refresh_action->setText(Lang::get(Lang::Refresh));
	m->reload_library_action->setText(Lang::get(Lang::ReloadLibrary));
	m->clear_action->setText(Lang::get(Lang::Clear));
	m->cover_view_action->setText(tr("Cover view"));
	m->filetype_action->setText(Lang::get(Lang::Filetype));
	m->show_filetype_bar_action->setText(Lang::get(Lang::Show) + ": " + tr("Toolbar"));

	m->play_action->setShortcut(QKeySequence(Qt::Key_Enter));
	m->delete_action->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Delete));
	m->remove_action->setShortcut(QKeySequence(QKeySequence::Delete));
	m->clear_action->setShortcut(QKeySequence(Qt::Key_Backspace));

	shortcut_changed(ShortcutIdentifier::Invalid);
}


void ContextMenu::shortcut_changed(ShortcutIdentifier identifier)
{
	Q_UNUSED(identifier)
	ShortcutHandler* sch = ShortcutHandler::instance();

	m->play_new_tab_action->setShortcut(sch->shortcut(ShortcutIdentifier::PlayNewTab).sequence());
	m->play_next_action->setShortcut(sch->shortcut(ShortcutIdentifier::PlayNext).sequence());
	m->append_action->setShortcut(sch->shortcut(ShortcutIdentifier::Append).sequence());
	m->cover_view_action->setShortcut(sch->shortcut(ShortcutIdentifier::CoverView).sequence());
	m->reload_library_action->setShortcut(sch->shortcut(ShortcutIdentifier::ReloadLibrary).sequence());
}

void ContextMenu::skin_changed()
{
	using namespace Gui;

	QTimer::singleShot(100, this, &ContextMenu::skin_timer_timeout);
}

void ContextMenu::skin_timer_timeout()
{
	namespace Icons=Gui::Icons;

	m->info_action->setIcon(Icons::icon(Icons::Info));
	m->lyrics_action->setIcon(Icons::icon(Icons::Lyrics));
	m->edit_action->setIcon(Icons::icon(Icons::Edit));
	m->remove_action->setIcon(Icons::icon(Icons::Remove));
	m->delete_action->setIcon(Icons::icon(Icons::Delete));
	m->play_action->setIcon(Icons::icon(Icons::PlaySmall));
	m->play_new_tab_action->setIcon(Icons::icon(Icons::PlaySmall));
	m->play_next_action->setIcon(Icons::icon(Icons::PlaySmall));
	m->append_action->setIcon(Icons::icon(Icons::Append));
	m->refresh_action->setIcon(Icons::icon(Icons::Undo));
	m->reload_library_action->setIcon(Icons::icon(Icons::Refresh));
	m->clear_action->setIcon(Icons::icon(Icons::Clear));
}

ContextMenu::Entries ContextMenu::get_entries() const
{
	ContextMenu::Entries entries = EntryNone;

	for(auto it=m->entry_action_map.cbegin(); it != m->entry_action_map.cend(); it++)
	{
		QAction* action = it.value();
		if(action->isVisible()){
			entries |= m->entry_action_map.key(action);
		}
	}

	return entries;
}


void ContextMenu::show_actions(ContextMenu::Entries entries)
{
	for(auto it=m->entry_action_map.cbegin(); it != m->entry_action_map.cend(); it++)
	{
		QAction* action = it.value();
		action->setVisible( entries & m->entry_action_map.key(action) );
	}
}

void ContextMenu::show_action(ContextMenu::Entry entry, bool visible)
{
	ContextMenu::Entries entries = this->get_entries();
	if(visible){
		entries |= entry;
	}

	else{
		entries &= ~(entry);
	}

	show_actions(entries);
}

void ContextMenu::show_all()
{
	const QList<QAction*> actions = this->actions();
	for(QAction* action : actions)
	{
		action->setVisible(true);
	}
}

QAction* ContextMenu::get_action(ContextMenu::Entry entry) const
{
	return m->entry_action_map[entry];
}

QAction* ContextMenu::get_action_after(ContextMenu::Entry entry) const
{
	QAction* a = get_action(entry);
	if(!a){
		return nullptr;
	}

	QList<QAction*> actions = this->actions();
	auto it = std::find(actions.begin(), actions.end(), a);

	if(it == actions.end()){
		return nullptr;
	}

	it++;
	if(it == actions.end()) {
		return nullptr;
	}

	return *it;
}

QAction* ContextMenu::add_preference_action(Gui::PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->has_preference_actions){
		m->preference_separator = this->addSeparator();
		actions << m->preference_separator;
	}

	actions << action;

	this->addActions(actions);
	m->has_preference_actions = true;

	return action;
}

QAction* ContextMenu::before_preference_action() const
{
	return m->preference_separator;
}

void ContextMenu::set_action_shortcut(ContextMenu::Entry entry, const QString& shortcut)
{
	QAction* action = get_action(entry);
	if(action)
	{
		action->setShortcut(QKeySequence(shortcut));
	}
}

void ContextMenu::set_extensions(const Gui::ExtensionSet& extensions)
{
	QMenu* fem = m->filetype_menu;
	if(fem->isEmpty())
	{
		fem->addActions({fem->addSeparator(), m->show_filetype_bar_action});
	}

	while(fem->actions().count() > 2)
	{
		fem->removeAction(fem->actions().at(0));
	}

	QAction* sep = fem->actions().at(fem->actions().count() - 2);

	const QStringList ext_list = extensions.extensions();

	for(const QString& ext : ext_list)
	{
		QAction* a = new QAction(ext, fem);
		a->setCheckable(true);
		a->setChecked(extensions.is_enabled(ext));
		a->setEnabled(ext_list.count() > 1);

		connect(a, &QAction::triggered, this, [=](bool b){
			emit sig_filter_triggered(a->text(), b);
		});

		fem->insertAction(sep, a);
	}
}

void ContextMenu::set_selection_count(int num_selections)
{
	bool has_selections = (num_selections > 0);
	for(auto it : m->entry_action_map)
	{
		it->setEnabled(has_selections);
	}

	m->entry_action_map[EntryCoverView]->setEnabled(true);
	m->entry_action_map[EntryReload]->setEnabled(true);
}

QKeySequence ContextMenu::shortcut(ContextMenu::Entry entry) const
{
	QAction* a = get_action(entry);
	if(!a){
		return QKeySequence();
	}

	return a->shortcut();
}

void ContextMenu::show_cover_view_changed()
{
	m->cover_view_action->setChecked(GetSetting(Set::Lib_ShowAlbumCovers));
}

void ContextMenu::show_cover_triggered(bool b)
{
	Q_UNUSED(b)
	bool show_covers = GetSetting(Set::Lib_ShowAlbumCovers);
	SetSetting(Set::Lib_ShowAlbumCovers, !show_covers);
}

void ContextMenu::show_filter_ext_bar_changed()
{
	m->show_filetype_bar_action->setChecked(GetSetting(Set::Lib_ShowFilterExtBar));
}

void ContextMenu::show_filter_extension_bar_triggered(bool b)
{
	SetSetting(Set::Lib_ShowFilterExtBar, b);

	if(b)
	{
		Message::info
		(
			tr("The toolbar is visible when there are tracks with differing file types listed in the track view"),
			Lang::get(Lang::Filetype)
		);
	}
}

