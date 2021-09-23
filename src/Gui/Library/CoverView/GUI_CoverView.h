/* CoverView.h */

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

#ifndef ALBUMCOVERVIEW_H
#define ALBUMCOVERVIEW_H

#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

#include "Gui/Utils/Widgets/Widget.h"
#include "Gui/Utils/GuiClass.h"

class QTableView;
class AbstractLibrary;
class LocalLibrary;

UI_FWD(GUI_CoverView)

namespace Library
{
	class CoverView;

	class GUI_CoverView :
		public Gui::Widget
	{
		Q_OBJECT
		UI_CLASS_SHARED_PTR(GUI_CoverView)

		signals:
			void sigSortorderChanged(SortOrder so);
			void sigDeleteClicked();
			void sigReloadClicked();

		public:
			explicit GUI_CoverView(QWidget* parent = nullptr);
			virtual ~GUI_CoverView() override;

			void init(LocalLibrary* library);
			bool isInitialized() const;

			IndexSet selectedItems() const;
			void clearSelections() const;

		protected:
			void initSortingActions();
			void initZoomActions();

			void languageChanged() override;
			void showEvent(QShowEvent* e) override;

		private:
			void zoomChanged();
			void sortorderChanged();
			void showArtistChanged();

		private slots:
			void comboSortingChanged(int index);
			void comboZoomChanged(int index);
			void closeClicked();
			void showArtistTriggered(bool showArtist);

			void showUtilsChanged();
	};
}

#endif // ALBUMCOVERVIEW_H
