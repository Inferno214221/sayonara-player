/* GUI_PlayerMenubar.h */

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

#ifndef GUI_PLAYERMENUBAR_H
#define GUI_PLAYERMENUBAR_H

#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QMenuBar>

namespace Library
{
	class AbstractContainer;
}

namespace PlayerPlugin
{
	class Base;
}

class PlaylistCreator;

class Menubar :
	public Gui::WidgetTemplate<QMenuBar>
{
	Q_OBJECT
	PIMPL(Menubar)

	signals:
		void sigCloseClicked();
		void sigMinimizeClicked();
		void sigLoggerClicked();

	public:
		explicit Menubar(PlaylistCreator* playlistCreator, QWidget* parent = nullptr);
		~Menubar() override;

		void insertPreferenceAction(QAction* action);

		void showLibraryAction(bool visible);
		void setShowLibraryActionEnabled(bool b);
		void showLibraryMenu(bool b);

	private:
		void initDonateLink();
		void initConnections();
		void styleChanged();

		QAction* changeCurrentLibrary(Library::AbstractContainer* library);

	private slots:
		void openDirClicked();
		void openFilesClicked();
		void shutdownClicked();
		void closeClicked();
		void minimizeClicked();
		void skinToggled(bool b);
		void bigCoverToggled(bool b);
		void showLibraryToggled(bool b);
		void showFullscreenToggled(bool b);
		void helpClicked();
		void aboutClicked();
		void shortcutChanged(ShortcutIdentifier identifier);
		void pluginAdded(PlayerPlugin::Base* plugin);

	protected:
		void languageChanged() override;
		void skinChanged() override;
};

#endif // GUI_PLAYERMENUBAR_H
