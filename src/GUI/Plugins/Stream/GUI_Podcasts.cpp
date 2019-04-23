/* GUI_Podcasts.cpp */

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

#include "GUI_Podcasts.h"
#include "GUI/Plugins/ui_GUI_Podcasts.h"
#include "Components/Streaming/Streams/StreamHandlerPodcasts.h"

#include "Utils/Language.h"

GUI_Podcasts::GUI_Podcasts(QWidget *parent) :
	GUI_AbstractStream(parent)
{}

GUI_Podcasts::~GUI_Podcasts()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_Podcasts::get_name() const
{
	return "Podcasts";
}

QString GUI_Podcasts::get_display_name() const
{
	return Lang::get(Lang::Podcasts);
}

void GUI_Podcasts::init_ui()
{
	setup_parent(this, &ui);
}

void GUI_Podcasts::retranslate_ui()
{
	GUI_AbstractStream::retranslate_ui();
	ui->retranslateUi(this);
}

QString GUI_Podcasts::get_title_fallback_name() const
{
	return tr("Podcast");
}

QLineEdit* GUI_Podcasts::le_url()
{
	return ui->le_url;
}

QComboBox* GUI_Podcasts::combo_stream()
{
	return ui->combo_stream;
}

QPushButton* GUI_Podcasts::btn_play()
{
	return ui->btn_play;
}

MenuToolButton* GUI_Podcasts::btn_menu()
{
	return ui->btn_tool;
}

QLabel*GUI_Podcasts::lab_listen()
{
	return ui->lab_listen;
}

AbstractStreamHandler* GUI_Podcasts::stream_handler() const
{
	return new StreamHandlerPodcasts();
}
