/* DirectoryTreeView.h */

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

#ifndef DIRECTORYTREEVIEW_H
#define DIRECTORYTREEVIEW_H

#include "DirectoryModel.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Widgets/Dragable.h"
#include "Utils/Pimpl.h"

#include <QTreeView>
#include <QModelIndexList>
#include <QTreeView>

class DirectoryModel;
class IconProvider;

namespace Gui
{
	class LibraryContextMenu;
	class CustomMimeData;
}

namespace Library
{
	class Info;
}

using SearchableTreeView=Gui::WidgetTemplate<SearchableView<QTreeView, DirectoryModel>>;

/**
 * @brief The DirectoryTreeView class
 * @ingroup GuiDirectories
 */
class DirectoryTreeView :
		public SearchableTreeView,
		protected Gui::Dragable
{
	Q_OBJECT
	PIMPL(DirectoryTreeView)

	signals:
		void sigInfoClicked();
		void sigEditClicked();
		void sigLyricsClicked();
		void sigDeleteClicked();
		void sigPlayClicked();
		void sigPlayNewTabClicked();
		void sigPlayNextClicked();
		void sigAppendClicked();
		void sigDirectoryLoaded(const QModelIndex& index);
		void sigCurrentIndexChanged(const QModelIndex& index);

		void sigEnterPressed();
		void sigImportRequested(LibraryId lib_id, const QStringList& v_md, const QString& targetDirectory);

		void sigCopyRequested(const QStringList& paths, const QString& target);
		void sigMoveRequested(const QStringList& paths, const QString& target);
		void sigRenameRequested(const QString& path, const QString& target);

		void sigCopyToLibraryRequested(LibraryId libraryId);
		void sigMoveToLibraryRequested(LibraryId libraryId);

	public:
		explicit DirectoryTreeView(QWidget* parent=nullptr);
		~DirectoryTreeView() override;

		QModelIndex		search(const QString& searchTerm);
		QString			directoryName(const QModelIndex& index);

		QModelIndexList	selctedRows() const;
		QStringList		selectedPaths() const;

		QMimeData*		dragableMimedata() const override;

		void			setLibraryInfo(const Library::Info& info);
		void			setBusy(bool b);

	private:
		enum class DropAction
		{
			Copy,
			Move,
			Cancel
		};

		void initContextMenu();
		DropAction showDropMenu(const QPoint& pos);
		void handleSayonaraDrop(const Gui::CustomMimeData* mimedata, const QString& targetDirectory);


	private slots:
		void createDirectoryClicked();
		void renameDirectoryClicked();
		void dragTimerTimeout();


	protected:
		// Dragable
		bool hasDragLabel() const override;
		QString dragLabel() const override;

		void skinChanged() override;

		void keyPressEvent(QKeyEvent* event) override;
		void contextMenuEvent(QContextMenuEvent* event) override;

		void dragEnterEvent(QDragEnterEvent *event) override;
		void dragLeaveEvent(QDragLeaveEvent* event) override;
		void dragMoveEvent(QDragMoveEvent *event) override;
		void dropEvent(QDropEvent *event) override;

		// SayonaraSelectionView
		void selectMatch(const QString& str, SearchDirection direction) override;
		void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

		int mapModelIndexToIndex(const QModelIndex& idx) const override;
		ModelIndexRange mapIndexToModelIndexes(int idx) const override;
};

#endif // DIRECTORYTREEVIEW_H
