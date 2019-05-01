/* EnginePlugin.cpp */

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

#include "EnginePlugin.h"
#include "EngineColorStyleChooser.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/PlayManager/PlayManager.h"

#include <QResizeEvent>

struct EnginePlugin::Private
{
	GUI_StyleSettings*	style_settings=nullptr;

	PlayManagerPtr		play_manager=nullptr;
	Engine::Handler*	engine=nullptr;
	QPushButton*		btn_config=nullptr;
	QPushButton*		btn_prev=nullptr;
	QPushButton*		btn_next=nullptr;
	QPushButton*		btn_close=nullptr;

	QTimer*				timer=nullptr;
	int					timer_stopped;

	Private() :
		timer_stopped(true)
	{
		play_manager = PlayManager::instance();
		engine = Engine::Handler::instance();
	}
};

EnginePlugin::EnginePlugin(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}


EnginePlugin::~EnginePlugin()
{
	if(_ecsc){
		delete _ecsc; _ecsc = nullptr;
	}
}


void EnginePlugin::init_ui()
{
	connect(m->play_manager, &PlayManager::sig_playstate_changed, this, &EnginePlugin::playstate_changed);

	_ecsc = new EngineColorStyleChooser(minimumWidth(), minimumHeight());
	m->style_settings = new GUI_StyleSettings(this);

	m->timer = new QTimer();
	m->timer->setInterval(30);
	m->timer_stopped = true;

	QWidget* w = widget();

	m->btn_config = new QPushButton(QString::fromUtf8("≡"), w);
	m->btn_prev = new QPushButton("<", w);
	m->btn_next = new QPushButton(">", w);
	m->btn_close = new QPushButton("x", w);

	m->btn_close->setFocusProxy(w);

	init_buttons( has_small_buttons() );

	connect(m->timer, &QTimer::timeout, this, &EnginePlugin::do_fadeout_step);
	connect(m->style_settings, &GUI_StyleSettings::sig_style_update, this, &EnginePlugin::style_changed);
}


bool EnginePlugin::is_title_shown() const
{
	return false;
}


void EnginePlugin::init_buttons(bool small)
{
	int height = this->height() - 4;
	int x = 10;
	int y;
	int width;

	if(small){
		width = 15;
		y = 5;
	}

	else{
		width = 20;
		y = 10;
	}

	m->btn_config->setGeometry(x, y, width, height);
	x += width + 5;
	m->btn_prev->setGeometry(x, y, width, height);
	x += width + 5;
	m->btn_next->setGeometry(x, y, width, height);
	x += width + 5;
	m->btn_close->setGeometry(x, y, width, height);

	connect(m->btn_config, &QPushButton::clicked, this, &EnginePlugin::config_clicked);
	connect(m->btn_prev, &QPushButton::clicked, this, &EnginePlugin::prev_clicked);
	connect(m->btn_next, &QPushButton::clicked, this, &EnginePlugin::next_clicked);
	connect(m->btn_close, &QPushButton::clicked, this, &EnginePlugin::close);
	connect(m->btn_close, &QPushButton::clicked, this->parentWidget(), &QWidget::close);

	m->btn_config->hide();
	m->btn_prev->hide();
	m->btn_next->hide();
	m->btn_close->hide();
}

Engine::Handler *EnginePlugin::engine() const
{
	return m->engine;
}

void EnginePlugin::config_clicked()
{
	m->style_settings->show(current_style_index());
}


void EnginePlugin::next_clicked()
{
	int n_styles = _ecsc->get_num_color_schemes();

	int new_index = (current_style_index() + 1) % n_styles;

	update_style(new_index);
}


void EnginePlugin::prev_clicked()
{
	int n_styles = _ecsc->get_num_color_schemes();

	int new_index = (current_style_index() - 1);
	if(new_index < 0){
		new_index = n_styles - 1;
	}

	update_style(new_index);
}


void EnginePlugin::update()
{
	QWidget::update();

	if(!is_ui_initialized()){
		return;
	}
}


void EnginePlugin::playstate_changed(PlayState state)
{
	switch(state)
	{
		case PlayState::Playing:
			played();
			break;
		case PlayState::Paused:
			paused();
			break;
		case PlayState::Stopped:
			stopped();
			break;
		default:
			break;
	}
}

void EnginePlugin::played() {}
void EnginePlugin::paused() {}

void EnginePlugin::stopped()
{
	if(!is_ui_initialized()){
		return;
	}

	m->timer->start();
	m->timer_stopped = false;
}


void EnginePlugin::closeEvent(QCloseEvent *e)
{
	PlayerPlugin::Base::closeEvent(e);
	update();
}


void EnginePlugin::resizeEvent(QResizeEvent* e)
{
	PlayerPlugin::Base::resizeEvent(e);

	if(!is_ui_initialized()){
		return;
	}

	update_style(current_style_index());

	QSize new_size = e->size();

	if(!m->btn_config) return;

	if(new_size.height() >= 30){
		m->btn_config->setGeometry(10, 10, 20, 20);
		m->btn_prev->setGeometry(35, 10, 20, 20);
		m->btn_next->setGeometry(60, 10, 20, 20);
		m->btn_close->setGeometry(85, 10, 20, 20);

		QFont font = m->btn_config->font();
		font.setPointSize(8);
		m->btn_config->setFont(font);
		m->btn_prev->setFont(font);
		m->btn_next->setFont(font);
		m->btn_close->setFont(font);
	}

	else {
		m->btn_config->setGeometry(10, 5, 15, 15);
		m->btn_prev->setGeometry(30, 5, 15, 15);
		m->btn_next->setGeometry(50, 5, 15, 15);
		m->btn_close->setGeometry(70, 5, 15, 15);

		QFont font = m->btn_config->font();
		font.setPointSize(6);
		m->btn_config->setFont(font);
		m->btn_prev->setFont(font);
		m->btn_next->setFont(font);
		m->btn_close->setFont(font);
	}
}


void EnginePlugin::mousePressEvent(QMouseEvent *e)
{
	switch(e->button())
	{
		case Qt::LeftButton:
			next_clicked();
			break;

		case Qt::MidButton:
			if(this->parentWidget()){
				this->parentWidget()->close();
			}
			break;

		case Qt::RightButton:
			m->style_settings->show(current_style_index());
			break;
		default:
			break;
	}
}


void EnginePlugin::enterEvent(QEvent* e)
{
	PlayerPlugin::Base::enterEvent(e);

	m->btn_config->show();
	m->btn_prev->show();
	m->btn_next->show();
	m->btn_close->show();
}

void EnginePlugin::leaveEvent(QEvent* e)
{
	PlayerPlugin::Base::leaveEvent(e);

	m->btn_config->hide();
	m->btn_prev->hide();
	m->btn_next->hide();
	m->btn_close->hide();
}


void EnginePlugin::stop_fadeout_timer()
{
	if(!m->timer_stopped )
	{
		m->timer_stopped = true;

		if(m->timer) {
			m->timer->stop();
		}
	}
}

void EnginePlugin::style_changed()
{
	update_style(current_style_index());
}

