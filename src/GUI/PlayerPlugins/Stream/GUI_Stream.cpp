/* GUI_Stream.cpp */

/* Copyright (C) 2011-2016  Lucio Carreras
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

GUI_Stream::GUI_Stream(QWidget *parent) :
	GUI_AbstractStream(new StreamHandlerStreams(), parent),
	Ui::GUI_Stream()
{
	_title_fallback_name = tr("Radio Station");
	setup_parent(this);
}


GUI_Stream::~GUI_Stream() {

}

QString GUI_Stream::get_name() const
{
	return "Webstreams";
}

QString GUI_Stream::get_display_name() const
{
	return tr("Webstreams");
}

QLabel *GUI_Stream::get_title_label() const
{
	return lab_title;
}

QPushButton* GUI_Stream::get_close_button() const
{
	return btn_close;
}

void GUI_Stream::language_changed() {
	retranslateUi(this);
}

