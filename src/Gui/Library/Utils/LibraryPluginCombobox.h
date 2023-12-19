/* LibraryPluginCombobox.h */

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

#ifndef LIBRARYPLUGINCOMBOBOX_H
#define LIBRARYPLUGINCOMBOBOX_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/ComboBox.h"

namespace Library
{
	class PluginHandler;
	class PluginCombobox :
		public Gui::ComboBox
	{
		Q_OBJECT
		PIMPL(PluginCombobox)

		public:
			PluginCombobox(PluginHandler* libraryPluginHandler, const QString& text, QWidget* parent = nullptr);
			~PluginCombobox() override;

		public slots:
			void setupActions();

		protected:
			void skinChanged() override;
			void languageChanged() override;

		private slots:
			void currentLibraryChanged();
			void currentIndexChanged(int index);
	};
}

#endif // LIBRARYPLUGINCOMBOBOX_H
