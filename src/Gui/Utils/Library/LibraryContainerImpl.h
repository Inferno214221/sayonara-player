/* LibraryContainerImpl.h */

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

#ifndef LIBRARYCONTAINERIMPL_H
#define LIBRARYCONTAINERIMPL_H

#include "Utils/Pimpl.h"
#include "Interfaces/Library/LibraryContainer.h"

/**
 * @brief An interface class needed when implementing a library plugin
 * @ingroup LibraryPlugins
 * @ingroup Interfaces
 */

namespace Library
{
	class PluginHandler;

	class ContainerImpl :
		public QObject,
		public Library::Container
	{
		Q_OBJECT
		PIMPL(ContainerImpl)

		friend class PluginHandler;

		protected:
			/**
			 * @brief Should initialize the ui. The ui constructor should be called within this function
			 */
			virtual void				init_ui()=0;

			/**
			 * @brief this is a frame at the top left of the container
			 * where the combo box will be located
			 * @return
			 */
			virtual QFrame*				header() const=0;

		public:
			explicit ContainerImpl(QObject* parent=nullptr);
			virtual ~ContainerImpl() override;

			virtual void rename(const QString& new_name) override;

			virtual QString				display_name() const override;

			virtual QMenu*				menu() override;
			void						init() override;
			virtual bool				is_local() const override;
	};
}

#endif // LIBRARYCONTAINER_H
