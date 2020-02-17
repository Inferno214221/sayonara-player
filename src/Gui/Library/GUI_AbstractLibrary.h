/* GUI_AbstractLibrary.h */

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

#ifndef GUI_ABSTRACTLIBRARY_H
#define GUI_ABSTRACTLIBRARY_H

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"
#include "Gui/Utils/Widgets/Widget.h"

#include "Utils/Library/Filter.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Pimpl.h"

class QPushButton;
class QComboBox;
class AbstractLibrary;

namespace Library
{
	class TableView;
	class SearchBar;

	/**
	 * @brief The GUI_AbstractLibrary class
	 * @ingroup GuiLibrary
	 */
	class GUI_AbstractLibrary :
			public Gui::Widget
	{
		Q_OBJECT
		PIMPL(GUI_AbstractLibrary)

	public:
		explicit GUI_AbstractLibrary(AbstractLibrary* library,
									 QWidget* parent=nullptr);

		virtual ~GUI_AbstractLibrary() override;

	private:
		virtual void init();
		virtual void initSearchBar();

	protected:
		virtual void languageChanged() override;
		virtual void initShortcuts();
		virtual bool hasSelections() const;

		virtual TrackDeletionMode showDeleteDialog(int n_tracks)=0;

	protected slots:
		virtual void liveSearchChanged();

		virtual void clearSelections();
		virtual void searchTriggered();
		virtual void searchEdited(const QString& searchstring);
		virtual void keyPressed(int key);
		virtual void queryLibrary();

		virtual void itemDeleteClicked();
		virtual void showDeleteAnswer(QString);

		void tracksDeleteClicked();

	protected:
		virtual TableView* lvArtist() const=0;
		virtual TableView* lvAlbum() const=0;
		virtual TableView* lvTracks() const=0;
		virtual SearchBar* leSearch() const=0;

		virtual QList<Filter::Mode> searchOptions() const=0;

		template<typename T, typename UI>
		void setupParent(T* subclass, UI** ui)
		{
			*ui = new UI();

			UI* uiPtr = *ui;
			uiPtr->setupUi(subclass);

			init();
		}
	};
}

#endif // GUI_ABSTRACTLIBRARY_H
