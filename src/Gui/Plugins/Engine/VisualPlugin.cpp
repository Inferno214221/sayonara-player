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

#include "Gui/Utils/GuiUtils.h"

#include "Interfaces/PlayManager.h"

struct VisualPlugin::Private
{
	PlayManager* playManager;
	GUI_StyleSettings* styleSettings = nullptr;
	QPushButton* btnConfig = nullptr;
	QPushButton* btnPrev = nullptr;
	QPushButton* btnNext = nullptr;
	QPushButton* btnClose = nullptr;

	QTimer* timer = nullptr;
	int timerStopped;

	Private(PlayManager* playManager) :
		playManager(playManager),
		timerStopped(true) {}
};

VisualPlugin::VisualPlugin(PlayManager* playManager, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(playManager);
}

VisualPlugin::~VisualPlugin()
{
	if(m_ecsc)
	{
		delete m_ecsc;
		m_ecsc = nullptr;
	}
}

void VisualPlugin::initUi()
{
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &VisualPlugin::playstate_changed);

	m_ecsc = new VisualColorStyleChooser(minimumWidth(), minimumHeight());
	m->styleSettings = new GUI_StyleSettings(this);

	m->timer = new QTimer();
	m->timer->setInterval(30);
	m->timerStopped = true;

	connect(m->timer, &QTimer::timeout, this, &VisualPlugin::doFadeoutStep);
	connect(m->styleSettings, &GUI_StyleSettings::sig_style_update, this, &VisualPlugin::style_changed);
}

bool VisualPlugin::hasTitle() const
{
	return false;
}

void VisualPlugin::set_button_sizes()
{
	init_buttons();

	QFont font = m->btnConfig->font();

	QFontMetrics fm = this->fontMetrics();
	int char_width = Gui::Util::textWidth(fm, "W");

	int x = 10;
	int y = 5;
	int height = fm.height() + 2;
	int width = char_width + 4;
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
	for(QPushButton* button : {m->btnConfig, m->btnPrev, m->btnNext, m->btnClose})
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

	m->btnConfig->setVisible(b);
	m->btnPrev->setVisible(b);
	m->btnNext->setVisible(b);
	m->btnClose->setVisible(b);
}

void VisualPlugin::init_buttons()
{
	if(m->btnConfig)
	{
		return;
	}

	QWidget* w = widget();
	m->btnConfig = new QPushButton(QString::fromUtf8("≡"), w);
	m->btnPrev = new QPushButton("<", w);
	m->btnNext = new QPushButton(">", w);
	m->btnClose = new QPushButton("x", w);

	m->btnClose->setFocusProxy(w);

	set_button_sizes();
	set_buttons_visible(false);

	connect(m->btnConfig, &QPushButton::clicked, this, &VisualPlugin::config_clicked);
	connect(m->btnPrev, &QPushButton::clicked, this, &VisualPlugin::prev_clicked);
	connect(m->btnNext, &QPushButton::clicked, this, &VisualPlugin::next_clicked);
	connect(m->btnClose, &QPushButton::clicked, this, &VisualPlugin::close);
	connect(m->btnClose, &QPushButton::clicked, this->parentWidget(), &QWidget::close);
}

void VisualPlugin::showEvent(QShowEvent* e)
{
	PlayerPlugin::Base::showEvent(e);
	init_buttons();
}

void VisualPlugin::config_clicked()
{
	m->styleSettings->show(currentStyleIndex());
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
	if(new_index < 0)
	{
		new_index = n_styles - 1;
	}

	update_style(new_index);
}

void VisualPlugin::update()
{
	QWidget::update();

	if(!isUiInitialized())
	{
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
	if(!isUiInitialized())
	{
		return;
	}

	m->timer->start();
	m->timerStopped = false;
}

void VisualPlugin::closeEvent(QCloseEvent* e)
{
	PlayerPlugin::Base::closeEvent(e);
	update();
}

void VisualPlugin::resizeEvent(QResizeEvent* e)
{
	PlayerPlugin::Base::resizeEvent(e);

	if(!isUiInitialized())
	{
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

		case Qt::MiddleButton:
			if(this->parentWidget())
			{
				this->parentWidget()->close();
			}
			break;

		case Qt::RightButton:
			m->styleSettings->show(currentStyleIndex());
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
	if(!m->timerStopped)
	{
		m->timerStopped = true;

		if(m->timer)
		{
			m->timer->stop();
		}
	}
}

void VisualPlugin::style_changed()
{
	update_style(currentStyleIndex());
}

