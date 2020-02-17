/* LocalLibraryMenu.h */

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

#ifndef LOCALLIBRARYMENU_H
#define LOCALLIBRARYMENU_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"
#include "Utils/Pimpl.h"

#include <QMenu>
#include <QAction>

namespace Gui
{
	class PreferenceAction;
}

namespace Library
{
	/**
	 * @brief A menu in the player's menubar containing
	 * some library actions
	 * @ingroup GuiLibrary
	 */
	class LocalLibraryMenu :
			public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(LocalLibraryMenu)

		signals:
			void sigReloadLibrary();
			void sigImportFile();
			void sigImportFolder();
			void sigInfo();

			void sigNameChanged(const QString& name);
			void sigPathChanged(const QString& path);

		public:
			explicit LocalLibraryMenu(const QString& name, const QString& path, QWidget* parent=nullptr);
			~LocalLibraryMenu() override;

			void refreshName(const QString& name);
			void refreshPath(const QString& path);

			void setLibraryBusy(bool b);
			void setLibraryEmpty(bool b);

			void addPreferenceAction(Gui::PreferenceAction* action);

		private:
			void initMenu();
			void shortcutChanged(ShortcutIdentifier identifier);

		private slots:
			void showAlbumArtistsChanged();
			void showAlbumArtistsTriggered(bool b);

			void livesearchTriggered();
			void setLiveSearchEnabled(bool b);

			void editClicked();
			void editAccepted();

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}

#endif // LOCALLIBRARYMENU_H
