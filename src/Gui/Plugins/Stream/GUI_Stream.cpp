/* GUI_Stream.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "ConfigureStreamDialog.h"
#include "GUI_StationSearcher.h"
#include "Gui/Plugins/ui_GUI_Stream.h"

#include "Components/Streaming/Streams/StreamHandler.h"
#include "Interfaces/PlaylistInterface.h"

#include "Utils/Language/Language.h"
#include "Gui/Utils/Icons.h"

#include <QAction>

struct GUI_Stream::Private
{
	GUI_StationSearcher* searcher = nullptr;
	QAction* actionSearchRadioStation = nullptr;
	StreamHandler* streamHandler;

	explicit Private(StreamHandler* streamHandler) :
		streamHandler {streamHandler} {}
};

GUI_Stream::GUI_Stream(PlaylistCreator* playlistCreator, StreamHandler* streamHandler, QWidget* parent) :
	Gui::AbstractStationPlugin(playlistCreator, streamHandler, parent),
	m {Pimpl::make<Private>(streamHandler)} {}

GUI_Stream::~GUI_Stream()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QString GUI_Stream::name() const
{
	return "Webstreams";
}

QString GUI_Stream::displayName() const
{
	return Lang::get(Lang::Streams);
}

void GUI_Stream::retranslate()
{
	Gui::AbstractStationPlugin::retranslate();
	ui->retranslateUi(this);

	const auto actionText = tr("Search radio station");

	if(m->actionSearchRadioStation)
	{
		m->actionSearchRadioStation->setText(actionText);
	}

	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->btnSearch->setToolTip(actionText);
}

void GUI_Stream::initUi()
{
	setupParent(this, &ui);
	Gui::AbstractStationPlugin::initUi();

	m->actionSearchRadioStation = new QAction(ui->btnTool);
	ui->btnTool->registerAction(m->actionSearchRadioStation);

	connect(m->actionSearchRadioStation, &QAction::triggered, this, &GUI_Stream::searchRadioTriggered);
	connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_Stream::searchRadioTriggered);

	retranslate();
}

QString GUI_Stream::titleFallbackName() const { return Lang::get(Lang::Radio); }

QComboBox* GUI_Stream::comboStream() { return ui->comboStream; }

QPushButton* GUI_Stream::btnPlay() { return ui->btnListen; }

Gui::MenuToolButton* GUI_Stream::btnMenu() { return ui->btnTool; }

void GUI_Stream::skinChanged()
{
	Gui::AbstractStationPlugin::skinChanged();

	if(m->actionSearchRadioStation)
	{
		m->actionSearchRadioStation->setIcon(Gui::Icons::icon(Gui::Icons::Search));
	}

	if(ui)
	{
		ui->btnSearch->setIcon(Gui::Icons::icon(Gui::Icons::Search));
	}
}

void GUI_Stream::searchRadioTriggered()
{
	if(!m->searcher)
	{
		m->searcher = new GUI_StationSearcher(this);
		connect(m->searcher, &GUI_StationSearcher::sigStreamSelected, this, &GUI_Stream::streamSelected);
	}

	m->searcher->show();
}

void GUI_Stream::streamSelected(const QString& name, const QString& url, const bool save)
{
	addStream(m->streamHandler->createStreamInstance(name, url), !save);
}

GUI_ConfigureStation* GUI_Stream::createConfigDialog()
{
	return new ConfigureStreamDialog(this);
}
