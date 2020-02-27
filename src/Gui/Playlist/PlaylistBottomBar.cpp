
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

#include "Utils/Macros.h"
#include "Utils/Message/Message.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#ifdef SAYONARA_WITH_SHUTDOWN
	#include "Gui/Shutdown/GUI_Shutdown.h"
#endif

#include "Components/LibraryManagement/LibraryManager.h"

#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

namespace Pl=::Playlist;
using Pl::BottomBarButton;
using Pl::BottomBar;

struct BottomBar::Private
{
	Playlist::Mode		plm;

#ifdef SAYONARA_WITH_SHUTDOWN
	GUI_Shutdown*		uiShutdown=nullptr;
	Shutdown*			shutdown=nullptr;
#endif

	BottomBarButton*		btnRep1=nullptr;
	BottomBarButton*		btnAppend=nullptr;
	BottomBarButton*		btnRepall=nullptr;
	BottomBarButton*		btnDynamic=nullptr;
	BottomBarButton*		btnShuffle=nullptr;
	BottomBarButton*		btnGapless=nullptr;
	BottomBarButton*		btnShutdown=nullptr;

	Private()
	{
		shutdown = Shutdown::instance();
	}

	QList<BottomBarButton*> buttons()
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
	Widget(parent)
{
	m = Pimpl::make<Private>();

	using namespace Gui;
	m->btnRep1 = new BottomBarButton(Icons::pixmap(Icons::Repeat1, Icons::ForceSayonaraIcon), this);
	m->btnRepall = new BottomBarButton(Icons::pixmap(Icons::RepeatAll, Icons::ForceSayonaraIcon), this);
	m->btnAppend = new BottomBarButton(Icons::pixmap(Icons::Append, Icons::ForceSayonaraIcon), this);
	m->btnDynamic = new BottomBarButton(Icons::pixmap(Icons::Dynamic, Icons::ForceSayonaraIcon), this);
	m->btnShuffle = new BottomBarButton(Icons::pixmap(Icons::Shuffle, Icons::ForceSayonaraIcon), this);
	m->btnGapless = new BottomBarButton(Icons::pixmap(Icons::Gapless, Icons::ForceSayonaraIcon), this);
	m->btnShutdown = new BottomBarButton(Icons::pixmap(Icons::Shutdown), this);

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

	const QList<BottomBarButton*> buttons = m->buttons();
	for(BottomBarButton* btn : buttons)
	{
		btn->setCheckable(true);
		btn->setFlat(false);
		btn->setFocusPolicy(Qt::NoFocus);
	}

	m->btnGapless->setCheckable(false);


#ifdef SAYONARA_WITH_SHUTDOWN
	m->uiShutdown = new GUI_Shutdown(this);
#endif

	m->plm = GetSetting(Set::PL_Mode);

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

	connect(m->btnShutdown, &QPushButton::clicked, this, &BottomBar::shutdown_clicked);
	connect(m->shutdown, &Shutdown::sigStarted, this, &BottomBar::shutdown_started);
	connect(m->shutdown, &Shutdown::sigStopped, this, &BottomBar::shutdown_closed);

	ListenSetting(Set::PL_Mode, BottomBar::playlistModeSettingChanged);
}

BottomBar::~BottomBar() = default;

void BottomBar::rep1Checked(bool checked)
{
	if(checked){
		m->btnRepall->setChecked(false);
		m->btnShuffle->setChecked(false);
	}

	changePlaylistMode();
}

void BottomBar::repAllChecked(bool checked)
{
	if(checked){
		m->btnRep1->setChecked(false);
	}

	changePlaylistMode();
}

void BottomBar::shuffleChecked(bool checked)
{
	if(checked){
		m->btnRep1->setChecked(false);
	}

	changePlaylistMode();
}

// internal gui slot
void BottomBar::changePlaylistMode()
{
	parentWidget()->setFocus();

	Playlist::Mode plm;

	plm.setAppend(m->btnAppend->isChecked(), m->btnAppend->isEnabled());
	plm.setRep1(m->btnRep1->isChecked(), m->btnRep1->isEnabled());
	plm.setRepAll(m->btnRepall->isChecked(), m->btnRepall->isEnabled());
	plm.setShuffle(m->btnShuffle->isChecked(), m->btnShuffle->isEnabled());
	plm.setDynamic(m->btnDynamic->isChecked(), m->btnDynamic->isEnabled());

	if(plm == m->plm){
		return;
	}

	m->plm = plm;

	SetSetting(Set::PL_Mode, m->plm);
}

void BottomBar::gaplessClicked()
{
	PlayerPlugin::Handler::instance()->showPlugin("Crossfader");
}

// setting slot
void BottomBar::playlistModeSettingChanged()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);

	if(plm == m->plm) {
		return;
	}

	m->plm = plm;

	m->btnAppend->setChecked( Playlist::Mode::isActive(m->plm.append()));
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
	int n_libs = Library::Manager::instance()->count();

	if(n_libs == 0) {
		m->btnDynamic->setToolTip(tr("Please set library path first"));
	}

	else{
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
	m->btnShutdown->setToolTip(Lang::get(Lang::Shutdown) + ": " + Lang::get(Lang::Cancel));

	checkDynamicPlayButton();
}


void BottomBar::skinChanged()
{
	Gui::Widget::skinChanged();

	QFontMetrics fm = this->fontMetrics();
	int w = (Gui::Util::textWidth(fm, "m") * 250) / 100;
	w = std::max(29, w);

	const QList<BottomBarButton*> buttons = m->buttons();
	for(BottomBarButton* btn : buttons)
	{
		QSize sz(w, w);

		btn->setFixedSize(sz);
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
	void BottomBar::shutdown_clicked()
	{
		Message::Answer answer = Message::question_yn(tr("Cancel shutdown?"));

		if(answer == Message::Answer::Yes) {
			Shutdown::instance()->stop();
		}
	}

	void BottomBar::shutdown_started(MilliSeconds time2go)
	{
		Q_UNUSED(time2go)

		bool b = Shutdown::instance()->is_running();
		m->btnShutdown->setVisible(b);
		m->btnShutdown->setChecked(b);
	}


	void BottomBar::shutdown_closed()
	{
		bool b = Shutdown::instance()->is_running();
		m->btnShutdown->setVisible(b);
		m->btnShutdown->setChecked(b);
	}

#endif

