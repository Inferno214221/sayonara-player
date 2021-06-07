
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

#ifndef LIBRARY_HEADERVIEW_H
#define LIBRARY_HEADERVIEW_H

#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/Widgets/HeaderView.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

namespace Library
{
	/**
	 * @brief The HeaderView class
	 * @ingroup GuiLibrary
	 */
	class HeaderView :
			public Gui::HeaderView
	{
		Q_OBJECT
		PIMPL(HeaderView)

		signals:
			void sigAutoResizeToggled(bool b);

		public:
			HeaderView(Qt::Orientation orientation, QWidget* parent=nullptr);
			virtual ~HeaderView() override;

			void init(const ColumnHeaderList& columnHeaderList, const QByteArray& state, Library::SortOrder sortOrder, bool autoResizeState);
			void resizeColumnsAutomatically();

			Library::SortOrder sortorder(int index, Qt::SortOrder sortOrder);
			QString columnText(int index) const;
			void reloadColumnTexts();

			QSize sizeHint() const override;

		protected:
			void languageChanged() override;
			void showEvent(QShowEvent* e) override;
			void resizeEvent(QResizeEvent* e) override;

		private:
			void initializeView();

		private slots:
			void actionTriggered(bool b);
			void actionResizeTriggered();
			void actionAutoResizeTriggered(bool b);
	};
}

#endif // LIBRARY_HEADERVIEW_H
