/* GenreView.h */

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

#ifndef LIBRARYGENREVIEW_H
#define LIBRARYGENREVIEW_H

#include <QTreeWidget>

#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "GUI/Utils/SearchableWidget/SearchableView.h"
#include "Utils/Pimpl.h"
#include "Utils/SetFwd.h"

class MetaDataList;
class Genre;
class TreeDelegate;
class QStringList;
class LocalLibrary;

namespace SP
{
	template<typename T>
	class Tree;
}

using GenreNode=SP::Tree<QString>;

namespace Library
{
	class GenreView :
			public Gui::WidgetTemplate<QTreeWidget>
	{
		Q_OBJECT
		PIMPL(GenreView)

	signals:
		void sig_progress(const QString& message, int progress);
		void sig_genres_reloaded();

	public:
		explicit GenreView(QWidget* parent=nullptr);
		~GenreView();

		void reload_genres();
		bool has_items() const;
		void set_local_library(LocalLibrary* library);

	private:
		void set_genres(const SP::Set<Genre>& genres);
		void build_genre_data_tree(const SP::Set<Genre>& genres);
		void populate_widget(QTreeWidgetItem* parent_item, GenreNode* node);

		QTreeWidgetItem* find_genre(const QString& genre);

		void init_context_menu();

	private slots:
		void item_expanded(QTreeWidgetItem* item);
		void item_collapsed(QTreeWidgetItem* item);

		void progress_changed(int progress);
		void update_finished();

		void new_pressed();
		void rename_pressed();
		void delete_pressed();

		void tree_action_changed();
		void tree_action_toggled(bool b);

	protected:
		void language_changed() override;

		void keyPressEvent(QKeyEvent* e) override;
		void dragEnterEvent(QDragEnterEvent* e) override;
		void dragMoveEvent(QDragMoveEvent* e) override;
		void dragLeaveEvent(QDragLeaveEvent* e) override;
		void dropEvent(QDropEvent* e) override;
		void contextMenuEvent(QContextMenuEvent* e) override;
	};
}

#endif // LIBRARYGENREVIEW_H
