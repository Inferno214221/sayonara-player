
/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Utils/Macros.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"

#ifdef SAYONARA_WITH_SHUTDOWN
	#include "Gui/Shutdown/GUI_Shutdown.h"
#endif

// Think about CMake
#include "Components/Library/LibraryManager.h"

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
	GUI_Shutdown*		ui_shutdown=nullptr;
	Shutdown*			shutdown=nullptr;
#endif

	BottomBarButton*		btn_rep1=nullptr;
	BottomBarButton*		btn_append=nullptr;
	BottomBarButton*		btn_repAll=nullptr;
	BottomBarButton*		btn_dynamic=nullptr;
	BottomBarButton*		btn_shuffle=nullptr;
	BottomBarButton*		btn_gapless=nullptr;
	BottomBarButton*		btn_shutdown=nullptr;

	Private()
	{
		shutdown = Shutdown::instance();
	}

	QList<BottomBarButton*> buttons()
	{
		return {
			btn_rep1,
			btn_append,
			btn_repAll,
			btn_dynamic,
			btn_shuffle,
			btn_gapless,
			btn_shutdown
		};
	}
};

BottomBar::BottomBar(QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	using namespace Gui;
	m->btn_rep1 = new BottomBarButton(Icons::pixmap(Icons::Repeat1, Icons::ForceSayonaraIcon), this);
	m->btn_repAll = new BottomBarButton(Icons::pixmap(Icons::RepeatAll, Icons::ForceSayonaraIcon), this);
	m->btn_append = new BottomBarButton(Icons::pixmap(Icons::Append, Icons::ForceSayonaraIcon), this);
	m->btn_dynamic = new BottomBarButton(Icons::pixmap(Icons::Dynamic, Icons::ForceSayonaraIcon), this);
	m->btn_shuffle = new BottomBarButton(Icons::pixmap(Icons::Shuffle, Icons::ForceSayonaraIcon), this);
	m->btn_gapless = new BottomBarButton(Icons::pixmap(Icons::Gapless, Icons::ForceSayonaraIcon), this);
	m->btn_shutdown = new BottomBarButton(Icons::pixmap(Icons::Shutdown), this);

	QLayout* layout = new QHBoxLayout(this);
	this->setLayout(layout);

	layout->addWidget(m->btn_rep1);
	layout->addWidget(m->btn_repAll);
	layout->addWidget(m->btn_shuffle);
	layout->addWidget(m->btn_dynamic);
	layout->addWidget(m->btn_append);
	layout->addWidget(m->btn_gapless);
	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding));
	layout->addWidget(m->btn_shutdown);

	layout->setContentsMargins(3, 2, 3, 5);
	layout->setSpacing(5);

	const QList<BottomBarButton*> buttons = m->buttons();
	for(BottomBarButton* btn : buttons)
	{
		btn->setCheckable(true);
		btn->setFlat(false);
		btn->setFocusPolicy(Qt::NoFocus);
	}

	m->btn_gapless->setCheckable(false);


#ifdef SAYONARA_WITH_SHUTDOWN
	m->ui_shutdown = new GUI_Shutdown(this);
#endif

	m->plm = GetSetting(Set::PL_Mode);

	m->btn_rep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->btn_repAll->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->btn_append->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->btn_dynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));
	m->btn_shuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->btn_shutdown->setVisible(false);

	connect(m->btn_rep1, &QPushButton::clicked, this, &BottomBar::rep1_checked);
	connect(m->btn_repAll, &QPushButton::clicked, this, &BottomBar::rep_all_checked);
	connect(m->btn_append, &QPushButton::released, this, &BottomBar::playlist_mode_changed);
	connect(m->btn_shuffle, &QPushButton::clicked, this, &BottomBar::shuffle_checked);
	connect(m->btn_dynamic, &QPushButton::released, this, &BottomBar::playlist_mode_changed);
	connect(m->btn_gapless, &QPushButton::clicked, this, &BottomBar::gapless_clicked);

	connect(m->btn_shutdown, &QPushButton::clicked, this, &BottomBar::shutdown_clicked);
	connect(m->shutdown, &Shutdown::sig_started, this, &BottomBar::shutdown_started);
	connect(m->shutdown, &Shutdown::sig_stopped, this, &BottomBar::shutdown_closed);

	ListenSetting(Set::PL_Mode, BottomBar::s_playlist_mode_changed);
}

BottomBar::~BottomBar() = default;

void BottomBar::rep1_checked(bool checked)
{
	if(checked){
		m->btn_repAll->setChecked(false);
		m->btn_shuffle->setChecked(false);
	}

	playlist_mode_changed();
}

void BottomBar::rep_all_checked(bool checked)
{
	if(checked){
		m->btn_rep1->setChecked(false);
	}

	playlist_mode_changed();
}

void BottomBar::shuffle_checked(bool checked)
{
	if(checked){
		m->btn_rep1->setChecked(false);
	}

	playlist_mode_changed();
}

// internal gui slot
void BottomBar::playlist_mode_changed()
{
	parentWidget()->setFocus();

	Playlist::Mode plm;

	plm.setAppend(m->btn_append->isChecked(), m->btn_append->isEnabled());
	plm.setRep1(m->btn_rep1->isChecked(), m->btn_rep1->isEnabled());
	plm.setRepAll(m->btn_repAll->isChecked(), m->btn_repAll->isEnabled());
	plm.setShuffle(m->btn_shuffle->isChecked(), m->btn_shuffle->isEnabled());
	plm.setDynamic(m->btn_dynamic->isChecked(), m->btn_dynamic->isEnabled());

	if(plm == m->plm){
		return;
	}

	m->plm = plm;

	SetSetting(Set::PL_Mode, m->plm);
}

void BottomBar::gapless_clicked()
{
	PlayerPlugin::Handler::instance()->show_plugin("Crossfader");
}

// setting slot
void BottomBar::s_playlist_mode_changed()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);

	if(plm == m->plm) {
		return;
	}

	m->plm = plm;

	m->btn_append->setChecked( Playlist::Mode::isActive(m->plm.append()));
	m->btn_rep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->btn_repAll->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->btn_shuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->btn_dynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));

	m->btn_rep1->setEnabled(Playlist::Mode::isEnabled(m->plm.rep1()));
	m->btn_append->setEnabled(Playlist::Mode::isEnabled(m->plm.append()));
	m->btn_repAll->setEnabled(Playlist::Mode::isEnabled(m->plm.repAll()));
	m->btn_dynamic->setEnabled(Playlist::Mode::isEnabled(m->plm.dynamic()));
	m->btn_shuffle->setEnabled(Playlist::Mode::isEnabled(m->plm.shuffle()));
	m->btn_gapless->setEnabled(Playlist::Mode::isEnabled(m->plm.gapless()));

	check_dynamic_play_button();
}


void BottomBar::check_dynamic_play_button()
{
	int n_libs = Library::Manager::instance()->count();

	if(n_libs == 0) {
		m->btn_dynamic->setToolTip(tr("Please set library path first"));
	}

	else{
		m->btn_dynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	}
}


void BottomBar::language_changed()
{
	m->btn_append->setToolTip(Lang::get(Lang::Append));
	m->btn_dynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	m->btn_gapless->setToolTip(Lang::get(Lang::GaplessPlayback));
	m->btn_rep1->setToolTip(Lang::get(Lang::Repeat1));
	m->btn_repAll->setToolTip(Lang::get(Lang::RepeatAll));
	m->btn_shuffle->setToolTip(Lang::get(Lang::Shuffle));
	m->btn_shutdown->setToolTip(Lang::get(Lang::Shutdown) + ": " + Lang::get(Lang::Cancel));

	check_dynamic_play_button();
}


void BottomBar::skin_changed()
{
	Gui::Widget::skin_changed();

	QFontMetrics fm = this->fontMetrics();
	int w = (this->fontMetrics().width("m") * 250) / 100;
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

	skin_changed();
}

void BottomBar::resizeEvent(QResizeEvent* e)
{
	Gui::Widget::resizeEvent(e);

	skin_changed();
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
		m->btn_shutdown->setVisible(b);
		m->btn_shutdown->setChecked(b);
	}


	void BottomBar::shutdown_closed()
	{
		bool b = Shutdown::instance()->is_running();
		m->btn_shutdown->setVisible(b);
		m->btn_shutdown->setChecked(b);
	}

#endif

