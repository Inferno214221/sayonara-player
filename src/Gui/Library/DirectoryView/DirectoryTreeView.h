/* DirectoryTreeView.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Gui/InfoDialog/InfoDialogContainer.h"

#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Widgets/Dragable.h"
#include "Utils/Pimpl.h"

#include <QTreeView>
#include <QModelIndexList>
#include <QTreeView>

class Model;
class IconProvider;

namespace Gui
{
	class LibraryContextMenu;
	class CustomMimeData;
}

namespace Library
{
	class Info;
	class InfoAccessor;
}

namespace Directory
{
	/**
	 * @brief The DirectoryTreeView class
	 * @ingroup GuiDirectories
	 */
	class TreeView :
		public Gui::WidgetTemplate<QTreeView>,
		public InfoDialogContainer,
		protected Gui::Dragable
	{
		Q_OBJECT
		PIMPL(TreeView)

			using Parent = Gui::WidgetTemplate<QTreeView>;

		signals:
			void sigDeleteClicked();
			void sigPlayClicked();
			void sigPlayNewTabClicked();
			void sigPlayNextClicked();
			void sigAppendClicked();
			void sigDirectoryLoaded(const QModelIndex& index);
			void sigCurrentIndexChanged(const QModelIndex& index);

			void sigEnterPressed();
			void sigImportRequested(LibraryId libraryId, const QStringList& tracks, const QString& targetDirectory);

			void sigCopyRequested(const QStringList& paths, const QString& target);
			void sigMoveRequested(const QStringList& paths, const QString& target);
			void sigRenameRequested(const QString& path, const QString& target);

			void sigCopyToLibraryRequested(LibraryId libraryId);
			void sigMoveToLibraryRequested(LibraryId libraryId);

		public:
			explicit TreeView(QWidget* parent = nullptr);
			~TreeView() override;

			void init(Library::InfoAccessor* libraryInfoAccessor, const Library::Info& info);

			QString directoryName(const QModelIndex& index);

			QModelIndexList selectedRows() const;
			QStringList selectedPaths() const;

			void setFilterTerm(const QString& filter);

		public slots:
			void setBusy(bool b);

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
			void viewInFileManagerClicked();
			void dragTimerTimeout();

		protected:
			void skinChanged() override;

			void keyPressEvent(QKeyEvent* event) override;
			void contextMenuEvent(QContextMenuEvent* event) override;

			void dragEnterEvent(QDragEnterEvent* event) override;
			void dragLeaveEvent(QDragLeaveEvent* event) override;
			void dragMoveEvent(QDragMoveEvent* event) override;
			void dropEvent(QDropEvent* event) override;

			void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

			// InfoDialogContainer interface
			MD::Interpretation metadataInterpretation() const override;
			MetaDataList infoDialogData() const override;
			bool hasMetadata() const override;
			QStringList pathlist() const override;
			QWidget* getParentWidget() override;
	};
}

#endif // DIRECTORYTREEVIEW_H
