/* GUI_AbstractStream.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "AbstractStationPlugin.h"
#include "GUI_ConfigureStation.h"

#include "Components/Streaming/Streams/AbstractStationHandler.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/MenuTool/MenuToolButton.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Streams/Station.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QAbstractItemView>
#include <QVBoxLayout>

namespace Algorithm=Util::Algorithm;

struct Gui::AbstractStationPlugin::Private
{
	QList<StationPtr>		stations;
	QList<StationPtr>		temporary_stations;
	ProgressBar*			loading_bar=nullptr;
	QComboBox*				combo_stream=nullptr;
	QPushButton*			btn_play=nullptr;
	MenuToolButton*			btn_tool=nullptr;
	AbstractStationHandler*	stream_handler=nullptr;

	bool					searching;

	Private() :
		searching(false)
	{}
};

Gui::AbstractStationPlugin::AbstractStationPlugin(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}

Gui::AbstractStationPlugin::~AbstractStationPlugin() = default;

void Gui::AbstractStationPlugin::init_connections()
{
	m->btn_play->setFocusPolicy(Qt::StrongFocus);

	m->btn_tool->show_actions(ContextMenuEntries(ContextMenu::EntryNew));
	m->btn_tool->register_preference_action(new StreamPreferenceAction(m->btn_tool));
	m->btn_tool->register_preference_action(new StreamRecorderPreferenceAction(m->btn_tool));

	setTabOrder(m->combo_stream, m->btn_play);

	connect(m->btn_play, &QPushButton::clicked, this, &AbstractStationPlugin::listen_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_edit, this, &AbstractStationPlugin::edit_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_delete, this, &AbstractStationPlugin::delete_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_new, this, &AbstractStationPlugin::new_clicked);
	connect(m->btn_tool, &MenuToolButton::sig_save, this, &AbstractStationPlugin::save_clicked);

	connect(m->combo_stream, combo_activated_int, this, &AbstractStationPlugin::current_index_changed);
	connect(m->stream_handler, &AbstractStationHandler::sig_error, this, &AbstractStationPlugin::error);
	connect(m->stream_handler, &AbstractStationHandler::sig_data_available, this, &AbstractStationPlugin::data_available);
	connect(m->stream_handler, &AbstractStationHandler::sig_stopped, this, &AbstractStationPlugin::stopped);
}


void Gui::AbstractStationPlugin::init_ui()
{
	m->stream_handler = stream_handler();
	connect(m->stream_handler, &AbstractStationHandler::sig_too_many_urls_found,
			this, &AbstractStationPlugin::too_many_urls_found);

	m->loading_bar = new ProgressBar(this);

	init_connections();
	setup_stations();
	skin_changed();

	set_searching(false);

	ListenSetting(Set::Player_Style, AbstractStationPlugin::_sl_skin_changed);
}


void Gui::AbstractStationPlugin::setup_stations()
{
	QString current_name = m->combo_stream->currentText();
	QList<StationPtr> stations;

	m->btn_play->setEnabled(false);
	m->combo_stream->clear();
	m->stream_handler->get_all_streams(stations);

	for(StationPtr station : stations)
	{
		m->combo_stream->addItem(station->name(), station->url());
	}

	int idx = Util::Algorithm::indexOf(stations, [&current_name](StationPtr station) {
		return(current_name == station->name() || current_name == station->url());
	});

	if(m->combo_stream->count() > 0)
	{
		idx = std::max(0, idx);
	}

	if(idx >= 0)
	{
		m->combo_stream->setCurrentIndex(idx);
		current_index_changed(m->combo_stream->currentIndex());
	}
}


void Gui::AbstractStationPlugin::current_index_changed(int idx)
{
	QString current_text = m->combo_stream->currentText();
	bool is_temporary = Util::Algorithm::contains(m->temporary_stations, [&current_text](StationPtr station){
		return (station->name() == current_text);
	});

	QString url = current_url();

	m->btn_tool->show_action(ContextMenu::EntrySave, is_temporary);
	m->btn_tool->show_action(ContextMenu::EntryEdit, (idx >= 0) && !is_temporary);
	m->btn_tool->show_action(ContextMenu::EntryDelete, (idx >= 0));

	m->combo_stream->setToolTip(url);

	bool listen_disabled = (url.size() < 8 && !m->searching);
	m->btn_play->setDisabled(listen_disabled);
}


void Gui::AbstractStationPlugin::data_available()
{
	set_searching(false);
}

void Gui::AbstractStationPlugin::listen_clicked()
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
		station_name = current_name();
	}

	QString url = current_url();
	if(url.size() > 5)
	{
		set_searching(true);
		play(url, station_name);
	}

	else {
		sp_log(Log::Warning, this) << "Url is empty";
	}
}


void Gui::AbstractStationPlugin::play(QString url, QString station_name)
{
	bool success = m->stream_handler->parse_station(url, station_name);
	if(!success)
	{
		sp_log(Log::Warning, this) << "Stream Handler busy";
		set_searching(false);
	}
}


void Gui::AbstractStationPlugin::stopped()
{
	set_searching(false);
}


void Gui::AbstractStationPlugin::set_searching(bool searching)
{
	QString text = (searching == true) ? Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btn_play->setText(text);
	m->btn_play->setDisabled(false);
	m->loading_bar->setVisible(searching);
	m->searching = searching;
}


void Gui::AbstractStationPlugin::error()
{
	set_searching(false);

	sp_log(Log::Warning, this) << "Stream Handler error";
	Message::Answer answer = Message::question_yn
	(
		tr("Cannot open stream") + "\n" +
		current_url() + "\n\n" +
		Lang::get(Lang::Retry).question()
	);

	if(answer == Message::Answer::Yes){
		listen_clicked();
	}

	else
	{
		QString current_name = combo_stream()->currentText();
		auto it = Util::Algorithm::find(m->temporary_stations, [&current_name](StationPtr station){
			return (station->name() == current_name);
		});

		if(it != m->temporary_stations.end())
		{
			m->temporary_stations.erase(it);
			m->combo_stream->removeItem(m->combo_stream->currentIndex());
		}
	}
}

QString Gui::AbstractStationPlugin::current_name() const
{
	return m->combo_stream->currentText();
}

QString Gui::AbstractStationPlugin::current_url() const
{
	return m->combo_stream->currentData().toString();
}

void Gui::AbstractStationPlugin::add_stream(const QString& name, const QString& url)
{
	m->temporary_stations << m->stream_handler->create_stream(name, url);
	m->combo_stream->addItem(name, url);
	m->combo_stream->setCurrentText(name);
	current_index_changed(m->combo_stream->currentIndex());
}

void Gui::AbstractStationPlugin::new_clicked()
{
	GUI_ConfigureStation* cs = create_config_dialog();
	connect(cs, &Gui::Dialog::finished, this, &AbstractStationPlugin::new_finished);

	cs->set_mode(get_title_fallback_name(), GUI_ConfigureStation::Mode::New);
	cs->init_ui(nullptr);

	cs->open();
}

void Gui::AbstractStationPlugin::save_clicked()
{
	GUI_ConfigureStation* cs = create_config_dialog();
	connect(cs, &Gui::Dialog::finished, this, &AbstractStationPlugin::new_finished);

	StationPtr station = m->stream_handler->create_stream(current_name(), current_url());

	cs->set_mode(current_name(), GUI_ConfigureStation::Mode::Edit);
	cs->init_ui(station);
	cs->open();
}

void Gui::AbstractStationPlugin::new_finished()
{
	auto* cs = static_cast<GUI_ConfigureStation*>(sender());

	bool b = cs->was_accepted();
	if(!b){
		cs->deleteLater();
		return;
	}

	StationPtr station = cs->configured_station();

	int index = m->combo_stream->findText(station->name());
	if(index >= 0)
	{
		sp_log(Log::Warning, this) << "Stream already there";
		cs->set_error_message(tr("Please choose another name"));
		cs->open();

		return;
	}

	bool success = m->stream_handler->add_stream(station);
	if(success)
	{
		int index = Util::Algorithm::indexOf(m->temporary_stations, [station](StationPtr s){
			return (s->name() == station->name());
		});

		if(index >= 0) {
			m->temporary_stations.removeAt(index);
		}
	}

	cs->deleteLater();

	setup_stations();
}

void Gui::AbstractStationPlugin::edit_clicked()
{
	StationPtr station = m->stream_handler->station(current_name());
	if(!station)
	{
		return;
	}

	GUI_ConfigureStation* cs = create_config_dialog();
	connect(cs, &Gui::Dialog::finished, this, &AbstractStationPlugin::edit_finished);

	cs->set_mode(station->name(), GUI_ConfigureStation::Mode::Edit);
	cs->init_ui(station);

	cs->open();
}

void Gui::AbstractStationPlugin::edit_finished()
{
	auto* cs = static_cast<GUI_ConfigureStation*>(sender());

	int b = cs->was_accepted();
	if(!b){
		cs->deleteLater();
		return;
	}

	StationPtr station = cs->configured_station();

	if(current_url() != station->url()) {
		m->stream_handler->update_url(current_name(), station->url());
	}

	if(current_name() != station->name()) {
		m->stream_handler->rename(current_name(), station->name());
	}

	cs->deleteLater();
	setup_stations();
}

void Gui::AbstractStationPlugin::delete_clicked()
{
	Message::Answer ret = Message::question_yn(tr("Do you really want to delete %1").arg(current_name()) + "?");

	if(ret == Message::Answer::Yes)
	{
		bool success = m->stream_handler->delete_stream(current_name());
		if(success){
			sp_log(Log::Info, this) << current_name() << "successfully deleted";
		}

		else {
			sp_log(Log::Warning, this) << "Cannot delete " << current_name();
		}
	}

	setup_stations();
}

void Gui::AbstractStationPlugin::too_many_urls_found(int n_urls, int n_max_urls)
{
	Message::error(QString("Found %1 urls").arg(n_urls) + "<br />" +
				   QString("Maximum number is %1").arg(n_max_urls)
	);

	set_searching(false);
}

bool Gui::AbstractStationPlugin::has_loading_bar() const
{
	return true;
}

void Gui::AbstractStationPlugin::assign_ui_vars()
{
	m->combo_stream=combo_stream();
	m->btn_play=btn_play();
	m->btn_tool = btn_menu();
}

void Gui::AbstractStationPlugin::retranslate_ui()
{
	QString text = (m->searching) ? Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btn_play->setText(text);
}

void Gui::AbstractStationPlugin::_sl_skin_changed()
{
	if(!is_ui_initialized()){
		return;
	}

	set_searching(m->searching);
	btn_play()->setIcon(Gui::Icons::icon(Gui::Icons::Play));
}


Gui::StreamPreferenceAction::StreamPreferenceAction(QWidget* parent) :
	PreferenceAction(QString(Lang::get(Lang::Streams) + " && " +Lang::get(Lang::Podcasts)), identifier(), parent) {}

Gui::StreamPreferenceAction::~StreamPreferenceAction() {}

QString Gui::StreamPreferenceAction::identifier() const { return "streams"; }

QString Gui::StreamPreferenceAction::display_name() const { return Lang::get(Lang::Streams) + " && " + Lang::get(Lang::Podcasts); }

