/* PlaylistActionMenu.cpp */

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

#include "PlaylistActionMenu.h"

#include "Components/DynamicPlayback/DynamicPlaybackChecker.h"
#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"

#include <QList>
#include <QAction>

using Playlist::ActionMenu;

struct ActionMenu::Private
{
	DynamicPlaybackChecker* dynamicPlaybackChecker;

	Playlist::Mode plm;

	QAction* actionRep1 = nullptr;
	QAction* actionAppend = nullptr;
	QAction* actionRepall = nullptr;
	QAction* actionDynamic = nullptr;
	QAction* actionShuffle = nullptr;
	QAction* actionGapless = nullptr;

	Private(DynamicPlaybackChecker* dynamicPlaybackChecker) :
		dynamicPlaybackChecker(dynamicPlaybackChecker) {}

	QList<QAction*> actions()
	{
		return
			{
				actionRep1,
				actionAppend,
				actionRepall,
				actionDynamic,
				actionShuffle,
				actionGapless,
			};
	}
};

ActionMenu::ActionMenu(DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(dynamicPlaybackChecker);

	using namespace Gui;
	m->actionRep1 = new QAction(this);
	m->actionRepall = new QAction(this);
	m->actionAppend = new QAction(this);
	m->actionDynamic = new QAction(this);
	m->actionShuffle = new QAction(this);
	m->actionGapless = new QAction(this);

	const auto actions = m->actions();
	for(auto* action: actions)
	{
		action->setCheckable(true);
	}

	m->actionGapless->setCheckable(false);

	this->addActions(m->actions());

	m->plm = GetSetting(Set::PL_Mode);

	m->actionRep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->actionRepall->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->actionAppend->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->actionDynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));
	m->actionShuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));

	connect(m->actionRep1, &QAction::toggled, this, &ActionMenu::rep1Checked);
	connect(m->actionRepall, &QAction::toggled, this, &ActionMenu::repAllChecked);
	connect(m->actionAppend, &QAction::toggled, this, &ActionMenu::changePlaylistMode);
	connect(m->actionShuffle, &QAction::toggled, this, &ActionMenu::shuffleChecked);
	connect(m->actionDynamic, &QAction::toggled, this, &ActionMenu::changePlaylistMode);
	connect(m->actionGapless, &QAction::triggered, this, &ActionMenu::gaplessClicked);

	ListenSetting(Set::PL_Mode, ActionMenu::changePlaylistMode);

	languageChanged();
}

ActionMenu::~ActionMenu() = default;

void ActionMenu::rep1Checked(bool checked)
{
	if(checked)
	{
		m->actionRepall->setChecked(false);
		m->actionShuffle->setChecked(false);
	}

	changePlaylistMode();
}

void ActionMenu::repAllChecked(bool checked)
{
	if(checked)
	{
		m->actionRep1->setChecked(false);
	}

	changePlaylistMode();
}

void ActionMenu::shuffleChecked(bool checked)
{
	if(checked)
	{
		m->actionRep1->setChecked(false);
	}

	changePlaylistMode();
}

// setting slot
void ActionMenu::playlistModeSettingChanged()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);

	if(plm == m->plm)
	{
		return;
	}

	m->plm = plm;

	m->actionAppend->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->actionRep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->actionRepall->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->actionShuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->actionDynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));

	m->actionRep1->setEnabled(Playlist::Mode::isEnabled(m->plm.rep1()));
	m->actionAppend->setEnabled(Playlist::Mode::isEnabled(m->plm.append()));
	m->actionRepall->setEnabled(Playlist::Mode::isEnabled(m->plm.repAll()));
	m->actionDynamic->setEnabled(Playlist::Mode::isEnabled(m->plm.dynamic()));
	m->actionShuffle->setEnabled(Playlist::Mode::isEnabled(m->plm.shuffle()));
	m->actionGapless->setEnabled(Playlist::Mode::isEnabled(m->plm.gapless()));

	checkDynamicPlayButton();
}

// internal gui slot
void ActionMenu::changePlaylistMode()
{
	auto plm = GetSetting(Set::PL_Mode);

	plm.setAppend(m->actionAppend->isChecked(), m->actionAppend->isEnabled());
	plm.setRep1(m->actionRep1->isChecked(), m->actionRep1->isEnabled());
	plm.setRepAll(m->actionRepall->isChecked(), m->actionRepall->isEnabled());
	plm.setShuffle(m->actionShuffle->isChecked(), m->actionShuffle->isEnabled());
	plm.setDynamic(m->actionDynamic->isChecked(), m->actionDynamic->isEnabled());

	const auto isEqual = (plm == m->plm);
	if(!isEqual)
	{
		m->plm = plm;
		SetSetting(Set::PL_Mode, m->plm);
	}
}

void ActionMenu::gaplessClicked()
{
	PlayerPlugin::Handler::instance()->showPlugin("Crossfader");
}

void ActionMenu::checkDynamicPlayButton()
{
	if(m->dynamicPlaybackChecker->isDynamicPlaybackPossible())
	{
		m->actionDynamic->setToolTip(tr("Please set library path first"));
	}

	else
	{
		m->actionDynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	}
}

void ActionMenu::languageChanged()
{
	m->actionAppend->setText(Lang::get(Lang::Append));
	m->actionDynamic->setText(Lang::get(Lang::DynamicPlayback));
	m->actionGapless->setText(Lang::get(Lang::GaplessPlayback));
	m->actionRep1->setText(Lang::get(Lang::Repeat1));
	m->actionRepall->setText(Lang::get(Lang::RepeatAll));
	m->actionShuffle->setText(Lang::get(Lang::Shuffle));

	checkDynamicPlayButton();
}
