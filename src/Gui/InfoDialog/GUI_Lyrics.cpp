/* GUI_Lyrics.cpp */

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

#include "GUI_Lyrics.h"
#include "Gui/InfoDialog/ui_GUI_Lyrics.h"
#include "Gui/Utils/Widgets/ProgressBar.h"

#include "Components/Lyrics/Lyrics.h"
#include "Components/Lyrics/LyricLookup.h"

#include "Gui/Utils/Widgets/Completer.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include <QWheelEvent>
#include <QShowEvent>
#include <QShortcut>

#include <cmath>

using Gui::ProgressBar;
using Gui::Completer;

struct GUI_Lyrics::Private
{
	Lyrics::Lyrics*	lyrics=nullptr;
	ProgressBar*    loading_bar=nullptr;
	qreal           font_size;
	qreal           initial_font_size;

	Private(QObject* parent)
	{
		lyrics = new Lyrics::Lyrics(parent);
	}
};

GUI_Lyrics::GUI_Lyrics(QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(this);
}

GUI_Lyrics::~GUI_Lyrics()
{
	if(ui){
		delete ui;
	}

	ui = nullptr;
}


void GUI_Lyrics::init()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_Lyrics();
	ui->setupUi(this);

	ui->te_lyrics->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->te_lyrics->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	m->loading_bar = new ProgressBar(ui->te_lyrics);
	m->loading_bar->set_position(ProgressBar::Position::Bottom);
	m->loading_bar->setVisible(false);

	QString server = GetSetting(Set::Lyrics_Server);
	QStringList servers = m->lyrics->servers();

	ui->combo_servers->addItems(servers);
	int idx = ui->combo_servers->findText(server);
	if(idx < 0){
		idx = 0;
	}

	ui->combo_servers->setCurrentIndex(idx);
	ui->le_artist->setText(m->lyrics->artist());
	ui->le_title->setText(m->lyrics->title());

	int zoom_factor = GetSetting(Set::Lyrics_Zoom);
	m->font_size = QApplication::font().pointSizeF();
	m->initial_font_size = QApplication::font().pointSizeF();
	ui->sb_zoom->setValue(zoom_factor);

	zoom( (zoom_factor * m->initial_font_size) / 100.0 );

	connect(ui->combo_servers, combo_activated_int, this, &GUI_Lyrics::lyric_server_changed);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_Lyrics::prepare_lyrics);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_Lyrics::sig_closed);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_Lyrics::close);
	connect(ui->btn_switch, &QPushButton::clicked, this, &GUI_Lyrics::switch_pressed);
	connect(ui->sb_zoom, spinbox_value_changed_int, this, [=](int percent){
		zoom( (percent * m->initial_font_size) / 100.0 );
	});

	connect(ui->btn_save_lyrics, &QPushButton::clicked, this, &GUI_Lyrics::save_lyrics_clicked);
	connect(m->lyrics, &Lyrics::Lyrics::sig_lyrics_fetched, this, &GUI_Lyrics::lyrics_fetched);

	setup_sources();
	prepare_lyrics();

	new QShortcut(QKeySequence(QKeySequence::ZoomIn), this, SLOT(zoom_in()), nullptr, Qt::WidgetWithChildrenShortcut);
	new QShortcut(QKeySequence(QKeySequence::ZoomOut), this, SLOT(zoom_out()), nullptr, Qt::WidgetWithChildrenShortcut);
}


void GUI_Lyrics::lyric_server_changed(int idx)
{
	Q_UNUSED(idx)

	if(ui->combo_servers->currentData().toInt() >= 0) {
		SetSetting(Set::Lyrics_Server, ui->combo_servers->currentText());
	}

	prepare_lyrics();
}

void GUI_Lyrics::save_lyrics_clicked()
{
	m->lyrics->save_lyrics(ui->te_lyrics->toPlainText());

	setup_sources();
	set_save_button_text();
}

void GUI_Lyrics::prepare_lyrics()
{
	if(!ui){
		return;
	}

	ui->te_lyrics->clear();

	int current_server_index = ui->combo_servers->currentData().toInt();
	if(current_server_index < 0) {
		show_local_lyrics();
	}

	else
	{
		bool running = m->lyrics->fetch_lyrics
		(
					ui->le_artist->text(),
					ui->le_title->text(),
					current_server_index
		);

		if(running)
		{
			m->loading_bar->show();
			m->loading_bar->setVisible(true);
			ui->btn_search->setEnabled(false);
			ui->combo_servers->setEnabled(false);
			ui->btn_save_lyrics->setEnabled(false);
		}
	}
}

void GUI_Lyrics::show_lyrics(const QString& lyrics, const QString& header, bool rich)
{
	if(!ui){
		return;
	}

	if(rich){
		ui->te_lyrics->setHtml(lyrics);
	}
	else {
		ui->te_lyrics->setPlainText(lyrics);
	}

	ui->lab_header->setText(header);
	ui->btn_search->setEnabled(true);
	ui->combo_servers->setEnabled(true);
	ui->btn_save_lyrics->setEnabled(m->lyrics->is_lyric_tag_supported());
	m->loading_bar->setVisible(false);
}

void GUI_Lyrics::show_local_lyrics()
{
	show_lyrics(m->lyrics->local_lyrics(), m->lyrics->local_lyric_header(), false);
}

void GUI_Lyrics::lyrics_fetched()
{
	show_lyrics(m->lyrics->lyrics(), m->lyrics->lyric_header(), true);
}

void GUI_Lyrics::set_metadata(const MetaData& md)
{
	m->lyrics->set_metadata(md);

	if(!ui){
		return;
	}

	ui->le_artist->setText(m->lyrics->artist());
	ui->le_title->setText(m->lyrics->title());

	QStringList completer_entries;
	completer_entries << md.artist() << md.album_artist();
	completer_entries.removeDuplicates();

	if(ui->le_artist->completer() != nullptr){
		ui->le_artist->completer()->deleteLater();
	}

	ui->le_artist->setCompleter( new Gui::Completer(completer_entries, ui->le_artist) );

	setup_sources();
	prepare_lyrics();
	set_save_button_text();
}

void GUI_Lyrics::switch_pressed()
{
	QString artist = ui->le_artist->text();
	QString title = ui->le_title->text();

	ui->le_artist->setText(title);
	ui->le_title->setText(artist);
}

void GUI_Lyrics::zoom(qreal font_size)
{
	m->font_size = std::min(30.0, font_size);
	m->font_size = std::max(5.0, font_size);

	ui->te_lyrics->setStyleSheet("font-size: " + QString::number(m->font_size) + "pt;");
	SetSetting(Set::Lyrics_Zoom, ui->sb_zoom->value());
}

void GUI_Lyrics::setup_sources()
{
	ui->combo_servers->clear();

	if(m->lyrics->is_lyric_tag_available()){
		ui->combo_servers->addItem(Lang::get(Lang::File), -1);
	}

	int i=0;
	const QStringList servers = m->lyrics->servers();
	for(const QString& str : servers){
		ui->combo_servers->addItem(str, i++);
	}

	choose_source();
}

void GUI_Lyrics::choose_source()
{
	int new_index = 0;

	if(!m->lyrics->is_lyric_tag_available())
	{
		QString last_server = GetSetting(Set::Lyrics_Server);
		new_index = std::max(0, ui->combo_servers->findText(last_server));
	}

	ui->combo_servers->setCurrentIndex(new_index);
}

void GUI_Lyrics::zoom_in()
{
	zoom(m->font_size + 1.0);
}

void GUI_Lyrics::zoom_out()
{
	zoom(m->font_size - 1.0);
}

void GUI_Lyrics::set_save_button_text()
{
	if(!m->lyrics->is_lyric_tag_supported())
	{
		ui->btn_save_lyrics->setEnabled(false);
		ui->btn_save_lyrics->setText(tr("Save lyrics not supported"));
	}

	else if(m->lyrics->is_lyric_tag_available()) {
		ui->btn_save_lyrics->setText(tr("Overwrite lyrics"));
	}

	else {
		ui->btn_save_lyrics->setText(tr("Save lyrics"));
	}
}


void GUI_Lyrics::language_changed()
{
	if(!ui){
		return;
	}

	ui->lab_artist->setText(Lang::get(Lang::Artist));
	ui->lab_tit->setText(Lang::get(Lang::Title));
	ui->lab_zoom->setText(Lang::get(Lang::Zoom));
	ui->lab_source->setText(tr("Source"));
	ui->btn_close->setText(Lang::get(Lang::Close));
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));

	setup_sources();
	set_save_button_text();
}

void GUI_Lyrics::showEvent(QShowEvent* e)
{
	init();

	Widget::showEvent(e);
}

void GUI_Lyrics::wheelEvent(QWheelEvent* e)
{
	e->accept();

	if( (e->modifiers() & Qt::ShiftModifier) ||
		(e->modifiers() & Qt::ControlModifier))
	{
		int delta_zoom = 10;
		if(e->delta() < 0){
			delta_zoom = -10;
		}

		ui->sb_zoom->setValue( ui->sb_zoom->value() + delta_zoom);
	}
}
