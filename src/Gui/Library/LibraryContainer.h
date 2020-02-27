/* LibraryContainerImpl.h */

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

#ifndef LIBRARYCONTAINERIMPL_H
#define LIBRARYCONTAINERIMPL_H

#include "Utils/Pimpl.h"
#include "Components/LibraryManagement/AbstractLibraryContainer.h"

/**
 * @brief An interface class needed when implementing a library plugin
 * @ingroup LibraryPlugins
 * @ingroup Interfaces
 */

namespace Library
{
	class PluginHandler;

	class Container :
		public QObject,
		public Library::AbstractContainer
	{
		Q_OBJECT
		PIMPL(Container)

		friend class PluginHandler;

		protected:
			/**
			 * @brief Should initialize the ui. The ui constructor should be called within this function
			 */
			virtual void				initUi()=0;

		public:
			explicit Container(QObject* parent=nullptr);
			virtual ~Container() override;

			void init() override;

			virtual void rename(const QString& new_name) override;
			virtual QString	displayName() const override;
			virtual QMenu* menu() override;
			virtual bool isLocal() const override;
	};
}

#endif // LIBRARYCONTAINER_H
