/* LibraryContainer.h */

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

#ifndef LIBRARYCONTAINERINTERFACE_H
#define LIBRARYCONTAINERINTERFACE_H

#include "Utils/Pimpl.h"

#include <QObject>

class QFrame;
class QAction;
class QPixmap;
class QMenu;
class QWidget;
class QComboBox;
class Settings;

/**
 * @brief An interface class needed when implementing a library plugin
 * @ingroup LibraryPlugins
 * @ingroup Interfaces
 */

namespace Library
{
	class PluginHandler;

	class Container :
		public QObject
	{
		Q_OBJECT
		PIMPL(Container)

		friend class PluginHandler;

		private:
			void set_initialized();

		public:
			explicit Container(QObject* parent=nullptr);
			virtual ~Container();

			/**
			 * @brief Should return an untranslated name used for identifying this widget
			 * @return name
			 */
			virtual QString				name() const=0;

			virtual void				set_name(const QString& name);

			/**
			 * @brief Should return the translated name displayed in the library view combobox
			 * @return display name
			 */
			virtual QString				display_name() const;

			/**
			 * @brief Should return the UI for the library view
			 * @return pointer to the ui
			 */
			virtual QWidget*			widget() const=0;

			/**
			 * @brief this is a frame at the top left of the container
			 * where the combo box will be located
			 * @return
			 */
			virtual QFrame*				header() const=0;

			/**
			 * @brief Every library should show a icon in the
			 * combo box
			 * @return
			 */
			virtual QPixmap				icon() const=0;


			/**
			 * @brief return actions menu (may be nullptr). The title does not have to be set
			 * @return the translated menu relevant for the corresponding library
			 */
			virtual QMenu*				menu();


			/**
			 * @brief sets the action member field used in the player menu bar. This is
			 * called by the player if the language has changed
			 * @param action the new translated action
			 */
			void						set_menu_action(QAction* action);

			/**
			 * @brief get the action in the player menubar.
			 * @return pointer to action field
			 */
			QAction*					menu_action() const;


			/**
			 * @brief Should initialize the ui. The ui constructor should be called within this function
			 */
			virtual void				init_ui()=0;

			/**
			 * @brief tests, if the ui already has been initialized
			 * @return true if ui has been initialized, false else
			 */
			bool						is_initialized() const;

			/**
			 * @brief a local library is a library which writes to the
			 * library field of the database. This should be false for
			 * every new plugin
			 * @return
			 */
			virtual bool				is_local() const;

			virtual	void				show();

			virtual void				hide();

			void						retranslate();
	};
}

using LibraryContainerInterface=Library::Container;
Q_DECLARE_INTERFACE(LibraryContainerInterface, "com.sayonara-player.library")

#endif // LIBRARYCONTAINER_H