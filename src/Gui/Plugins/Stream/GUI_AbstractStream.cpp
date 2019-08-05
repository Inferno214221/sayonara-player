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

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

#include "Components/Streaming/Streams/AbstractStreamHandler.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QAbstractItemView>

using namespace Gui;
namespace Algorithm=Util::Algorithm;

using StreamMap = QMap<QString, QString>;
struct AbstractStream::Private
{
	StreamMap				stations;
	StreamMap				temporary_stations;
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

AbstractStream::AbstractStream(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}

AbstractStream::~AbstractStream() {}

void AbstractStream::init_connections()
{
	m->btn_play->setFocusPolicy(Qt::StrongFocus);

	m->btn_tool->show_actions(static_cast<ContextMenuEntries>(ContextMenu::EntryNew));
	m->btn_tool->register_preference_action(new StreamPreferenceAction(m->btn_tool));
	m->btn_tool->register_preference_action(new StreamRecorderPreferenceAction(m->btn_tool));

	setTabOrder(m->combo_stream, m->btn_play);

	connect(m->btn_play, &QPushButton::clicked, this, &AbstractStream::listen_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_edit, this, &AbstractStream::edit_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_delete, this, &AbstractStream::delete_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_new, this, &AbstractStream::new_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_save, this, &AbstractStream::save_clicked);

	connect(m->combo_stream, combo_activated_int, this, &AbstractStream::combo_idx_changed);
	connect(m->stream_handler, &AbstractStreamHandler::sig_error, this, &AbstractStream::error);
	connect(m->stream_handler, &AbstractStreamHandler::sig_data_available, this, &AbstractStream::data_available);
	connect(m->stream_handler, &AbstractStreamHandler::sig_stopped, this, &AbstractStream::stopped);
}


void AbstractStream::init_ui()
{
	m->stream_handler = stream_handler();
	connect(m->stream_handler, &AbstractStreamHandler::sig_too_many_urls_found,
			this, &AbstractStream::too_many_urls_found);

	m->loading_bar = new ProgressBar(this);

	init_connections();
	setup_stations();
	skin_changed();

	set_searching(false);

	ListenSetting(Set::Player_Style, AbstractStream::_sl_skin_changed);
}


void AbstractStream::setup_stations()
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


void AbstractStream::combo_idx_changed(int idx)
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


void AbstractStream::data_available()
{
	set_searching(false);
}


void AbstractStream::listen_clicked()
{
	if(m->searching)
	{
		m->btn_play->setDisabled(true);
		m->stream_handler->stop();
		return;
	}

	QString station_name = get_title_fallback_name();
	if(m->combo_stream->currentIndex() >= 0)
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


void AbstractStream::play(QString url, QString station_name)
{
	bool success = m->stream_handler->parse_station(url, station_name);
	if(!success)
	{
		sp_log(Log::Warning, this) << "Stream Handler busy";
		set_searching(false);
	}
}


void AbstractStream::stopped()
{
	set_searching(false);
}


void AbstractStream::set_searching(bool searching)
{
	QString text = (searching == true) ? Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btn_play->setText(text);
	m->btn_play->setDisabled(false);
	m->loading_bar->setVisible(searching);
	m->searching = searching;
}


void AbstractStream::error()
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


QString AbstractStream::current_station() const
{
	return m->combo_stream->currentText();
}


QString AbstractStream::url() const
{
	return m->combo_stream->currentData().toString();
}


void AbstractStream::add_stream(const QString& name, const QString& url)
{
	m->temporary_stations[name] = url;

	m->combo_stream->addItem(name, url);
	m->combo_stream->setCurrentText(name);
	combo_idx_changed(m->combo_stream->currentIndex());
}


void AbstractStream::new_clicked()
{
	GUI_ConfigureStreams* cs = new GUI_ConfigureStreams(this->get_display_name(), GUI_ConfigureStreams::New, this);
	connect(cs, &Gui::Dialog::finished, this, &AbstractStream::new_finished);

	cs->open();
}


void AbstractStream::save_clicked()
{
	GUI_ConfigureStreams* cs = new GUI_ConfigureStreams(this->get_display_name(), GUI_ConfigureStreams::New, this);
	connect(cs, &Gui::Dialog::finished, this, &AbstractStream::new_finished);

	cs->set_name(current_station());
	cs->set_url(url());

	cs->open();
}

void AbstractStream::new_finished()
{
	GUI_ConfigureStreams* cs = static_cast<GUI_ConfigureStreams*>(sender());

	bool b = cs->was_accepted();
	if(!b){
		cs->deleteLater();
		return;
	}


	QString name = cs->name();
	bool exists = Algorithm::contains(m->stations.keys(), [&name](QString station){
		return (station == name);
	});

	if(exists)
	{
		sp_log(Log::Warning, this) << "Stream already there";
		cs->set_error_message(tr("Please choose another name"));
		cs->open();

		return;
	}

	bool success = m->stream_handler->add_stream(name,  cs->url());
	if(success)
	{
		m->temporary_stations.remove(name);
	}

	cs->deleteLater();

	setup_stations();
}


void AbstractStream::edit_clicked()
{
	GUI_ConfigureStreams* cs = new GUI_ConfigureStreams(this->get_display_name(), GUI_ConfigureStreams::Edit, this);
	connect(cs, &Gui::Dialog::finished, this, &AbstractStream::edit_finished);

	cs->set_name(current_station());
	cs->set_url(url());

	cs->open();
}


void AbstractStream::edit_finished()
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


void AbstractStream::delete_clicked()
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


void AbstractStream::too_many_urls_found(int n_urls, int n_max_urls)
{
	Message::error(QString("Found %1 urls").arg(n_urls) + "<br />" +
				   QString("Maximum number is %1").arg(n_max_urls)
	);

	set_searching(false);
}


bool AbstractStream::has_loading_bar() const
{
	return true;
}


void AbstractStream::assign_ui_vars()
{
	m->combo_stream=combo_stream();
	m->btn_play=btn_play();
	m->btn_tool = btn_menu();
}


void AbstractStream::retranslate_ui()
{
	QString text = (m->searching) ? Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btn_play->setText(text);
}


void AbstractStream::_sl_skin_changed()
{
	if(!is_ui_initialized()){
		return;
	}

	set_searching(m->searching);
}



StreamPreferenceAction::StreamPreferenceAction(QWidget* parent) :
	PreferenceAction(QString(Lang::get(Lang::Streams) + " && " +Lang::get(Lang::Podcasts)), identifier(), parent) {}

StreamPreferenceAction::~StreamPreferenceAction() {}

QString StreamPreferenceAction::identifier() const { return "streams"; }

QString StreamPreferenceAction::display_name() const { return Lang::get(Lang::Streams) + " && " + Lang::get(Lang::Podcasts); }

