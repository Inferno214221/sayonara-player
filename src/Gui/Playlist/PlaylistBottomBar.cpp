
/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "PlaylistBottomBar.h"
#include "PlaylistBottomBarButton.h"
#include "PlaylistActionMenu.h"

#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"

#include "Interfaces/DynamicPlayback.h"

#include "Utils/Macros.h"
#include "Utils/Message/Message.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#ifdef SAYONARA_WITH_SHUTDOWN

#include "Gui/Shutdown/GUI_Shutdown.h"

#endif

#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

using Playlist::BottomBar;
namespace Icons = Gui::Icons;

namespace
{
	Playlist::BottomBarButton*
	createButton(QWidget* parent, Icons::IconName iconName, Icons::IconMode iconMode = Icons::ForceSayonaraIcon)
	{
		return new Playlist::BottomBarButton {Icons::icon(iconName, iconMode), parent};
	}
}

struct BottomBar::Private
{
	Playlist::Mode plm;

	DynamicPlaybackChecker* dynamicPlaybackChecker;

#ifdef SAYONARA_WITH_SHUTDOWN
	Shutdown* shutdown {Shutdown::instance()};
#endif

	BottomBarButton* btnRep1;
	BottomBarButton* btnRepall;
	BottomBarButton* btnAppend;
	BottomBarButton* btnDynamic;
	BottomBarButton* btnShuffle;
	BottomBarButton* btnGapless;
	BottomBarButton* btnShutdown;

	Private(DynamicPlaybackChecker* dynamicPlaybackChecker, BottomBar* parent) :
		plm {GetSetting(Set::PL_Mode)},
		dynamicPlaybackChecker {dynamicPlaybackChecker},
		btnRep1 {createButton(parent, Icons::Repeat1)},
		btnRepall {createButton(parent, Icons::RepeatAll)},
		btnAppend {createButton(parent, Icons::Append)},
		btnDynamic {createButton(parent, Icons::Dynamic)},
		btnShuffle {createButton(parent, Icons::Shuffle)},
		btnGapless {createButton(parent, Icons::Gapless)},
		btnShutdown {createButton(parent, Icons::Shutdown, Icons::Automatic)} {}

	QList<BottomBarButton*> buttons() const
	{
		return {
			btnRep1,
			btnAppend,
			btnRepall,
			btnDynamic,
			btnShuffle,
			btnGapless,
			btnShutdown
		};
	}
};

BottomBar::BottomBar(QWidget* parent) :
	Widget(parent) {}

BottomBar::~BottomBar() = default;

void BottomBar::init(DynamicPlaybackChecker* dynamicPlaybackChecker)
{
	m = Pimpl::make<Private>(dynamicPlaybackChecker, this);

	using namespace Gui;

	QLayout* layout = new QHBoxLayout(this);
	this->setLayout(layout);

	layout->addWidget(m->btnRep1);
	layout->addWidget(m->btnRepall);
	layout->addWidget(m->btnShuffle);
	layout->addWidget(m->btnDynamic);
	layout->addWidget(m->btnAppend);
	layout->addWidget(m->btnGapless);
	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding));
	layout->addWidget(m->btnShutdown);

	layout->setContentsMargins(3, 2, 3, 5);
	layout->setSpacing(5);

	const auto buttons = m->buttons();
	for(auto* button : buttons)
	{
		button->setCheckable(true);
		button->setFlat(false);
		button->setFocusPolicy(Qt::NoFocus);
	}

	m->btnGapless->setCheckable(false);

	m->btnRep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->btnRepall->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->btnAppend->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->btnDynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));
	m->btnShuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->btnShutdown->setVisible(false);

	connect(m->btnRep1, &QPushButton::clicked, this, &BottomBar::rep1Checked);
	connect(m->btnRepall, &QPushButton::clicked, this, &BottomBar::repAllChecked);
	connect(m->btnAppend, &QPushButton::released, this, &BottomBar::changePlaylistMode);
	connect(m->btnShuffle, &QPushButton::clicked, this, &BottomBar::shuffleChecked);
	connect(m->btnDynamic, &QPushButton::released, this, &BottomBar::changePlaylistMode);
	connect(m->btnGapless, &QPushButton::clicked, this, &BottomBar::gaplessClicked);

	connect(m->btnShutdown, &QPushButton::clicked, this, &BottomBar::shutdownClicked);
	connect(m->shutdown, &Shutdown::sigStarted, this, &BottomBar::shutdownStarted);
	connect(m->shutdown, &Shutdown::sigStopped, this, &BottomBar::shutdownClosed);

	ListenSetting(Set::PL_Mode, BottomBar::playlistModeSettingChanged);
}

void BottomBar::rep1Checked(bool checked)
{
	if(checked)
	{
		m->btnRepall->setChecked(false);
		m->btnShuffle->setChecked(false);
	}

	changePlaylistMode();
}

void BottomBar::repAllChecked(bool checked)
{
	if(checked)
	{
		m->btnRep1->setChecked(false);
	}

	changePlaylistMode();
}

void BottomBar::shuffleChecked(bool checked)
{
	if(checked)
	{
		m->btnRep1->setChecked(false);
	}

	changePlaylistMode();
}

void BottomBar::changePlaylistMode()
{
	parentWidget()->setFocus();

	auto plm = GetSetting(Set::PL_Mode);

	plm.setAppend(m->btnAppend->isChecked(), m->btnAppend->isEnabled());
	plm.setRep1(m->btnRep1->isChecked(), m->btnRep1->isEnabled());
	plm.setRepAll(m->btnRepall->isChecked(), m->btnRepall->isEnabled());
	plm.setShuffle(m->btnShuffle->isChecked(), m->btnShuffle->isEnabled());
	plm.setDynamic(m->btnDynamic->isChecked(), m->btnDynamic->isEnabled());

	if(const auto isEqual = (plm == m->plm); !isEqual)
	{
		m->plm = plm;
		SetSetting(Set::PL_Mode, m->plm);
	}
}

void BottomBar::gaplessClicked()
{
	PlayerPlugin::Handler::instance()->showPlugin("Crossfader");
}

void BottomBar::playlistModeSettingChanged()
{
	const auto plm = GetSetting(Set::PL_Mode);
	if(plm == m->plm)
	{
		return;
	}

	m->plm = plm;

	m->btnAppend->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->btnRep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->btnRepall->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->btnShuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->btnDynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));

	m->btnRep1->setEnabled(Playlist::Mode::isEnabled(m->plm.rep1()));
	m->btnAppend->setEnabled(Playlist::Mode::isEnabled(m->plm.append()));
	m->btnRepall->setEnabled(Playlist::Mode::isEnabled(m->plm.repAll()));
	m->btnDynamic->setEnabled(Playlist::Mode::isEnabled(m->plm.dynamic()));
	m->btnShuffle->setEnabled(Playlist::Mode::isEnabled(m->plm.shuffle()));
	m->btnGapless->setEnabled(Playlist::Mode::isEnabled(m->plm.gapless()));

	checkDynamicPlayButton();
}

void BottomBar::checkDynamicPlayButton()
{
	if(m->dynamicPlaybackChecker->isDynamicPlaybackPossible())
	{
		m->btnDynamic->setToolTip(tr("Please set library path first"));
	}

	else
	{
		m->btnDynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	}
}

void BottomBar::languageChanged()
{
	m->btnAppend->setToolTip(Lang::get(Lang::Append));
	m->btnDynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	m->btnGapless->setToolTip(Lang::get(Lang::GaplessPlayback));
	m->btnRep1->setToolTip(Lang::get(Lang::Repeat1));
	m->btnRepall->setToolTip(Lang::get(Lang::RepeatAll));
	m->btnShuffle->setToolTip(Lang::get(Lang::Shuffle));
	m->btnShutdown->setToolTip(
		Lang::get(Lang::Shutdown) + ": " + Lang::get(Lang::Cancel));

	checkDynamicPlayButton();
}

void BottomBar::skinChanged()
{
	const auto fm = this->fontMetrics();
	auto width = (Gui::Util::textWidth(fm, "m") * 250) / 100;
	width = std::max(29, width);

	const auto buttons = m->buttons();
	for(auto* button : buttons)
	{
		button->setFixedSize(QSize {width, width});
	}
}

void BottomBar::showEvent(QShowEvent* e)
{
	Gui::Widget::showEvent(e);
	skinChanged();
}

void BottomBar::resizeEvent(QResizeEvent* e)
{
	Gui::Widget::resizeEvent(e);
	skinChanged();
}

#ifdef SAYONARA_WITH_SHUTDOWN

void BottomBar::shutdownClicked()
{
	const auto answer = Message::question_yn(tr("Cancel shutdown?"));
	if(answer == Message::Answer::Yes)
	{
		Shutdown::instance()->stop();
	}
}

void BottomBar::shutdownStarted([[maybe_unused]] MilliSeconds time2go)
{
	const auto b = Shutdown::instance()->is_running();
	m->btnShutdown->setVisible(b);
	m->btnShutdown->setChecked(b);
}

void BottomBar::shutdownClosed()
{
	const auto b = Shutdown::instance()->is_running();
	m->btnShutdown->setVisible(b);
	m->btnShutdown->setChecked(b);
}

#endif

