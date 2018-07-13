/* LibraryContextMenu.cpp */

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

#include "LibraryContextMenu.h"

#include "GUI/Utils/Icons.h"
#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/RatingLabel.h"
#include "GUI/Utils/PreferenceAction.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"

#include <QMap>

struct LibraryContextMenu::Private
{
	QMap<LibraryContextMenu::Entry, QAction*> entry_action_map;

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
	QAction*	clear_action=nullptr;
	QAction*	rating_action=nullptr;
	QAction*	cover_view_action=nullptr;
	QAction*	clear_selection_action=nullptr;
	QMenu*		rating_menu=nullptr;

	QAction*	preference_separator=nullptr;

	bool has_preference_actions;

	Private() :
		has_preference_actions(false)
	{}
};

LibraryContextMenu::LibraryContextMenu(QWidget* parent) :
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
	m->clear_action = new QAction(this);
	m->clear_selection_action = new QAction(this);
	m->cover_view_action = new QAction(this);
	m->cover_view_action->setCheckable(true);
	Set::listen<Set::Lib_ShowAlbumCovers>(this, &LibraryContextMenu::show_cover_view_changed);

	m->rating_menu = new QMenu(this);

	QList<QAction*> rating_actions;
	for(Rating i=0; i<=5; i++)
	{
		rating_actions << init_rating_action(i, m->rating_menu);
	}

	m->rating_menu->addActions(rating_actions);
	m->rating_action = this->addMenu(m->rating_menu);

	connect(m->info_action, &QAction::triggered, this, &LibraryContextMenu::sig_info_clicked);
	connect(m->lyrics_action, &QAction::triggered, this, &LibraryContextMenu::sig_lyrics_clicked);
	connect(m->edit_action, &QAction::triggered, this, &LibraryContextMenu::sig_edit_clicked);
	connect(m->remove_action, &QAction::triggered, this, &LibraryContextMenu::sig_remove_clicked);
	connect(m->delete_action, &QAction::triggered, this, &LibraryContextMenu::sig_delete_clicked);
	connect(m->play_action, &QAction::triggered, this, &LibraryContextMenu::sig_play_clicked);
	connect(m->play_new_tab_action, &QAction::triggered, this, &LibraryContextMenu::sig_play_new_tab_clicked);
	connect(m->play_next_action, &QAction::triggered, this, &LibraryContextMenu::sig_play_next_clicked);
	connect(m->append_action, &QAction::triggered, this, &LibraryContextMenu::sig_append_clicked);
	connect(m->refresh_action, &QAction::triggered, this, &LibraryContextMenu::sig_refresh_clicked);
	connect(m->clear_action, &QAction::triggered, this, &LibraryContextMenu::sig_clear_clicked);
	connect(m->clear_selection_action, &QAction::triggered, this, &LibraryContextMenu::sig_clear_selection_clicked);
	connect(m->cover_view_action, &QAction::triggered, this, &LibraryContextMenu::show_cover_triggered);

	QList<QAction*> actions;
	actions << m->play_action
			<< m->play_new_tab_action
			<< m->play_next_action
			<< m->append_action
			<< addSeparator()

			<< m->info_action
			<< m->lyrics_action
			<< m->edit_action
			<< m->rating_action
			<< addSeparator()

			<< m->refresh_action
			<< m->remove_action
			<< m->clear_action
			<< m->delete_action
			<< addSeparator()
			<< m->cover_view_action
			<< m->clear_selection_action
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
	m->entry_action_map[EntryClear] = m->clear_action;
	m->entry_action_map[EntryRating] = m->rating_action;
	m->entry_action_map[EntryClearSelection] = m->clear_selection_action;
	m->entry_action_map[EntryCoverView] = m->cover_view_action;

	for(QAction* action : ::Util::AsConst(actions))
	{
		action->setVisible(action->isSeparator());
	}


}

LibraryContextMenu::~LibraryContextMenu() {}

void LibraryContextMenu::language_changed()
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
	m->clear_action->setText(Lang::get(Lang::Clear));
	m->rating_action->setText(Lang::get(Lang::Rating));
	m->clear_selection_action->setText(tr("Clear selection"));
	m->cover_view_action->setText(tr("Cover view"));

	m->remove_action->setShortcut(QKeySequence(QKeySequence::Delete));
	m->play_action->setShortcut(QKeySequence(Qt::Key_Enter));
	m->play_new_tab_action->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Enter));
	m->play_next_action->setShortcut(QKeySequence(Qt::AltModifier + Qt::Key_Enter));
	m->append_action->setShortcut(QKeySequence(Qt::ShiftModifier + Qt::Key_Enter));
	m->info_action->setShortcut(QKeySequence(QKeySequence::WhatsThis));

/*	if(m->clear_action->isVisible()){
		m->clear_action->setShortcut(QKeySequence(Qt::Key_Backspace));
	}

	else if(m->clear_selection_action->isVisible()){
		m->clear_selection_action->setShortcut(QKeySequence(Qt::Key_Backspace));
	}
	*/
}


void LibraryContextMenu::skin_changed()
{
	using namespace Gui;
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
	m->clear_action->setIcon(Icons::icon(Icons::Clear));
	m->rating_action->setIcon(Icons::icon(Icons::Star));
	m->clear_selection_action->setIcon(Icons::icon(Icons::Clear));
}

LibraryContextMenu::Entries LibraryContextMenu::get_entries() const
{
	LibraryContextMenu::Entries entries = EntryNone;

	for(auto it=m->entry_action_map.cbegin(); it != m->entry_action_map.cend(); it++)
	{
		QAction* action = it.value();
		if(action->isVisible()){
			entries |= m->entry_action_map.key(action);
		}
	}

	return entries;
}


void LibraryContextMenu::show_actions(LibraryContextMenu::Entries entries)
{
	for(auto it=m->entry_action_map.cbegin(); it != m->entry_action_map.cend(); it++)
	{
		QAction* action = it.value();
		action->setVisible( entries & m->entry_action_map.key(action) );
	}
}

void LibraryContextMenu::show_action(LibraryContextMenu::Entry entry, bool visible)
{
	LibraryContextMenu::Entries entries = this->get_entries();
	if(visible){
		entries |= entry;
	}

	else{
		entries &= ~(entry);
	}

	show_actions(entries);
}

void LibraryContextMenu::show_all()
{
	const QList<QAction*> actions = this->actions();
	for(QAction* action : actions)
	{
		action->setVisible(true);
	}
}


QAction* LibraryContextMenu::init_rating_action(Rating rating, QObject* parent)
{
	QAction* action = new QAction(QString::number(rating), parent);
	action->setData(rating);
	action->setCheckable(true);

	connect(action, &QAction::triggered, this, [=]()
	{
		emit sig_rating_changed(rating);
	});

	return action;
}

void LibraryContextMenu::set_rating(Rating rating)
{
	QList<QAction*> actions = m->rating_menu->actions();
	for(QAction* action : actions){
		int data = action->data().toInt();
		action->setChecked(data == rating);
	}

	QString rating_text = Lang::get(Lang::Rating);
	if(rating > 0){
		m->rating_action->setText(rating_text + " (" + QString::number(rating) + ")");
	}

	else{
		m->rating_action->setText(rating_text);
	}
}

QAction* LibraryContextMenu::get_action(LibraryContextMenu::Entry entry) const
{
	return m->entry_action_map[entry];
}

QAction* LibraryContextMenu::add_preference_action(PreferenceAction* action)
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

QAction* LibraryContextMenu::before_preference_action() const
{
	return m->preference_separator;
}

void LibraryContextMenu::show_cover_view_changed()
{
	m->cover_view_action->setChecked(_settings->get<Set::Lib_ShowAlbumCovers>());
}

void LibraryContextMenu::show_cover_triggered(bool b)
{
	Q_UNUSED(b)
	bool show_covers = _settings->get<Set::Lib_ShowAlbumCovers>();
	_settings->set<Set::Lib_ShowAlbumCovers>(!show_covers);
}

