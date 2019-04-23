/* GUI_Stream.cpp */

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

#include "GUI_Stream.h"
#include "GUI/Plugins/ui_GUI_Stream.h"
#include "GUI/Plugins/Stream/GUI_StationSearcher.h"

#include "Components/Streaming/Streams/StreamHandlerStreams.h"

#include "Utils/Language.h"

struct GUI_Stream::Private
{
	GUI_StationSearcher* searcher = nullptr;
};

GUI_Stream::GUI_Stream(QWidget *parent) :
	GUI_AbstractStream(parent)
{
	m = Pimpl::make<Private>();
}

GUI_Stream::~GUI_Stream()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_Stream::get_name() const
{
	return "Webstreams";
}

QString GUI_Stream::get_display_name() const
{
	return Lang::get(Lang::Streams);
}

void GUI_Stream::retranslate_ui()
{
	GUI_AbstractStream::retranslate_ui();
	ui->retranslateUi(this);
}

void GUI_Stream::init_ui()
{
	setup_parent(this, &ui);

	connect(ui->le_url, &QLineEdit::textEdited, this, &GUI_Stream::text_changed);
}


QString GUI_Stream::get_title_fallback_name() const
{
	return Lang::get(Lang::Radio);
}


QLineEdit* GUI_Stream::le_url()
{
	return ui->le_url;
}

QComboBox* GUI_Stream::combo_stream()
{
	return ui->combo_stream;
}

QPushButton* GUI_Stream::btn_play()
{
	return ui->btn_play;
}

MenuToolButton* GUI_Stream::btn_menu()
{
	return ui->btn_tool;
}

QLabel* GUI_Stream::lab_listen()
{
	return ui->lab_listen;
}

AbstractStreamHandler* GUI_Stream::stream_handler() const
{
	return new StreamHandlerStreams();
}

void GUI_Stream::text_changed(const QString& text)
{
	if(text == "ss")
	{
		if(!m->searcher)
		{
			m->searcher = new GUI_StationSearcher(this);
			connect(m->searcher, &GUI_StationSearcher::sig_stream_selected, this, &GUI_Stream::stream_selected);
		}

		m->searcher->show();
	}
}

void GUI_Stream::stream_selected(const QString& name, const QString& url)
{
	ui->le_url->setText(url);
	ui->combo_stream->setCurrentText(name);
}
