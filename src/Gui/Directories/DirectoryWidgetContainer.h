/* DirectoryWidgetContainer.h */

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

#ifndef DIRECTORYWIDGETCONTAINER_H
#define DIRECTORYWIDGETCONTAINER_H

#include "Gui/Utils/Library/LibraryContainerImpl.h"

class GUI_DirectoryWidget;

namespace Library
{
	/**
	 * @brief The DirectoryContainer class
	 * @ingroup GuiDirectories
	 */
	class DirectoryContainer :
			public ContainerImpl
	{
		Q_OBJECT

	private:
		GUI_DirectoryWidget*		ui=nullptr;

	public:

		explicit DirectoryContainer(QObject* parent=nullptr);
		~DirectoryContainer() override;

		QString				displayName() const override;
		QString				name() const override;
		QWidget*			widget() const override;
		QFrame*				header() const override;
		QPixmap				icon() const override;
		void				initUi() override;
	};
}

#endif // DIRECTORYWIDGETCONTAINER_H
