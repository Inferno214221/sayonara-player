/* LocalLibraryContainer.h */

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

#ifndef LOCALLIBRARYCONTAINER_H
#define LOCALLIBRARYCONTAINER_H

#include "Gui/Utils/Library/LibraryContainerImpl.h"
#include "Utils/Pimpl.h"

namespace Library
{
	class Info;
}

/**
 * @brief The LocalLibraryContainer class
 * @ingroup GuiLibrary
 */
class LocalLibraryContainer :
	public Library::ContainerImpl
{
	Q_OBJECT
	PIMPL(LocalLibraryContainer)

public:
	explicit LocalLibraryContainer(const Library::Info& library, QObject* parent=nullptr);
	virtual ~LocalLibraryContainer() override;

	// override from LibraryViewInterface
	QString     name() const override;
	QString     display_name() const override;
	QWidget*    widget() const override;
	QMenu*      menu() override;
	QFrame*		header() const override;
	QPixmap		icon() const override;
	void        init_ui() override;
	bool		is_local() const override;
	void		rename(const QString& new_name) override;
};

#endif // LOCALLIBRARYCONTAINER_H
