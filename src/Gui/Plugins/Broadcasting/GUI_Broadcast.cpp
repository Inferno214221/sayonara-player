/* GUI_Broadcast.cpp */

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

#include "GUI_Broadcast.h"
#include "Gui/Plugins/ui_GUI_Broadcast.h"

#include "Components/Broadcasting/StreamServer.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/PreferenceAction.h"

class BroadcastAction :
	public Gui::PreferenceAction
{
	public:
		BroadcastAction(QWidget* parent);
		~BroadcastAction() override;

		QString identifier() const override;

	protected:
		QString displayName() const override;
};

BroadcastAction::BroadcastAction(QWidget* parent) :
	PreferenceAction(Lang::get(Lang::Broadcast), identifier(), parent) {}

BroadcastAction::~BroadcastAction() = default;

QString BroadcastAction::identifier() const { return "broadcast"; }

QString BroadcastAction::displayName() const { return Lang::get(Lang::Broadcast); }

struct GUI_Broadcast::Private
{
	PlayManager* playManager;
	StreamServer* server = nullptr;
	QAction* actionDismiss = nullptr;
	QAction* actionDismissAll = nullptr;

	Private(PlayManager* playManager) :
		playManager(playManager) {}
};

GUI_Broadcast::GUI_Broadcast(PlayManager* playManager, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<GUI_Broadcast::Private>(playManager);

	ListenSetting(Set::Broadcast_Active, GUI_Broadcast::startServer);
}

GUI_Broadcast::~GUI_Broadcast()
{
	if(m->server)
	{
		m->server->deleteLater();
	}

	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QString GUI_Broadcast::name() const
{
	return "Broadcast";
}

QString GUI_Broadcast::displayName() const
{
	return Lang::get(Lang::Broadcast);
}

void GUI_Broadcast::retranslate()
{
	ui->retranslateUi(this);
	setStatusLabel();
	ui->btn_retry->setText(Lang::get(Lang::Retry));

	if(m->actionDismiss)
	{
		m->actionDismiss->setText(tr("Dismiss"));
		m->actionDismissAll->setText(tr("Dismiss all"));
	}
}

void GUI_Broadcast::initUi()
{
	if(isUiInitialized())
	{
		return;
	}

	setupParent(this, &ui);

	if(m->server)
	{
		const QStringList clients = m->server->connectedClients();
		for(const QString& client : clients)
		{
			ui->combo_clients->addItem(client);
		}
	}

	m->actionDismiss = new QAction(ui->btn_menu);
	m->actionDismissAll = new QAction(ui->btn_menu);

	updateDismissButtons();

	ui->btn_menu->registerAction(m->actionDismiss);
	ui->btn_menu->registerAction(m->actionDismissAll);
	ui->btn_menu->registerAction(new BroadcastAction(ui->btn_menu));

	connect(m->actionDismiss, &QAction::triggered, this, &GUI_Broadcast::dismissClicked);
	connect(m->actionDismissAll, &QAction::triggered, this, &GUI_Broadcast::dismissAllClicked);
	connect(ui->combo_clients, combo_current_index_changed_int, this, &GUI_Broadcast::currentIndexChanged);
	connect(ui->btn_retry, &QPushButton::clicked, this, &GUI_Broadcast::retry);

	setStatusLabel();
	retranslate();

	ListenSetting(SetNoDB::MP3enc_found, GUI_Broadcast::mp3EncoderFound);
}

void GUI_Broadcast::setStatusLabel()
{
	if(!isUiInitialized())
	{
		return;
	}

	int n_listeners = ui->combo_clients->count();

	QString str_listeners = tr("%n listener(s)", "", n_listeners);

	ui->lab_status->setText(str_listeners);
}

// finally connection is established
void GUI_Broadcast::connectionEstablished(const QString& ip)
{
	if(!isUiInitialized())
	{
		return;
	}

	ui->combo_clients->addItem(ip);
	setStatusLabel();
	ui->combo_clients->setCurrentIndex(ui->combo_clients->count() - 1);
}

void GUI_Broadcast::connectionClosed(const QString& ip)
{
	if(!isUiInitialized())
	{
		return;
	}

	if(ip.isEmpty())
	{
		return;
	}

	spLog(Log::Info, this) << "Connection closed: " << ip;

	int idx;
	for(idx = 0; idx < ui->combo_clients->count(); idx++)
	{
		if(ui->combo_clients->itemText(idx).contains(ip))
		{
			break;
		}
	}

	if(idx >= ui->combo_clients->count())
	{
		return;
	}

	ui->combo_clients->removeItem(idx);

	updateDismissButtons();

	setStatusLabel();
}

void GUI_Broadcast::canListenChanged(bool success)
{
	if(!isUiInitialized())
	{
		return;
	}

	ui->lab_status->setVisible(success);
	ui->lab_error->setVisible(!success);
	ui->btn_retry->setVisible(!success);

	if(!success)
	{
		QString msg = tr("Cannot broadcast on port %1").arg(GetSetting(Set::Broadcast_Port));
		msg += "\n" + tr("Maybe another application is using this port?");

		Message::warning(msg);
	}
}

void GUI_Broadcast::retry()
{
	m->server->restart();
}

void GUI_Broadcast::dismissAt(int idx)
{
	if(!isUiInitialized())
	{
		return;
	}

	if(idx < 0 || idx >= ui->combo_clients->count())
	{
		return;
	}

	QString ip = ui->combo_clients->itemText(idx);

	if(ip.startsWith("(d)")) { return; }

	ui->combo_clients->setItemText(idx, QString("(d) ") + ip);

	m->server->dismiss(idx);

	updateDismissButtons();
}

void GUI_Broadcast::dismissClicked()
{
	int idx = ui->combo_clients->currentIndex();
	dismissAt(idx);
}

void GUI_Broadcast::dismissAllClicked()
{
	for(int idx = 0; idx < ui->combo_clients->count(); idx++)
	{
		dismissAt(idx);
	}
}

void GUI_Broadcast::currentIndexChanged(int idx)
{
	Q_UNUSED(idx)
	updateDismissButtons();
}

bool GUI_Broadcast::checkDismissVisible() const
{
	if(!isUiInitialized())
	{
		return false;
	}

	QString text = ui->combo_clients->currentText();

	if(text.startsWith("(d)"))
	{
		return false;
	}
	else if(!text.isEmpty())
	{
		return true;
	}

	return false;
}

bool GUI_Broadcast::checkDismissAllVisible() const
{
	if(!isUiInitialized())
	{
		return false;
	}

	return (ui->combo_clients->count() > 0);
}

void GUI_Broadcast::updateDismissButtons()
{
	if(!isUiInitialized())
	{
		return;
	}

	m->actionDismiss->setVisible(checkDismissVisible());
	m->actionDismissAll->setVisible(checkDismissAllVisible());
}

void GUI_Broadcast::startServer()
{
	bool enabled = GetSetting(Set::Broadcast_Active);
	if(enabled && !m->server)
	{
		m->server = new StreamServer(m->playManager, this);

		connect(m->server, &StreamServer::sigNewConnection, this, &GUI_Broadcast::connectionEstablished);
		connect(m->server, &StreamServer::sigConnectionClosed, this, &GUI_Broadcast::connectionClosed);
		connect(m->server, &StreamServer::sigListening, this, &GUI_Broadcast::canListenChanged);
	}
}

void GUI_Broadcast::mp3EncoderFound()
{
	if(!isUiInitialized())
	{
		return;
	}

	bool active = GetSetting(SetNoDB::MP3enc_found);
	if(!active)
	{
		ui->combo_clients->hide();
		ui->lab_status->hide();
		ui->lab_error->setText(Lang::get(Lang::CannotFindLame));
	}

	else
	{
		ui->lab_error->hide();
		ui->btn_retry->hide();
	}

	updateDismissButtons();
}
