/* GUI_AbstractLibrary.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "GUI/Utils/Delegates/ComboBoxDelegate.h"
#include "GUI/Utils/Widgets/Widget.h"

#include "Utils/Library/Filter.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/MetaData/MetaDataFwd.h"
#include "Utils/Pimpl.h"

#include <QList>

class QPushButton;
class QComboBox;

class AbstractLibrary;

namespace Library
{
	class TableView;

	class GUI_AbstractLibrary :
			public Gui::Widget
	{
		Q_OBJECT
		PIMPL(GUI_AbstractLibrary)

	public:
		explicit GUI_AbstractLibrary(AbstractLibrary* library,
									 QWidget *parent=nullptr);

		virtual ~GUI_AbstractLibrary();

	private:
		virtual void init();
		virtual void init_search_bar();

	protected:
		virtual void language_changed() override;
		virtual void init_shortcuts();
		virtual TrackDeletionMode show_delete_dialog(int n_tracks)=0;

	protected slots:
		virtual void _sl_live_search_changed();

		virtual void search_esc_pressed();
		virtual void search_return_pressed();
		virtual void search_edited(const QString& searchstring);
		virtual void search_mode_changed(Filter::Mode mode);
		virtual void esc_pressed();
		virtual void query_library();

		virtual void item_delete_clicked();
		void tracks_delete_clicked();
		virtual void show_delete_answer(QString);


	public slots:
		virtual void id3_tags_changed();


	protected:
		virtual TableView* lv_artist() const=0;
		virtual TableView* lv_album() const=0;
		virtual TableView* lv_tracks() const=0;
		virtual QLineEdit* le_search() const=0;

		virtual QList<Filter::Mode> search_options() const=0;

		template<typename T, typename UI>
		void setup_parent(T* subclass, UI** ui)
		{
			*ui = new UI();

			UI* ui_ptr = *ui;
			ui_ptr->setupUi(subclass);

			init();
		}
	};
}

#endif // GUI_ABSTRACTLIBRARY_H
