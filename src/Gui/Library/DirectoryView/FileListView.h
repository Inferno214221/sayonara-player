/* FileListView.h */

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

#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/Widgets/Dragable.h"

#include "Utils/Pimpl.h"

namespace Library
{
	class Info;
	class InfoAccessor;
}

namespace Directory
{
	class FileListModel;
	/**
	 * @brief The FileListView class
	 * @ingroup GuiDirectories
	 */
	class FileListView :
		public SearchableTableView,
		public InfoDialogContainer,
		private Gui::Dragable
	{
		Q_OBJECT
		PIMPL(FileListView)

		signals:
			void sigDeleteClicked();
			void sigPlayClicked();
			void sigPlayNewTabClicked();
			void sigPlayNextClicked();
			void sigAppendClicked();
			void sigEnterPressed();
			void sigImportRequested(LibraryId lib_id, const QStringList& files, const QString& targetDirectory);

			void sigRenameRequested(const QString& old_name, const QString& newName);
			void sigRenameByExpressionRequested(const QString& oldName, const QString& expression);

			void sigCopyToLibraryRequested(LibraryId libraryId);
			void sigMoveToLibraryRequested(LibraryId libraryId);

		public:
			explicit FileListView(QWidget* parent = nullptr);
			~FileListView() override;

			void init(Library::InfoAccessor* libraryInfoAccessor, const Library::Info& info);

			QStringList selectedPaths() const;

			void setParentDirectory(const QString& dir);
			QString parentDirectory() const;

		private:
			void initContextMenu();

		private slots:
			void renameFileClicked();
			void renameFileByTagClicked();

		protected:
			void contextMenuEvent(QContextMenuEvent* event) override;

			void dragEnterEvent(QDragEnterEvent* event) override;
			void dragMoveEvent(QDragMoveEvent* event) override;
			void dropEvent(QDropEvent* event) override;

			void skinChanged() override;

			// SayonaraSelectionView
			int mapModelIndexToIndex(const QModelIndex& idx) const override;
			ModelIndexRange mapIndexToModelIndexes(int idx) const override;

			// InfoDialogContainer interface
			MD::Interpretation metadataInterpretation() const override;
			MetaDataList infoDialogData() const override;
			bool hasMetadata() const override;
			QStringList pathlist() const override;
			QWidget* getParentWidget() override;
	};
}

#endif // FILELISTVIEW_H
