/* GUI_AbstractStream.cpp */

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

#include "GUI_AbstractStream.h"
#include "GUI_ConfigureStreams.h"

#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/MenuTool/MenuTool.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

#include "Components/Streaming/Streams/AbstractStreamHandler.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QAbstractItemView>

using namespace Gui;

struct GUI_AbstractStream::Private
{
	QMap<QString, QString>	stations;
	QMap<QString, QString>	temporary_stations;
	ProgressBar*			loading_bar=nullptr;
	QComboBox*				combo_stream=nullptr;
	QPushButton*			btn_play=nullptr;
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
	m->btn_play->setFocusPolicy(Qt::StrongFocus);

	m->btn_tool->show_actions(static_cast<ContextMenuEntries>(ContextMenu::EntryNew));
	m->btn_tool->register_preference_action(new StreamPreferenceAction(m->btn_tool));
	m->btn_tool->register_preference_action(new StreamRecorderPreferenceAction(m->btn_tool));

	setTabOrder(m->combo_stream, m->btn_play);

	connect(m->btn_play, &QPushButton::clicked, this, &GUI_AbstractStream::listen_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_edit, this, &GUI_AbstractStream::edit_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_delete, this, &GUI_AbstractStream::delete_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_new, this, &GUI_AbstractStream::new_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_save, this, &GUI_AbstractStream::save_clicked);

	connect(m->combo_stream, combo_activated_int, this, &GUI_AbstractStream::combo_idx_changed);
	connect(m->stream_handler, &AbstractStreamHandler::sig_error, this, &GUI_AbstractStream::error);
	connect(m->stream_handler, &AbstractStreamHandler::sig_data_available, this, &GUI_AbstractStream::data_available);
	connect(m->stream_handler, &AbstractStreamHandler::sig_stopped, this, &GUI_AbstractStream::stopped);
}


void GUI_AbstractStream::init_ui()
{
	m->stream_handler = stream_handler();
	connect(m->stream_handler, &AbstractStreamHandler::sig_too_many_urls_found,
			this, &GUI_AbstractStream::too_many_urls_found);

	m->loading_bar = new ProgressBar(this);

	init_connections();
	setup_stations();
	skin_changed();

	set_searching(false);

	ListenSetting(Set::Player_Style, GUI_AbstractStream::_sl_skin_changed);
}


void GUI_AbstractStream::setup_stations()
{
	QString old_name = current_station();
	QString old_url = url();

	m->btn_play->setEnabled(false);
	m->combo_stream->clear();
	m->stream_handler->get_all_streams(m->stations);

	int idx = -1;
	int i = 0;
	for(auto it=m->stations.begin(); it != m->stations.end(); it++, i++)
	{
		m->combo_stream->addItem(it.key(), it.value());

		if(it.key() == old_name || it.value() == old_url){
			idx = i;
		}
	}

	if(idx < 0 && m->combo_stream->count() > 0)
	{
		idx = 0;
	}

	if(idx >= 0)
	{
		m->combo_stream->setCurrentIndex(idx);
		combo_idx_changed(m->combo_stream->currentIndex());
	}
}


void GUI_AbstractStream::combo_idx_changed(int idx)
{
	QString current_text = m->combo_stream->currentText();
	bool is_temporary = m->temporary_stations.contains(current_text);

	m->btn_tool->show_action(ContextMenu::EntrySave, is_temporary);
	m->btn_tool->show_action(ContextMenu::EntryEdit, (idx >= 0) && !is_temporary);
	m->btn_tool->show_action(ContextMenu::EntryDelete, (idx >= 0));

	m->combo_stream->setToolTip(url());

	bool listen_disabled = (url().size() < 8 && !m->searching);
	m->btn_play->setDisabled(listen_disabled);
}


void GUI_AbstractStream::data_available()
{
	set_searching(false);
}


void GUI_AbstractStream::listen_clicked()
{
	if(m->searching)
	{
		m->btn_play->setDisabled(true);
		m->stream_handler->stop();
		return;
	}

	QString station_name = get_title_fallback_name();
	if(m->combo_stream->currentIndex() > 0)
	{
		station_name = current_station();
	}

	if(url().size() > 5)
	{
		set_searching(true);
		play(url(), station_name);
	}

	else {
		sp_log(Log::Warning, this) << "Url is empty";
	}
}


void GUI_AbstractStream::play(QString url, QString station_name)
{
	bool success = m->stream_handler->parse_station(url, station_name);
	if(!success)
	{
		sp_log(Log::Warning, this) << "Stream Handler busy";
		set_searching(false);
	}
}


void GUI_AbstractStream::stopped()
{
	set_searching(false);
}


void GUI_AbstractStream::set_searching(bool searching)
{
	QString text = (searching == true) ? Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btn_play->setText(text);
	m->btn_play->setDisabled(false);
	m->loading_bar->setVisible(searching);
	m->searching = searching;
}


void GUI_AbstractStream::error()
{
	set_searching(false);

	sp_log(Log::Warning, this) << "Stream Handler error";
	Message::Answer answer = Message::question_yn
	(
		tr("Cannot open stream") + "\n" +
		url() + "\n\n" +
		Lang::get(Lang::Retry).question()
	);

	if(answer == Message::Answer::Yes){
		listen_clicked();
	}

	else
	{
		QString current_station = combo_stream()->currentText();

		if(m->temporary_stations.contains(current_station))
		{
			m->temporary_stations.remove(combo_stream()->currentText());
			m->combo_stream->removeItem(m->combo_stream->currentIndex());
		}
	}
}


QString GUI_AbstractStream::current_station() const
{
	return m->combo_stream->currentText();
}


QString GUI_AbstractStream::url() const
{
	return m->combo_stream->currentData().toString();
}


void GUI_AbstractStream::add_stream(const QString& name, const QString& url, bool keep_old)
{
	if(!keep_old)
	{
		for(QString key : m->temporary_stations.keys())
		{
			int idx = m->combo_stream->findText(key);
			if(idx >= 0){
				m->combo_stream->removeItem(idx);
			}
		}

		m->temporary_stations.clear();
	}

	m->temporary_stations[name] = url;

	m->combo_stream->addItem(name, url);
	m->combo_stream->setCurrentText(name);
	combo_idx_changed(m->combo_stream->currentIndex());
}


void GUI_AbstractStream::new_clicked()
{
	GUI_ConfigureStreams* cs = new GUI_ConfigureStreams(this->get_display_name(), GUI_ConfigureStreams::New, this);
	connect(cs, &Gui::Dialog::finished, this, &GUI_AbstractStream::new_finished);

	cs->open();
}


void GUI_AbstractStream::save_clicked()
{
	GUI_ConfigureStreams* cs = new GUI_ConfigureStreams(this->get_display_name(), GUI_ConfigureStreams::New, this);
	connect(cs, &Gui::Dialog::finished, this, &GUI_AbstractStream::new_finished);

	cs->set_name(current_station());
	cs->set_url(url());

	cs->open();
}

void GUI_AbstractStream::new_finished()
{
	GUI_ConfigureStreams* cs = static_cast<GUI_ConfigureStreams*>(sender());

	bool b = cs->was_accepted();
	if(!b){
		cs->deleteLater();
		return;
	}


	QString name = cs->name();
	bool exists = Util::contains(m->stations.keys(), [&name](QString station){
		return (station == name);
	});

	if(exists)
	{
		sp_log(Log::Warning, this) << "Stream already there";
		cs->set_error_message(tr("Please choose another name"));
		cs->open();

		return;
	}

	m->stream_handler->add_stream(cs->name(),  cs->url());
	cs->deleteLater();

	setup_stations();
}


void GUI_AbstractStream::edit_clicked()
{
	GUI_ConfigureStreams* cs = new GUI_ConfigureStreams(this->get_display_name(), GUI_ConfigureStreams::Edit, this);
	connect(cs, &Gui::Dialog::finished, this, &GUI_AbstractStream::edit_finished);

	cs->set_name(current_station());
	cs->set_url(url());

	cs->open();
}


void GUI_AbstractStream::edit_finished()
{
	GUI_ConfigureStreams* cs = static_cast<GUI_ConfigureStreams*>(sender());

	QString new_name = cs->name();
	QString new_url	= cs->url();

	int b = cs->was_accepted();
	if(!b){
		cs->deleteLater();
		return;
	}

	QString old_name = current_station();
	QString old_url = url();

	if(old_url != new_url){
		m->stream_handler->update_url(old_name, new_url);
	}

	if(old_name != new_name){
		m->stream_handler->rename(old_name, new_name);
	}

	cs->deleteLater();
	setup_stations();
}


void GUI_AbstractStream::delete_clicked()
{
	QString cur_station = current_station();
	if(cur_station.isEmpty()) {
		return;
	}

	Message::Answer ret = Message::question_yn(tr("Do you really want to delete %1").arg(cur_station) + "?");

	if(ret == Message::Answer::Yes)
	{
		bool success = m->stream_handler->delete_stream(cur_station);
		if(success){
			sp_log(Log::Info, this) << cur_station << "successfully deleted";
		}

		else {
			sp_log(Log::Warning, this) << "Cannot delete " << cur_station;
		}
	}

	setup_stations();
}


void GUI_AbstractStream::too_many_urls_found(int n_urls, int n_max_urls)
{
	Message::error(QString("Found %1 urls").arg(n_urls) + "<br />" +
				   QString("Maximum number is %1").arg(n_max_urls)
	);

	set_searching(false);
}


bool GUI_AbstractStream::has_loading_bar() const
{
	return true;
}


void GUI_AbstractStream::assign_ui_vars()
{
	m->combo_stream=combo_stream();
	m->btn_play=btn_play();
	m->btn_tool = btn_menu();
}


void GUI_AbstractStream::retranslate_ui()
{
	QString text = (m->searching) ? Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btn_play->setText(text);
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



StreamPreferenceAction::StreamPreferenceAction(QWidget* parent) :
	PreferenceAction(QString(Lang::get(Lang::Streams) + " && " +Lang::get(Lang::Podcasts)), identifier(), parent) {}

StreamPreferenceAction::~StreamPreferenceAction() {}

QString StreamPreferenceAction::identifier() const { return "streams"; }

QString StreamPreferenceAction::display_name() const { return Lang::get(Lang::Streams) + " && " + Lang::get(Lang::Podcasts); }

