/* GUI_Broadcast.cpp */

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

#include "GUI_Broadcast.h"

#include "Components/Broadcasting/StreamServer.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/PreferenceAction.h"

#include "Gui/Plugins/ui_GUI_Broadcast.h"

class BroadcastAction :
		public PreferenceAction
{
public:
	BroadcastAction(QWidget* parent) : PreferenceAction(Lang::get(Lang::Broadcast), identifier(), parent) {}

	QString identifier() const override { return "broadcast"; }

protected:
	QString display_name() const override { return Lang::get(Lang::Broadcast); }
};

struct GUI_Broadcast::Private
{
	StreamServer*   server=nullptr;
	QAction*        action_dismiss=nullptr;
	QAction*        action_dismiss_all=nullptr;
};

GUI_Broadcast::GUI_Broadcast(QWidget *parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<GUI_Broadcast::Private>();

	ListenSetting(Set::Broadcast_Active, GUI_Broadcast::start_server);
}


GUI_Broadcast::~GUI_Broadcast()
{
	m->server->deleteLater();

	if(ui)
	{
		delete ui; ui = nullptr;
	}
}


QString GUI_Broadcast::get_name() const
{
	return "Broadcast";
}


QString GUI_Broadcast::get_display_name() const
{
	return Lang::get(Lang::Broadcast);
}


void GUI_Broadcast::retranslate_ui()
{
	ui->retranslateUi(this);
	set_status_label();
	ui->btn_retry->setText(Lang::get(Lang::Retry));

	if(m->action_dismiss)
	{
		m->action_dismiss->setText(tr("Dismiss"));
		m->action_dismiss_all->setText(tr("Dismiss all"));
	}
}


void GUI_Broadcast::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);

	m->action_dismiss = new QAction(ui->btn_menu);
	m->action_dismiss_all = new QAction(ui->btn_menu);

	update_dismiss_buttons();

	ui->btn_menu->register_action(m->action_dismiss);
	ui->btn_menu->register_action(m->action_dismiss_all);
	ui->btn_menu->register_action(new BroadcastAction(ui->btn_menu));

	connect(m->action_dismiss, &QAction::triggered, this, &GUI_Broadcast::dismiss_clicked);
	connect(m->action_dismiss_all, &QAction::triggered, this, &GUI_Broadcast::dismiss_all_clicked);
	connect(ui->combo_clients, combo_current_index_changed_int, this, &GUI_Broadcast::combo_changed);
	connect(ui->btn_retry, &QPushButton::clicked, this, &GUI_Broadcast::retry);

	set_status_label();
	retranslate_ui();

	ListenSetting(SetNoDB::MP3enc_found, GUI_Broadcast::mp3_enc_found);
}


void GUI_Broadcast::set_status_label()
{
	if(!is_ui_initialized()){
		return;
	}

	int n_listeners = ui->combo_clients->count();
	QString str_listeners;

	if(n_listeners == 1){
		str_listeners = tr("%1 listener").arg(n_listeners);
	}

	else{
		str_listeners = tr("%1 listeners").arg(n_listeners);
	}

	ui->lab_status->setText(str_listeners);
}


// finally connection is established
void GUI_Broadcast::connection_established(const QString& ip)
{
	if(!is_ui_initialized()){
		return;
	}

	ui->combo_clients->addItem(ip);
	set_status_label();
	ui->combo_clients->setCurrentIndex(ui->combo_clients->count() -1);
}


void GUI_Broadcast::connection_closed(const QString& ip)
{
	if(!is_ui_initialized()){
		return;
	}

	if(ip.isEmpty()) {
		return;
	}

	sp_log(Log::Info, this) << "Connection closed: " << ip;

	int idx;
	for(idx=0; idx<ui->combo_clients->count(); idx++){
		if(ui->combo_clients->itemText(idx).contains(ip)){
			break;
		}
	}

	if(idx >= ui->combo_clients->count()){
		return;
	}

	ui->combo_clients->removeItem(idx);

	update_dismiss_buttons();

	set_status_label();
}

void GUI_Broadcast::can_listen_changed(bool success)
{
	if(!is_ui_initialized()){
		return;
	}

	ui->lab_status->setVisible(success);
	ui->lab_error->setVisible(!success);
	ui->btn_retry->setVisible(!success);

	if(!success){
		QString msg = tr("Cannot broadcast on port %1").arg(GetSetting(Set::Broadcast_Port));
		msg += "\n" + tr("Maybe another application is using this port?");

		Message::warning(msg);
	}
}


void GUI_Broadcast::retry()
{
	m->server->restart();
}


void GUI_Broadcast::dismiss_at(int idx)
{
	if(!is_ui_initialized()){
		return;
	}

	if(idx < 0 || idx >= ui->combo_clients->count()){
		return;
	}

	QString ip = ui->combo_clients->itemText(idx);

	if(ip.startsWith("(d)")) return;

	ui->combo_clients->setItemText(idx, QString("(d) ") + ip);

	m->server->dismiss(idx);

	update_dismiss_buttons();
}


void GUI_Broadcast::dismiss_clicked()
{
	int idx = ui->combo_clients->currentIndex();
	dismiss_at(idx);
}


void GUI_Broadcast::dismiss_all_clicked()
{
	for(int idx = 0; idx <ui->combo_clients->count(); idx++){
		dismiss_at(idx);
	}
}


void GUI_Broadcast::combo_changed(int idx)
{
	Q_UNUSED(idx)
	update_dismiss_buttons();
}


bool GUI_Broadcast::check_dismiss_visible() const
{
	if(!is_ui_initialized()){
		return false;
	}

	QString text = ui->combo_clients->currentText();

	if(text.startsWith("(d)")){
		return false;
	}
	else if(!text.isEmpty()){
		return true;
	}

	return false;
}

bool GUI_Broadcast::check_dismiss_all_visible() const
{
	if(!is_ui_initialized()){
		return false;
	}

	return (ui->combo_clients->count() > 0);
}

void GUI_Broadcast::update_dismiss_buttons()
{
	if(!is_ui_initialized()){
		return;
	}

	m->action_dismiss->setVisible(check_dismiss_visible());
	m->action_dismiss_all->setVisible(check_dismiss_all_visible());
}

void GUI_Broadcast::start_server()
{
	bool enabled = GetSetting(Set::Broadcast_Active);
	if(enabled && !m->server)
	{
		m->server = new StreamServer(this);

		connect(m->server, &StreamServer::sig_new_connection, this, &GUI_Broadcast::connection_established);
		connect(m->server, &StreamServer::sig_connection_closed, this, &GUI_Broadcast::connection_closed);
		connect(m->server, &StreamServer::sig_listening, this, &GUI_Broadcast::can_listen_changed);
	}
}

void GUI_Broadcast::mp3_enc_found()
{
	if(!is_ui_initialized()){
		return;
	}

	bool active = GetSetting(SetNoDB::MP3enc_found);
	if(!active)
	{
		ui->combo_clients->hide();
		ui->lab_status->hide();
		ui->lab_error->setText(Lang::get(Lang::CannotFindLame));
	}

	else{
		ui->lab_error->hide();
		ui->btn_retry->hide();
	}

	update_dismiss_buttons();
}
