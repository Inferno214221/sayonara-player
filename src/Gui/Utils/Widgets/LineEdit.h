/* LineEdit.h */

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

#ifndef SAYONARA_LINEEDIT_H
#define SAYONARA_LINEEDIT_H

#include <QLineEdit>

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

class QAction;
class QMenu;
class QContextMenuEvent;

namespace Gui
{
	/**
	 * @brief GUI class for String fields meant to convert content to first upper case by a context menu
	 * @ingroup GuiTagging
	 **/
	class LineEdit :
		public Gui::WidgetTemplate<QLineEdit>
	{
		Q_OBJECT
		PIMPL(LineEdit)

		public:
			LineEdit(QWidget* parent=nullptr);
			~LineEdit() override;

		private:
			void initContextMenu();

		private slots:
			void itemTextChanged(const QString& text);
			void itemActionTriggered();
			void removeSpecialCharsTriggered();

		protected:
			void languageChanged() override;

			void keyPressEvent(QKeyEvent* event) override;
			void contextMenuEvent(QContextMenuEvent* event) override;
	};
}

#endif // SAYONARA_LINEEDIT_H
