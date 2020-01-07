/* EmptyLibraryContainer.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef EMPTYLIBRARYCONTAINER_H
#define EMPTYLIBRARYCONTAINER_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Library/LibraryContainerImpl.h"

/**
 * @brief The EmptyLibraryContainer class
 * @ingroup Library
 */
class EmptyLibraryContainer :
		public Library::ContainerImpl
{
	Q_OBJECT
	PIMPL(EmptyLibraryContainer)

	// LibraryContainerInterface interface
public:

	EmptyLibraryContainer(QObject* parent=nullptr);
	~EmptyLibraryContainer() override;

	QString		name() const override;
	QString		display_name() const override;
	QWidget*	widget() const override;
	QMenu*		menu() override;
	QFrame*		header() const override;
	QPixmap		icon() const override;

	void		init_ui() override;
};

#endif // EMPTYLIBRARYCONTAINER_H
