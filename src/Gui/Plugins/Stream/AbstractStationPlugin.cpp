/* GUI_AbstractStream.cpp */

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

#include "AbstractStationPlugin.h"

#include "Components/Playlist/PlaylistInterface.h"
#include "Components/Streaming/Streams/AbstractStationHandler.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/MenuTool/MenuToolButton.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Streams/Station.h"

#include <QComboBox>
#include <QPushButton>

namespace
{
	QString streamsAndPodcastString()
	{
		return QString("%1 && %2")
			.arg(Lang::get(Lang::Streams))
			.arg(Lang::get(Lang::Podcasts));
	}

	void configurePlayStopButton(QPushButton* btnPlayStop, const bool isSearching)
	{
		btnPlayStop->setText(isSearching
		                     ? Lang::get(Lang::Stop)
		                     : Lang::get(Lang::Listen));
	}
}

namespace Gui
{
	struct AbstractStationPlugin::Private
	{
		Playlist::Creator* playlistCreator;
		AbstractStationHandler* stationHandler {nullptr};
		ProgressBar* loadingBar {nullptr};
		QComboBox* comboStream {nullptr};
		QPushButton* btnPlay {nullptr};
		MenuToolButton* btnTool {nullptr};
		bool searching {false};

		explicit Private(Playlist::Creator* playlistCreator, AbstractStationHandler* stationHandler) :
			playlistCreator(playlistCreator),
			stationHandler(stationHandler) {}
	};

	AbstractStationPlugin::AbstractStationPlugin(Playlist::Creator* playlistCreator,
	                                             AbstractStationHandler* stationHandler, QWidget* parent) :
		PlayerPlugin::Base(parent),
		m {Pimpl::make<Private>(playlistCreator, stationHandler)} {}

	AbstractStationPlugin::~AbstractStationPlugin() = default;

	void AbstractStationPlugin::assignUiVariables()
	{
		m->comboStream = comboStream();
		m->btnPlay = btnPlay();
		m->btnTool = btnMenu();
		m->loadingBar = new ProgressBar(this);
	}

	void AbstractStationPlugin::initUi()
	{
		m->btnPlay->setFocusPolicy(Qt::StrongFocus);
		m->btnTool->showActions(ContextMenuEntries(ContextMenu::EntryNew));
		m->btnTool->registerPreferenceAction(new StreamPreferenceAction(m->btnTool));
		m->btnTool->registerPreferenceAction(new StreamRecorderPreferenceAction(m->btnTool));

		setTabOrder(m->comboStream, m->btnPlay);

		initConnections();
		setupStations();
		skinChanged();
		setSearching(false);
	}

	void AbstractStationPlugin::initConnections()
	{
		connect(m->btnPlay, &QPushButton::clicked, this, &AbstractStationPlugin::listenClicked);
		connect(m->btnTool, &MenuToolButton::sigEdit, this, &AbstractStationPlugin::editClicked);
		connect(m->btnTool, &MenuToolButton::sigDelete, this, &AbstractStationPlugin::deleteClicked);
		connect(m->btnTool, &MenuToolButton::sigNew, this, &AbstractStationPlugin::newClicked);
		connect(m->btnTool, &MenuToolButton::sigSave, this, &AbstractStationPlugin::saveClicked);
		connect(m->comboStream, combo_activated_int, this, &AbstractStationPlugin::currentIndexChanged);
		connect(m->stationHandler, &AbstractStationHandler::sigError, this, &AbstractStationPlugin::errorReceived);
		connect(m->stationHandler, &AbstractStationHandler::sigDataAvailable, this, [this]() { setSearching(false); });
		connect(m->stationHandler, &AbstractStationHandler::sigStopped, [this]() { setSearching(false); });
		connect(m->stationHandler, &AbstractStationHandler::sigUrlCountExceeded,
		        this, &AbstractStationPlugin::urlCountExceeded);
	}

	void AbstractStationPlugin::restorePreviousIndex(const QString& name)
	{
		if(m->comboStream->count() == 0)
		{
			currentIndexChanged(-1);
		}
		else
		{
			const auto index = std::max(0, m->comboStream->findText(name));
			m->comboStream->setCurrentIndex(index);
			currentIndexChanged(index);
		}
	}

	void AbstractStationPlugin::setupStations()
	{
		const auto lastName = currentName();

		auto stations = QList<StationPtr> {};
		m->stationHandler->getAllStreams(stations);

		m->comboStream->clear();
		for(const auto& station: stations)
		{
			m->comboStream->addItem(station->name(), station->url());
		}

		restorePreviousIndex(lastName);
	}

	void AbstractStationPlugin::currentIndexChanged(const int index)
	{
		const auto isTemporary = m->stationHandler->isTemporary(currentName());
		const auto isEditable = ((index >= 0) && !isTemporary);
		const auto isListentDisabled = ((currentUrl().size() < 8) && !m->searching);

		m->btnTool->showAction(ContextMenu::EntrySave, isTemporary);
		m->btnTool->showAction(ContextMenu::EntryEdit, isEditable);
		m->btnTool->showAction(ContextMenu::EntryDelete, (index >= 0));
		m->comboStream->setToolTip(currentUrl());
		m->btnPlay->setDisabled(isListentDisabled);
	}

	void AbstractStationPlugin::listenClicked()
	{
		if(m->searching)
		{
			m->btnPlay->setDisabled(true);
			m->stationHandler->stop();

			return;
		}

		if(const auto isUrlValid = (currentUrl().size() > 5); isUrlValid)
		{
			const auto stationName = currentName().isEmpty()
			                         ? titleFallbackName()
			                         : currentName();
			setSearching(true);
			play(stationName);

			return;
		}

		spLog(Log::Warning, this) << "Url is empty";
	}

	void AbstractStationPlugin::play(const QString& stationName)
	{
		auto station = m->stationHandler->station(stationName);
		if(station)
		{
			const auto success = m->stationHandler->parseStation(station);
			if(!success)
			{
				spLog(Log::Warning, this) << "Stream Handler busy";
				setSearching(false);
			}
		}
	}

	void AbstractStationPlugin::showConfigDialog(const QString& name, const StationPtr& station,
	                                             const GUI_ConfigureStation::Mode mode,
	                                             std::function<bool(GUI_ConfigureStation*)>&& callback)
	{
		auto* configDialog = createConfigDialog();
		connect(configDialog, &Dialog::accepted, [configDialog, mode, this, callback = std::move(callback)]() {
			const auto success = callback(configDialog);
			configDialog->deleteLater();

			if(success)
			{
				setupStations();
			}

			else
			{
				spLog(Log::Warning, this) << "Could not touch radio station (Mode " << static_cast<int>(mode) << ")";
			}
		});

		configDialog->open();
		configDialog->initUi();
		configDialog->setMode(name, mode);
		configDialog->configureWidgets(station);
	}

	void AbstractStationPlugin::addStream(const StationPtr& station, const bool temporary)
	{
		m->stationHandler->addTemporaryStation(station);
		m->comboStream->addItem(station->name(), station->url());
		m->comboStream->setCurrentText(station->name());
		currentIndexChanged(m->comboStream->currentIndex()); // Also trigger menu re-creation

		if(!temporary)
		{
			saveStation(station); // after successful save, the station will be removed from
		}
		else
		{
			listenClicked();
		}
	}

	void AbstractStationPlugin::saveStation(const StationPtr& station)
	{
		showConfigDialog(station->name(), station, GUI_ConfigureStation::Mode::Save, [this](auto* configDialog) {
			return m->stationHandler->addNewStream(configDialog->configuredStation());
		});
	}

	void AbstractStationPlugin::newClicked()
	{
		showConfigDialog(titleFallbackName(), nullptr, GUI_ConfigureStation::Mode::New, [this](auto* configDialog) {
			const auto station = configDialog->configuredStation();
			const auto isStatioNameValid = (m->comboStream->findText(station->name()) < 0);
			if(!isStatioNameValid)
			{
				configDialog->setError(tr("Please choose another name"));
				configDialog->open();
				return false;
			}

			return m->stationHandler->addNewStream(station);
		});
	}

	void AbstractStationPlugin::saveClicked()
	{
		if(m->stationHandler->isTemporary(currentName()))
		{
			const auto station = m->stationHandler->station(currentName());
			const auto streamAdded = m->stationHandler->addNewStream(station);
			if(streamAdded)
			{
				currentIndexChanged(m->comboStream->currentIndex()); // switch toolbutton from save to edit
			}
		}
	}

	void AbstractStationPlugin::editClicked()
	{
		const auto station = m->stationHandler->station(currentName());
		if(station)
		{
			showConfigDialog(currentName(), station, GUI_ConfigureStation::Mode::Edit, [this](auto* configDialog) {
				return m->stationHandler->updateStream(currentName(), configDialog->configuredStation());
			});
		}
	}

	void AbstractStationPlugin::deleteClicked()
	{
		const auto answer = Message::question_yn(tr("Do you really want to delete %1").arg(currentName()) + "?");
		if(answer == Message::Answer::Yes)
		{
			const auto success = m->stationHandler->removeStream(currentName());
			spLog(Log::Info, this) << "Delete " << currentName() << success;
		}

		setupStations();
	}

	void AbstractStationPlugin::urlCountExceeded(const int urlCount, const int maxUrlCount)
	{
		Message::error(QString("Found %1 urls").arg(urlCount) + "<br />" +
		               QString("Maximum number is %1").arg(maxUrlCount)
		);

		setSearching(false);
	}

	void AbstractStationPlugin::errorReceived()
	{
		setSearching(false);

		const auto question = QString("%1\n%2\n\n%3")
			.arg(tr("Cannot open stream"))
			.arg(currentUrl())
			.arg(Lang::get(Lang::Retry).question());

		const auto answer = Message::question_yn(question);
		if(answer == Message::Answer::Yes)
		{
			listenClicked();
		}
	}

	bool AbstractStationPlugin::hasLoadingBar() const { return true; }

	void AbstractStationPlugin::retranslate()
	{
		configurePlayStopButton(m->btnPlay, m->searching);
	}

	void AbstractStationPlugin::skinChanged()
	{
		if(isUiInitialized())
		{
			setSearching(m->searching);
			btnPlay()->setIcon(Icons::icon(Icons::Play));
		}
	}

	void AbstractStationPlugin::setSearching(const bool isSearching)
	{
		configurePlayStopButton(m->btnPlay, isSearching);

		m->btnPlay->setDisabled(false);
		m->loadingBar->setVisible(isSearching);
		m->searching = isSearching;
	}

	QString AbstractStationPlugin::currentName() const { return m->comboStream->currentText(); }

	QString AbstractStationPlugin::currentUrl() const { return m->comboStream->currentData().toString(); }

	StreamPreferenceAction::StreamPreferenceAction(QWidget* parent) :
		PreferenceAction(streamsAndPodcastString(), "streams", parent) {}

	StreamPreferenceAction::~StreamPreferenceAction() = default;

	QString StreamPreferenceAction::identifier() const { return "streams"; }

	QString StreamPreferenceAction::displayName() const { return streamsAndPodcastString(); }
}