/* GenreView.h */

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

#ifndef LIBRARYGENREVIEW_H
#define LIBRARYGENREVIEW_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Utils/Pimpl.h"

#include <QTreeWidget>

class Genre;
class TreeDelegate;
class LocalLibrary;
class QItemSelection;

namespace Util
{
	template<typename T>
	class Tree;
}

namespace Library
{
	/**
	 * @brief The GenreView class
	 * @ingroup GuiLibrary
	 */
	class GenreView :
		public Gui::WidgetTemplate<QTreeWidget>
	{
			using Parent = Gui::WidgetTemplate<QTreeWidget>;

		Q_OBJECT
		PIMPL(GenreView)

		signals:
			void sigProgress(const QString& name, int progress);
			void sigSelectedChanged(const QStringList& genres);
			void sigInvalidGenreSelected();

		private:
			using Parent::activated;
			using Parent::clicked;
			using Parent::pressed;

		public:
			explicit GenreView(QWidget* parent = nullptr);
			~GenreView() override;

			void init(LocalLibrary* library);
			void reloadGenres();

			static QString invalidGenreName();

		private:
			void initContextMenu();

			void setGenres(const Util::Set<Genre>& genres);

			[[maybe_unused]] QTreeWidgetItem* findGenre(const QString& genre);

		private slots: // NOLINT(*-redundant-access-specifiers)
			void expandCurrentItem();

			void progressChanged(int progress);
			void updateFinished();

			void newPressed();
			void renamePressed();
			void deletePressed();

			void switchTreeList();

			void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

		protected:
			void languageChanged() override;
			void dragEnterEvent(QDragEnterEvent* e) override;
			void dragMoveEvent(QDragMoveEvent* e) override;
			void dragLeaveEvent(QDragLeaveEvent* e) override;
			void dropEvent(QDropEvent* e) override;
			void contextMenuEvent(QContextMenuEvent* e) override;
	};

	class GenreTreeItem :
		public QTreeWidgetItem
	{
		public:
			GenreTreeItem(QTreeWidgetItem* parent, const QStringList& text);
			GenreTreeItem(QTreeWidget* parent, const QStringList& text, bool isInvalidGenre);

			void setInvalidGenre(bool b);
	};
}

#endif // LIBRARYGENREVIEW_H
