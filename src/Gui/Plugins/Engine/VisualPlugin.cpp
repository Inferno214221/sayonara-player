/* VisualPlugin.cpp */

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

#include "VisualPlugin.h"
#include "VisualColorStyleChooser.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Gui/Utils/GuiUtils.h"

struct VisualPlugin::Private
{
	GUI_StyleSettings*	style_settings=nullptr;
	QPushButton*		btn_config=nullptr;
	QPushButton*		btn_prev=nullptr;
	QPushButton*		btn_next=nullptr;
	QPushButton*		btn_close=nullptr;

	QTimer*				timer=nullptr;
	int					timer_stopped;

	Private() :
		timer_stopped(true)
	{}
};

VisualPlugin::VisualPlugin(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}


VisualPlugin::~VisualPlugin()
{
	if(m_ecsc){
		delete m_ecsc; m_ecsc = nullptr;
	}
}


void VisualPlugin::initUi()
{
	connect(PlayManager::instance(), &PlayManager::sigPlaystateChanged, this, &VisualPlugin::playstate_changed);

	m_ecsc = new VisualColorStyleChooser(minimumWidth(), minimumHeight());
	m->style_settings = new GUI_StyleSettings(this);

	m->timer = new QTimer();
	m->timer->setInterval(30);
	m->timer_stopped = true;

	connect(m->timer, &QTimer::timeout, this, &VisualPlugin::doFadeoutStep);
	connect(m->style_settings, &GUI_StyleSettings::sig_style_update, this, &VisualPlugin::style_changed);
}


bool VisualPlugin::hasTitle() const
{
	return false;
}


void VisualPlugin::set_button_sizes()
{
	init_buttons();

	QFont font = m->btn_config->font();

	QFontMetrics fm = this->fontMetrics();
	int char_width = Gui::Util::textWidth(fm, "W");

	int x = 10;
	int y = 5;
	int height = fm.height() + 2;
	int width =  char_width + 4;
	int font_size = 6;

	if(!hasSmallButtons())
	{
		y = 5;
		height = fm.height() * 2;
		width = char_width * 2;
		font_size = 8;
	}

	font.setPointSize(font_size);

	//QList<QPushButton*> buttons ;
	for(QPushButton* button : { m->btn_config, m->btn_prev, m->btn_next, m->btn_close })
	{
		button->setFont(font);
		button->setMaximumHeight(height);
		button->setMaximumWidth(width);
		button->setGeometry(x, y, width, height);

		QString css = QString("padding: 0; margin: 0; max-height: %1; min-height: %1;").arg(height);
		button->setStyleSheet(css);

		x += width + 5;
	}
}

void VisualPlugin::set_buttons_visible(bool b)
{
	init_buttons();

	m->btn_config->setVisible(b);
	m->btn_prev->setVisible(b);
	m->btn_next->setVisible(b);
	m->btn_close->setVisible(b);
}

void VisualPlugin::init_buttons()
{
	if(m->btn_config){
		return;
	}

	QWidget* w = widget();
	m->btn_config = new QPushButton(QString::fromUtf8("≡"), w);
	m->btn_prev = new QPushButton("<", w);
	m->btn_next = new QPushButton(">", w);
	m->btn_close = new QPushButton("x", w);

	m->btn_close->setFocusProxy(w);

	set_button_sizes();
	set_buttons_visible(false);

	connect(m->btn_config, &QPushButton::clicked, this, &VisualPlugin::config_clicked);
	connect(m->btn_prev, &QPushButton::clicked, this, &VisualPlugin::prev_clicked);
	connect(m->btn_next, &QPushButton::clicked, this, &VisualPlugin::next_clicked);
	connect(m->btn_close, &QPushButton::clicked, this, &VisualPlugin::close);
	connect(m->btn_close, &QPushButton::clicked, this->parentWidget(), &QWidget::close);
}

void VisualPlugin::showEvent(QShowEvent* e)
{
	PlayerPlugin::Base::showEvent(e);
	init_buttons();
}

void VisualPlugin::config_clicked()
{
	m->style_settings->show(currentStyleIndex());
}


void VisualPlugin::next_clicked()
{
	int n_styles = m_ecsc->get_num_color_schemes();

	int new_index = (currentStyleIndex() + 1) % n_styles;

	update_style(new_index);
}


void VisualPlugin::prev_clicked()
{
	int n_styles = m_ecsc->get_num_color_schemes();

	int new_index = (currentStyleIndex() - 1);
	if(new_index < 0){
		new_index = n_styles - 1;
	}

	update_style(new_index);
}


void VisualPlugin::update()
{
	QWidget::update();

	if(!isUiInitialized()){
		return;
	}
}


void VisualPlugin::playstate_changed(PlayState state)
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

void VisualPlugin::played() {}
void VisualPlugin::paused() {}

void VisualPlugin::stopped()
{
	if(!isUiInitialized()){
		return;
	}

	m->timer->start();
	m->timer_stopped = false;
}


void VisualPlugin::closeEvent(QCloseEvent* e)
{
	PlayerPlugin::Base::closeEvent(e);
	update();
}


void VisualPlugin::resizeEvent(QResizeEvent* e)
{
	PlayerPlugin::Base::resizeEvent(e);

	if(!isUiInitialized()){
		return;
	}

	update_style(currentStyleIndex());
	set_button_sizes();
}


void VisualPlugin::mousePressEvent(QMouseEvent* e)
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
			m->style_settings->show(currentStyleIndex());
			break;
		default:
			break;
	}
}


void VisualPlugin::enterEvent(QEvent* e)
{
	PlayerPlugin::Base::enterEvent(e);

	set_button_sizes();
	set_buttons_visible(true);

}

void VisualPlugin::leaveEvent(QEvent* e)
{
	PlayerPlugin::Base::leaveEvent(e);

	set_buttons_visible(false);
}


void VisualPlugin::stop_fadeout_timer()
{
	if(!m->timer_stopped )
	{
		m->timer_stopped = true;

		if(m->timer) {
			m->timer->stop();
		}
	}
}

void VisualPlugin::style_changed()
{
	update_style(currentStyleIndex());
}

