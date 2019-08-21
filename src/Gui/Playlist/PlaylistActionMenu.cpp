/* PlaylistActionMenu.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include "PlaylistActionMenu.h"
#include "Components/Library/LibraryManager.h"

#include "Gui/Plugins/PlayerPluginHandler.h"

#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"

#include <QList>
#include <QAction>

using Playlist::ActionMenu;

struct ActionMenu::Private
{
	Playlist::Mode		plm;

	QAction*		action_rep1=nullptr;
	QAction*		action_append=nullptr;
	QAction*		action_repAll=nullptr;
	QAction*		action_dynamic=nullptr;
	QAction*		action_shuffle=nullptr;
	QAction*		action_gapless=nullptr;

	QList<QAction*> actions()
	{
		return {
			action_rep1,
			action_append,
			action_repAll,
			action_dynamic,
			action_shuffle,
			action_gapless
		};
	}
};


ActionMenu::ActionMenu(QWidget* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>();

	using namespace Gui;
	m->action_rep1 = new QAction(this);
	m->action_repAll = new QAction(this);
	m->action_append = new QAction(this);
	m->action_dynamic = new QAction(this);
	m->action_shuffle = new QAction(this);
	m->action_gapless = new QAction(this);

	const auto actions = m->actions();
	for(auto action : actions)
	{
		action->setCheckable(true);
	}

	m->action_gapless->setCheckable(false);

	this->addActions(m->actions());


	m->plm = GetSetting(Set::PL_Mode);

	m->action_rep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->action_repAll->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->action_append->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->action_dynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));
	m->action_shuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));

	connect(m->action_rep1, &QAction::toggled, this, &ActionMenu::rep1_checked);
	connect(m->action_repAll, &QAction::toggled, this, &ActionMenu::rep_all_checked);
	connect(m->action_append, &QAction::toggled, this, &ActionMenu::playlist_mode_changed);
	connect(m->action_shuffle, &QAction::toggled, this, &ActionMenu::shuffle_checked);
	connect(m->action_dynamic, &QAction::toggled, this, &ActionMenu::playlist_mode_changed);
	connect(m->action_gapless, &QAction::triggered, this, &ActionMenu::gapless_clicked);

	ListenSetting(Set::PL_Mode, ActionMenu::s_playlist_mode_changed);

	language_changed();
}

ActionMenu::~ActionMenu() = default;

void ActionMenu::rep1_checked(bool checked)
{
	if(checked){
		m->action_repAll->setChecked(false);
		m->action_shuffle->setChecked(false);
	}

	playlist_mode_changed();
}

void ActionMenu::rep_all_checked(bool checked)
{
	if(checked){
		m->action_rep1->setChecked(false);
	}

	playlist_mode_changed();
}

void ActionMenu::shuffle_checked(bool checked)
{
	if(checked){
		m->action_rep1->setChecked(false);
	}

	playlist_mode_changed();
}


// internal gui slot
void ActionMenu::playlist_mode_changed()
{
	Playlist::Mode plm;

	plm.setAppend(m->action_append->isChecked(), m->action_append->isEnabled());
	plm.setRep1(m->action_rep1->isChecked(), m->action_rep1->isEnabled());
	plm.setRepAll(m->action_repAll->isChecked(), m->action_repAll->isEnabled());
	plm.setShuffle(m->action_shuffle->isChecked(), m->action_shuffle->isEnabled());
	plm.setDynamic(m->action_dynamic->isChecked(), m->action_dynamic->isEnabled());

	if(plm == m->plm){
		return;
	}

	m->plm = plm;

	SetSetting(Set::PL_Mode, m->plm);
}


void ActionMenu::gapless_clicked()
{
	PlayerPlugin::Handler::instance()->show_plugin("Crossfader");
}


void ActionMenu::language_changed()
{
	m->action_append->setText(Lang::get(Lang::Append));
	m->action_dynamic->setText(Lang::get(Lang::DynamicPlayback));
	m->action_gapless->setText(Lang::get(Lang::GaplessPlayback));
	m->action_rep1->setText(Lang::get(Lang::Repeat1));
	m->action_repAll->setText(Lang::get(Lang::RepeatAll));
	m->action_shuffle->setText(Lang::get(Lang::Shuffle));

	check_dynamic_play_button();
}


// setting slot
void ActionMenu::s_playlist_mode_changed()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);

	if(plm == m->plm) {
		return;
	}

	m->plm = plm;

	m->action_append->setChecked( Playlist::Mode::isActive(m->plm.append()));
	m->action_rep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->action_repAll->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->action_shuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->action_dynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));

	m->action_rep1->setEnabled(Playlist::Mode::isEnabled(m->plm.rep1()));
	m->action_append->setEnabled(Playlist::Mode::isEnabled(m->plm.append()));
	m->action_repAll->setEnabled(Playlist::Mode::isEnabled(m->plm.repAll()));
	m->action_dynamic->setEnabled(Playlist::Mode::isEnabled(m->plm.dynamic()));
	m->action_shuffle->setEnabled(Playlist::Mode::isEnabled(m->plm.shuffle()));
	m->action_gapless->setEnabled(Playlist::Mode::isEnabled(m->plm.gapless()));

	check_dynamic_play_button();
}


void ActionMenu::check_dynamic_play_button()
{
	int n_libs = Library::Manager::instance()->count();

	if(n_libs == 0) {
		m->action_dynamic->setToolTip(tr("Please set library path first"));
	}

	else{
		m->action_dynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	}
}

