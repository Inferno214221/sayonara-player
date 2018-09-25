/* GUI_AbstractStream.cpp */

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

#include "GUI_AbstractStream.h"

#include "GUI/Utils/Widgets/ProgressBar.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/MenuTool/MenuTool.h"
#include "GUI/Utils/Style.h"
#include "GUI/Utils/InputDialog/LineInputDialog.h"

#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Parser/PodcastParser.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

#include "Components/StreamPlugins/Streams/AbstractStreamHandler.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QAbstractItemView>

using namespace Gui;

struct GUI_AbstractStream::Private
{
	QMap<QString, QString>	stations;
	ProgressBar*			loading_bar=nullptr;
	QComboBox*				combo_stream=nullptr;
	QPushButton*			btn_play=nullptr;
	QLineEdit*				le_url=nullptr;
	QLabel*					lab_listen=nullptr;
	MenuToolButton*			btn_tool=nullptr;
	AbstractStreamHandler*	stream_handler=nullptr;
	bool					searching;

	Private() :
		searching(false)
	{}
};

GUI_AbstractStream::GUI_AbstractStream(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}

GUI_AbstractStream::~GUI_AbstractStream() {}


void GUI_AbstractStream::init_connections()
{
	m->combo_stream->setAutoCompletion(false);
	m->combo_stream->setFocusPolicy(Qt::StrongFocus);

	m->le_url->setFocusPolicy(Qt::StrongFocus);
	m->btn_play->setFocusPolicy(Qt::StrongFocus);

	setTabOrder(m->combo_stream, m->le_url);
	setTabOrder(m->le_url, m->btn_play);

	m->btn_tool->show_actions((ContextMenuEntries) (ContextMenu::EntryNew));
	m->btn_tool->register_preference_action(new StreamPreferenceAction(m->btn_tool));
	m->btn_tool->register_preference_action(new StreamRecorderPreferenceAction(m->btn_tool));

	connect(m->btn_play, &QPushButton::clicked, this, &GUI_AbstractStream::listen_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_rename, this, &GUI_AbstractStream::rename_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_save, this, &GUI_AbstractStream::save_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_delete, this, &GUI_AbstractStream::delete_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_new, this, &GUI_AbstractStream::new_clicked);

	connect(m->combo_stream, combo_activated_int, this, &GUI_AbstractStream::combo_idx_changed);
	connect(m->combo_stream, &QComboBox::editTextChanged, this, &GUI_AbstractStream::text_changed);

	connect(m->le_url, &QLineEdit::textChanged, this, &GUI_AbstractStream::text_changed);

	connect(m->stream_handler, &AbstractStreamHandler::sig_error, this, &GUI_AbstractStream::error);
	connect(m->stream_handler, &AbstractStreamHandler::sig_data_available, this, &GUI_AbstractStream::data_available);
	connect(m->stream_handler, &AbstractStreamHandler::sig_stopped, this, &GUI_AbstractStream::stopped);
}

void GUI_AbstractStream::init_ui()
{
	m->stream_handler = stream_handler();
	connect(m->stream_handler, &AbstractStreamHandler::sig_too_many_urls_found,
			this, &GUI_AbstractStream::too_many_urls_found);

	m->btn_play->setMinimumSize(QSize(24,24));
	m->btn_play->setMaximumSize(QSize(24,24));
	m->btn_play->setFlat(true);
	m->btn_play->setIconSize(QSize(18, 18));

	m->loading_bar = new ProgressBar(this);

	init_connections();
	setup_stations();
	skin_changed();

	set_searching(false);

	Set::listen<Set::Player_Style>(this, &GUI_AbstractStream::_sl_skin_changed);
}

void GUI_AbstractStream::retranslate_ui()
{
	m->le_url->setPlaceholderText(Lang::get(Lang::EnterUrl).triplePt());

	QLineEdit* le = m->combo_stream->lineEdit();
	le->setPlaceholderText(Lang::get(Lang::EnterName).triplePt());

	if(m->searching){
		m->lab_listen->setText(Lang::get(Lang::Stop));
	} else {
		m->lab_listen->setText(Lang::get(Lang::Listen));
	}
}

void GUI_AbstractStream::error()
{
	set_searching(false);

	sp_log(Log::Warning, this) << "Stream Handler error";
	Message::Answer answer = Message::question_yn
	(
		tr("Cannot open stream") + "\n" +
		m->le_url->text() + "\n\n" +
		Lang::get(Lang::Retry).question()
	);

	if(answer == Message::Answer::Yes){
		listen_clicked();
	}
}

void GUI_AbstractStream::data_available()
{
	set_searching(false);
}

void GUI_AbstractStream::stopped()
{
	set_searching(false);
}

void GUI_AbstractStream::set_searching(bool searching)
{
	bool is_dark = (_settings->get<Set::Player_Style>() == 1);

	m->loading_bar->setVisible(searching);
	m->btn_play->setDisabled(false);

	Icons::IconMode mode = Icons::IconMode::ForceStdIcon;
	if(is_dark){
		mode = Icons::IconMode::ForceSayonaraIcon;
	}

	if(!searching) {
		m->btn_play->setIcon( Icons::icon(Icons::Play, mode));
		m->lab_listen->setText(Lang::get(Lang::Listen));
	} else {
		m->btn_play->setIcon( Icons::icon(Icons::Stop, mode));
		m->lab_listen->setText(Lang::get(Lang::Stop));
	}

	m->searching = searching;
}

void GUI_AbstractStream::_sl_skin_changed()
{
	if(!is_ui_initialized()){
		return;
	}

	QAbstractItemView* view = m->combo_stream->view();

	view->parentWidget()->setStyleSheet("margin: 0px; padding: -4px -1px; border: 1px solid #282828; background: none;");
	view->setStyleSheet(Style::current_style());
	view->setMinimumHeight(20 * view->model()->rowCount());

	set_searching(m->searching);
}


void GUI_AbstractStream::play(QString url, QString station_name)
{
	bool success = m->stream_handler->parse_station(url, station_name);
	if(!success){
		sp_log(Log::Warning, this) << "Stream Handler busy";
		set_searching(false);
	}
}


bool GUI_AbstractStream::has_loading_bar() const
{
	return true;
}

void GUI_AbstractStream::listen_clicked()
{
	QString name, url;

	if(m->searching){
		m->btn_play->setDisabled(true);
		m->stream_handler->stop();
		return;
	}

	if( m->combo_stream->currentIndex() <= 0) {
		url = m->le_url->text();
		name = get_title_fallback_name();
	}

	else{
		url = m->le_url->text();
		name = m->combo_stream->currentText();
	}

	url = url.trimmed();
	if(url.size() > 5) {
		set_searching(true);
		play(url, name);
	}
}

void GUI_AbstractStream::combo_idx_changed(int idx)
{
	QString cur_station_name = m->combo_stream->currentText();
	QString address = m->stations[cur_station_name];

	if(address.size() > 0) {
		m->le_url->setText(address);
	}

	if(idx == 0) {
		m->le_url->clear();
	}

	m->btn_tool->show_action(ContextMenu::EntryDelete, idx > 0);

	m->combo_stream->setToolTip(address);

	QString text = m->le_url->text();
	bool listen_disabled = (text.size() < 8 && !m->searching);
	m->btn_play->setDisabled(listen_disabled);
}


void GUI_AbstractStream::text_changed(const QString& str)
{
	Q_UNUSED(str)

	bool has_url = (!m->le_url->text().isEmpty());
	bool has_text = (!m->combo_stream->currentText().isEmpty());

	m->btn_play->setEnabled(has_url);
	m->lab_listen->setEnabled(has_url);

	m->btn_tool->show_action(ContextMenu::EntrySave, has_url && has_text);
	m->btn_tool->show_action(ContextMenu::EntryRename, has_url && has_text);
}

void GUI_AbstractStream::too_many_urls_found(int n_urls, int n_max_urls)
{
	Message::error(QString("Found %1 urls").arg(n_urls) + "<br />" +
				   QString("Maximum number is %1").arg(n_max_urls)
	);

	set_searching(false);
}

void GUI_AbstractStream::new_clicked()
{
	if(m->combo_stream->count() > 0){
		m->combo_stream->setCurrentIndex(0);
		m->combo_stream->setItemText(0, "");
	}

	else{
		m->combo_stream->addItem("");
		m->combo_stream->setCurrentIndex(0);
	}

	m->le_url->clear();
}

void GUI_AbstractStream::delete_clicked()
{
	if(m->combo_stream->currentIndex() <= 0) return;

	QString cur_station_name = m->combo_stream->currentText();

	Message::Answer ret = Message::question_yn(tr("Do you really want to delete %1").arg(cur_station_name));

	if(ret == Message::Answer::Yes)
	{
		bool success = m->stream_handler->delete_stream(cur_station_name);
		if(success){
			sp_log(Log::Info, this) << cur_station_name << "successfully deleted";
		}

		else {
			sp_log(Log::Warning, this) << "Cannot delete " << cur_station_name;
		}
	}

	setup_stations();
}

void GUI_AbstractStream::save_clicked()
{
	QString name = m->combo_stream->currentText();
	QString url = m->le_url->text();
	Message::Answer answer;

	if(name.isEmpty() || url.isEmpty()){
		return;
	}

	bool overwritten = false;
	for(int i=0; i<m->combo_stream->count(); i++)
	{
		QString text = m->combo_stream->itemText(i);
		if(text == name)
		{
			answer = Message::question_yn(tr("Overwrite?") + "\n" + name + "\n" + url);
			if(answer == Message::Answer::Yes)
			{
				m->stream_handler->update_url(name, url);
				overwritten = true;
			}

			break;
		}
	}

	if(!overwritten)
	{
		m->stream_handler->save(name, url);
	}

	setup_stations();
}

void GUI_AbstractStream::rename_clicked()
{
	QString old_name = m->combo_stream->currentText();
	if(old_name.isEmpty() || old_name.isEmpty()){
		return;
	}

	LineInputDialog* dialog = new LineInputDialog(Lang::get(Lang::Rename), Lang::get(Lang::Rename), old_name, this);
	if(dialog->exec() == QDialog::Rejected){
		return;
	}

	QString new_name = dialog->textValue();
	if(new_name.isEmpty()){
		return;
	}

	m->stream_handler->rename(old_name, new_name);

	setup_stations();
}

void GUI_AbstractStream::setup_stations()
{
	QString old_name = m->combo_stream->currentText();
	QString old_url = m->le_url->text();

	StreamMap stations;
	m->stream_handler->get_all_streams(stations);

	m->combo_stream->clear();
	m->le_url->clear();
	m->btn_play->setEnabled(false);
	m->lab_listen->setEnabled(false);

	m->stations = stations;
	m->stations[""] = "";

	if(m->stations.size() == 1){
		m->combo_stream->setCurrentIndex(0);
	}

	int idx = 0;
	for(auto it = m->stations.begin(); it != m->stations.end(); it++)
	{
		QString name = it.key();
		QString url = it.value();

		m->combo_stream->addItem(name, url);

		if(name == old_name)
		{
			idx = std::distance(m->stations.begin(), it);
		}

		else if(url == old_url && idx == 0)
		{
			idx = std::distance(m->stations.begin(), it);
		}
	}

	m->btn_tool->show_actions((ContextMenuEntries) (ContextMenu::EntryNew));

	if(idx > 0)
	{
		m->le_url->setText(m->combo_stream->itemData(idx).toString());
		m->combo_stream->setCurrentIndex(idx);
	}
}

void GUI_AbstractStream::assign_ui_vars()
{
	m->combo_stream=combo_stream();
	m->btn_play=btn_play();
	m->le_url = le_url();
	m->lab_listen = lab_listen();
	m->btn_tool = btn_menu();
}

StreamPreferenceAction::StreamPreferenceAction(QWidget* parent) :
	PreferenceAction(QString(Lang::get(Lang::Streams) + " && " +Lang::get(Lang::Podcasts)), identifier(), parent) {}

StreamPreferenceAction::~StreamPreferenceAction() {}

QString StreamPreferenceAction::identifier() const { return "streams"; }

QString StreamPreferenceAction::display_name() const { return Lang::get(Lang::Streams) + " && " + Lang::get(Lang::Podcasts); }
