
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

#include "BottomBar.h"

#include "GUI/Utils/Icons.h"

#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Message/Message.h"

#ifdef WITH_SHUTDOWN
	#include "GUI/ShutdownDialog/GUI_Shutdown.h"
#endif

// Think about CMake
#include "Components/Library/LibraryManager.h"
#include "Interfaces/PlayerPlugin/PlayerPluginHandler.h"

#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

struct GUI_PlaylistBottomBar::Private
{
	Playlist::Mode		plm;

#ifdef WITH_SHUTDOWN
	GUI_Shutdown*		ui_shutdown=nullptr;
	Shutdown*			shutdown=nullptr;
#endif

	QPushButton*		btn_rep1=nullptr;
	QPushButton*		btn_append=nullptr;
	QPushButton*		btn_repAll=nullptr;
	QPushButton*		btn_dynamic=nullptr;
	QPushButton*		btn_shuffle=nullptr;
	QPushButton*		btn_gapless=nullptr;
	QPushButton*		btn_shutdown=nullptr;

	Private()
	{
		shutdown = Shutdown::instance();
	}

	QList<QPushButton*> buttons()
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

GUI_PlaylistBottomBar::GUI_PlaylistBottomBar(QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	using namespace Gui;
	m->btn_rep1 = new QPushButton(Icons::icon(Icons::Repeat1, Icons::ForceSayonaraIcon), "", this);
	m->btn_repAll = new QPushButton(Icons::icon(Icons::RepeatAll, Icons::ForceSayonaraIcon), "", this);
	m->btn_append = new QPushButton(Icons::icon(Icons::Append, Icons::ForceSayonaraIcon), "", this);
	m->btn_dynamic = new QPushButton(Icons::icon(Icons::Dynamic, Icons::ForceSayonaraIcon), "", this);
	m->btn_shuffle = new QPushButton(Icons::icon(Icons::Shuffle, Icons::ForceSayonaraIcon), "", this);
	m->btn_gapless = new QPushButton(Icons::icon(Icons::Gapless, Icons::ForceSayonaraIcon), "", this);
	m->btn_shutdown = new QPushButton(Icons::icon(Icons::Shutdown), "", this);

	const QList<QPushButton*> buttons = m->buttons();
	for(QPushButton* btn : buttons)
	{
		btn->setIconSize(QSize(20, 20));
		btn->setCheckable(true);
		btn->setFlat(false);
		btn->setFocusPolicy(Qt::NoFocus);
		btn->setMinimumSize(28, 28);
		btn->setMaximumSize(28, 28);
	}

	m->btn_gapless->setCheckable(false);

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


#ifdef WITH_SHUTDOWN
	m->ui_shutdown = new GUI_Shutdown(this);
#endif

	m->plm = _settings->get<Set::PL_Mode>();

	m->btn_rep1->setChecked(Playlist::Mode::isActive(m->plm.rep1()));
	m->btn_repAll->setChecked(Playlist::Mode::isActive(m->plm.repAll()));
	m->btn_append->setChecked(Playlist::Mode::isActive(m->plm.append()));
	m->btn_dynamic->setChecked(Playlist::Mode::isActive(m->plm.dynamic()));
	m->btn_shuffle->setChecked(Playlist::Mode::isActive(m->plm.shuffle()));
	m->btn_shutdown->setVisible(false);

	connect(m->btn_rep1, &QPushButton::clicked, this, &GUI_PlaylistBottomBar::rep1_checked);
	connect(m->btn_repAll, &QPushButton::clicked, this, &GUI_PlaylistBottomBar::rep_all_checked);
	connect(m->btn_append, &QPushButton::released, this, &GUI_PlaylistBottomBar::playlist_mode_changed);
	connect(m->btn_shuffle, &QPushButton::clicked, this, &GUI_PlaylistBottomBar::shuffle_checked);
	connect(m->btn_dynamic, &QPushButton::released, this, &GUI_PlaylistBottomBar::playlist_mode_changed);
	connect(m->btn_gapless, &QPushButton::clicked, this, &GUI_PlaylistBottomBar::gapless_clicked);

	connect(m->btn_shutdown, &QPushButton::clicked, this, &GUI_PlaylistBottomBar::shutdown_clicked);
	connect(m->shutdown, &Shutdown::sig_started, this, &GUI_PlaylistBottomBar::shutdown_started);
	connect(m->shutdown, &Shutdown::sig_stopped, this, &GUI_PlaylistBottomBar::shutdown_closed);

	Set::listen<Set::PL_Mode>(this, &GUI_PlaylistBottomBar::s_playlist_mode_changed);
}

GUI_PlaylistBottomBar::~GUI_PlaylistBottomBar() {}

void GUI_PlaylistBottomBar::rep1_checked(bool checked)
{
	if(checked){
		m->btn_repAll->setChecked(false);
		m->btn_shuffle->setChecked(false);
	}

	playlist_mode_changed();
}

void GUI_PlaylistBottomBar::rep_all_checked(bool checked)
{
	if(checked){
		m->btn_rep1->setChecked(false);
	}

	playlist_mode_changed();
}

void GUI_PlaylistBottomBar::shuffle_checked(bool checked)
{
	if(checked){
		m->btn_rep1->setChecked(false);
	}

	playlist_mode_changed();
}

// internal gui slot
void GUI_PlaylistBottomBar::playlist_mode_changed()
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

	_settings->set<Set::PL_Mode>(m->plm);
}

void GUI_PlaylistBottomBar::gapless_clicked()
{
	PlayerPlugin::Handler::instance()->show_plugin("Crossfader");
}

void GUI_PlaylistBottomBar::language_changed()
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

// setting slot
void GUI_PlaylistBottomBar::s_playlist_mode_changed()
{
	Playlist::Mode plm = _settings->get<Set::PL_Mode>();

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


void GUI_PlaylistBottomBar::check_dynamic_play_button()
{
	int n_libs = Library::Manager::instance()->count();

	if(n_libs == 0) {
		m->btn_dynamic->setToolTip(tr("Please set library path first"));
	}

	else{
		m->btn_dynamic->setToolTip(Lang::get(Lang::DynamicPlayback));
	}
}


#ifdef WITH_SHUTDOWN
	void GUI_PlaylistBottomBar::shutdown_clicked()
	{
		Message::Answer answer = Message::question_yn(tr("Cancel shutdown?"));

		if(answer == Message::Answer::Yes) {
			Shutdown::instance()->stop();
		}
	}

	void GUI_PlaylistBottomBar::shutdown_started(MilliSeconds time2go)
	{
		Q_UNUSED(time2go)

		bool b = Shutdown::instance()->is_running();
		m->btn_shutdown->setVisible(b);
		m->btn_shutdown->setChecked(b);
	}


	void GUI_PlaylistBottomBar::shutdown_closed()
	{
		bool b = Shutdown::instance()->is_running();
		m->btn_shutdown->setVisible(b);
		m->btn_shutdown->setChecked(b);
	}

#endif
