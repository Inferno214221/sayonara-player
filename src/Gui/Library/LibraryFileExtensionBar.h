/* LibraryFileExtensionBar.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#ifndef LIBRARYFILEEXTENSIONBAR_H
#define LIBRARYFILEEXTENSIONBAR_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

namespace Gui
{
	class ExtensionSet;
}

class AbstractLibrary;

namespace Library
{
	class FileExtensionBar : public Gui::Widget
	{
		Q_OBJECT
		PIMPL(FileExtensionBar)

	signals:
		void sig_close_clicked();

	public:
		explicit FileExtensionBar(QWidget* parent=nullptr);
		~FileExtensionBar() override;

		void init(AbstractLibrary* library);
		void refresh();
		void clear();

		bool has_extensions() const;

	protected:
		void language_changed() override;

	private slots:
		void button_toggled(bool b);
	};
}

#endif // LIBRARYFILEEXTENSIONBAR_H
