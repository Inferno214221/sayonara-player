/* GUI_AbstractStream.h */

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

#ifndef GUI_ABSTRACT_STREAM_H_
#define GUI_ABSTRACT_STREAM_H_

#include "Gui/Plugins/PlayerPluginBase.h"
#include "GUI_ConfigureStation.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Utils/Pimpl.h"
#include "Utils/Streams/Station.h"

class AbstractStationHandler;
class QComboBox;
class QPushButton;

namespace Playlist
{
	class Creator;
}

namespace Gui
{
	class MenuToolButton;

	class StreamPreferenceAction :
		public PreferenceAction
	{
		Q_OBJECT

		public:
			explicit StreamPreferenceAction(QWidget* parent);
			~StreamPreferenceAction() override;

			[[nodiscard]] QString identifier() const override;

		protected:
			[[nodiscard]] QString displayName() const override;
	};

	class AbstractStationPlugin :
		public PlayerPlugin::Base
	{
		Q_OBJECT
		PIMPL(AbstractStationPlugin)

		public:
			AbstractStationPlugin(Playlist::Creator* playlistCreator, AbstractStationHandler* stationHandler,
			                      QWidget* parent = nullptr);
			~AbstractStationPlugin() override;

		protected:
			[[nodiscard]] virtual QComboBox* comboStream() = 0;
			[[nodiscard]] virtual QPushButton* btnPlay() = 0;
			[[nodiscard]] virtual MenuToolButton* btnMenu() = 0;
			[[nodiscard]] virtual QString titleFallbackName() const = 0;
			[[nodiscard]] virtual GUI_ConfigureStation* createConfigDialog() = 0;

			void initUi() override;
			void assignUiVariables() override;
			void skinChanged() override;
			void retranslate() override;
			[[nodiscard]] bool hasLoadingBar() const override;

			void addStream(const StationPtr& station, bool temporary);

		private slots:
			void listenClicked();
			void currentIndexChanged(int index);
			void newClicked();
			void saveClicked();
			void editClicked();
			void deleteClicked();
			void urlCountExceeded(int urlCount, int maxUrlCount);
			void errorReceived();

		private: // NOLINT(readability-redundant-access-specifiers)
			void showConfigDialog(const QString& name, const StationPtr& station,
			                      GUI_ConfigureStation::Mode mode,
			                      std::function<bool(GUI_ConfigureStation*)>&& callback);
			void saveStation(const StationPtr& station);
			void initConnections();
			void setupStations();
			void restorePreviousIndex(const QString& name);
			void stopSearching();
			[[nodiscard]] QString currentName() const;
			[[nodiscard]] QString currentUrl() const;
			void setSearching(bool b);

			virtual void play(const QString& stationName);

	};
}

#endif // GUI_ABSTRACT_STREAM_H_
