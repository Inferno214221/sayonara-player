/* GUI_AbstractStream.cpp */

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

#include "AbstractStationPlugin.h"

#include "Components/Streaming/Streams/AbstractStationHandler.h"
#include "Interfaces/PlaylistInterface.h"

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

namespace
{
	QString streamsAndPodcastString()
	{
		return QString("%1 && %2")
			.arg(Lang::get(Lang::Streams))
			.arg(Lang::get(Lang::Podcasts));
	}
}

namespace Gui
{
	struct AbstractStationPlugin::Private
	{
		QMap<QString, StationPtr> temporaryStations;
		PlaylistCreator* playlistCreator;
		AbstractStationHandler* streamHandler {nullptr};
		ProgressBar* loadingBar {nullptr};
		QComboBox* comboStream {nullptr};
		QPushButton* btnPlay {nullptr};
		MenuToolButton* btnTool {nullptr};
		bool searching {false};

		explicit Private(PlaylistCreator* playlistCreator) :
			playlistCreator(playlistCreator) {}
	};

	AbstractStationPlugin::AbstractStationPlugin(PlaylistCreator* playlistCreator, QWidget* parent) :
		PlayerPlugin::Base(parent),
		m {Pimpl::make<Private>(playlistCreator)} {}

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
		m->streamHandler = streamHandler(); // sub classes might instantiate this new on every call, so only call this method once

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
		connect(m->comboStream, combo_activated_int, this, &AbstractStationPlugin::currentIndexChanged);
		connect(m->streamHandler, &AbstractStationHandler::sigError, this, &AbstractStationPlugin::error);
		connect(m->streamHandler, &AbstractStationHandler::sigDataAvailable, this, [this]() { setSearching(false); });
		connect(m->streamHandler, &AbstractStationHandler::sigStopped, [this]() { setSearching(false); });
		connect(m->streamHandler, &AbstractStationHandler::sigUrlCountExceeded,
		        this, &AbstractStationPlugin::urlCountExceeded);
	}

	void AbstractStationPlugin::setupStations()
	{
		const auto lastName = currentName();
		const auto lastUrl = currentUrl();

		auto stations = QList<StationPtr> {};
		m->streamHandler->getAllStreams(stations);

		m->comboStream->clear();
		for(const auto& station: stations)
		{
			m->comboStream->addItem(station->name(), station->url());
		}

		auto index = Util::Algorithm::indexOf(stations, [&](const auto& station) {
			return (lastName == station->name()) ||
			       (lastUrl == station->url());
		});

		if(m->comboStream->count() > 0)
		{
			index = std::max(0, index);
		}

		m->btnPlay->setEnabled(false);
		if(index >= 0)
		{
			m->comboStream->setCurrentIndex(index);
			currentIndexChanged(m->comboStream->currentIndex());
		}
	}

	void AbstractStationPlugin::currentIndexChanged(const int index)
	{
		const auto isTemporary = m->temporaryStations.contains(currentName());
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
			m->streamHandler->stop();

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
		auto stationPtr = m->streamHandler->station(stationName);
		if(!stationPtr)
		{
			stationPtr = m->temporaryStations[stationName];
		}

		if(stationPtr)
		{
			const auto success = m->streamHandler->parseStation(stationPtr);
			if(!success)
			{
				spLog(Log::Warning, this) << "Stream Handler busy";
				setSearching(false);
			}
		}
	}

	void AbstractStationPlugin::error()
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

		else
		{
			const auto removed = (m->temporaryStations.remove(currentName()) == 0);
			if(removed)
			{
				m->comboStream->removeItem(m->comboStream->currentIndex());
			}
		}
	}

	void AbstractStationPlugin::showConfigDialog(const QString& name, const StationPtr& station,
	                                             const GUI_ConfigureStation::Mode mode,
	                                             std::function<void(GUI_ConfigureStation*)>&& callback)
	{
		auto* configDialog = createConfigDialog();
		connect(configDialog, &Dialog::accepted, [configDialog, callback = std::move(callback)]() {
			callback(configDialog);
			configDialog->deleteLater();
		});

		configDialog->init_ui();
		configDialog->setMode(name, mode);
		configDialog->configureWidgets(station);
		configDialog->open();
	}

	void AbstractStationPlugin::addStream(const QString& name, const QString& url, const bool temporary)
	{
		const auto station = m->streamHandler->createStreamInstance(name, url);

		m->temporaryStations.insert(name, station);
		m->comboStream->addItem(name, url);
		m->comboStream->setCurrentText(name);

		if(!temporary)
		{
			saveStation(station);
		}
	}

	void AbstractStationPlugin::saveStation(const StationPtr& station)
	{
		showConfigDialog(station->name(), station, GUI_ConfigureStation::Mode::Save, [this](auto* configDialog) {
			const auto success = m->streamHandler->addNewStream(configDialog->configuredStation());
			if(success)
			{
				m->temporaryStations.remove(currentName());
			}

			spLog(Log::Info, this) << "Saved " << currentName() << ": " << success;
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
				return;
			}

			const auto streamAdded = m->streamHandler->addNewStream(station);
			if(streamAdded)
			{
				m->temporaryStations.remove(station->name());
			}
		});
	}

	void AbstractStationPlugin::editClicked()
	{
		const auto station = m->streamHandler->station(currentName());
		if(station)
		{
			showConfigDialog(currentName(), station, GUI_ConfigureStation::Mode::Edit, [this](auto* configDialog) {
				const auto success = m->streamHandler->update(currentName(), configDialog->configuredStation());
				spLog(Log::Info, this) << "Updated " << currentName() << ": " << success;
			});
		}
	}

	void AbstractStationPlugin::deleteClicked()
	{
		const auto answer = Message::question_yn(tr("Do you really want to delete %1").arg(currentName()) + "?");
		if(answer == Message::Answer::Yes)
		{
			const auto success = m->streamHandler->deleteStream(currentName());
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

	bool AbstractStationPlugin::hasLoadingBar() const { return true; }

	void AbstractStationPlugin::retranslate()
	{
		const auto text = (m->searching) ?
		                  Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

		m->btnPlay->setText(text);
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
		const auto text = isSearching
		                  ? Lang::get(Lang::Stop)
		                  : Lang::get(Lang::Listen);

		m->btnPlay->setText(text);
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