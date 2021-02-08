/* GUI_AbstractStream.h */

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

#ifndef GUI_ABSTRACT_STREAM_H_
#define GUI_ABSTRACT_STREAM_H_

#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Utils/Pimpl.h"
#include "Utils/Streams/Station.h"

class PlaylistCreator;

class QComboBox;
class QPushButton;
class QLineEdit;
class QLabel;
class AbstractStationHandler;
class GUI_ConfigureStation;

namespace Gui
{
	class MenuToolButton;

	/**
	 * @brief Currently only a Radio Search Entry action
	 */
	class StreamPreferenceAction :
			public PreferenceAction
	{
		Q_OBJECT

		public:
			StreamPreferenceAction(QWidget* parent);
			virtual ~StreamPreferenceAction() override;

			QString identifier() const override;

		protected:
			QString displayName() const override;
	};

	class AbstractStationPlugin :
			public PlayerPlugin::Base
	{
		Q_OBJECT
		PIMPL(AbstractStationPlugin)

		public:
			explicit AbstractStationPlugin(PlaylistCreator* playlistCreator, QWidget* parent=nullptr);
			virtual ~AbstractStationPlugin() override;

		protected:
			virtual void		retranslate() override;
			virtual void		play(const QString& station_name);

			bool				hasLoadingBar() const override;

			template<typename T, typename UiType>
			void setup_parent(T* subclass, UiType** uiptr)
			{
				PlayerPlugin::Base::setupParent(subclass, uiptr);
				AbstractStationPlugin::initUi();
			}

		protected slots:
			void listenClicked();
			void currentIndexChanged(int idx);

			void newClicked();
			void saveClicked();
			void editClicked();
			void deleteClicked();

			void urlCountExceeded(int urlCount, int maxUrlCount);

			void stopped();
			void error();
			void dataAvailable();

		private slots:
			void configFinished();

		protected:
			virtual QComboBox* comboStream()=0;
			virtual QPushButton* btnPlay()=0;
			virtual MenuToolButton* btnMenu()=0;
			virtual AbstractStationHandler* streamHandler() const=0;
			virtual QString	titleFallbackName() const=0;
			virtual GUI_ConfigureStation* createConfigDialog()=0;

			virtual int addStream(const QString& name, const QString& url);

			virtual void initUi() override;
			virtual void assignUiVariables() override;
			virtual void skinChanged() override;

		private:
			void initConnections();
			void setupStations();
	};
}

#endif // GUI_ABSTRACT_STREAM_H_
