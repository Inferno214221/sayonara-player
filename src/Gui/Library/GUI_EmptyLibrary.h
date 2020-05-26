/* GUI_EmptyLibrary.h */

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

#ifndef GUI_EMPTYLIBRARY_H
#define GUI_EMPTYLIBRARY_H

#include "Gui/Utils/Widgets/Widget.h"

UI_FWD(GUI_EmptyLibrary)

class QFrame;

namespace Library
{
	/**
	 * @brief The GUI_EmptyLibrary class
	 * @ingroup Gui
	 * @ingroup Library
	 */
	class GUI_EmptyLibrary :
		public Gui::Widget
	{
		Q_OBJECT
		UI_CLASS(GUI_EmptyLibrary)

		public:
			explicit GUI_EmptyLibrary(QWidget* parent = nullptr);
			GUI_EmptyLibrary(const GUI_EmptyLibrary& other) = delete;
			~GUI_EmptyLibrary() override;

			QFrame* headerFrame() const;

		private:
			bool checkName();
			bool checkPath();

		private slots:
			void okClicked();
			void chooseDirClicked();

			void pathChanged(const QString& str);
			void nameChanged(const QString& str);

		protected:
			void languageChanged() override;
	};
}

#endif // GUI_EMPTYLIBRARY_H
