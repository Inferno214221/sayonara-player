/* GUI_DirectoryView.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef GUI_DIRECTORYVIEW_H
#define GUI_DIRECTORYVIEW_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_DirectoryView)

namespace Library { class Manager; }

class QItemSelection;

class GUI_DirectoryView :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_DirectoryView)
	UI_CLASS(GUI_DirectoryView)

	public:
		explicit GUI_DirectoryView(QWidget* parent=nullptr);
		~GUI_DirectoryView() override;

		void init(Library::Manager* libraryManager, LibraryId libraryId);
		void setFilterTerm(const QString& filter);

	private:
		void initUi();

	private slots:
		void load();

		void importRequested(LibraryId id, const QStringList& paths, const QString& targetDirectory);
		void importDialogRequested(const QString& targetDirectory);

		void newDirectoryClicked();
		void viewInFileManagerClicked();

		void dirEnterPressed();
		void dirOpened(QModelIndex idx);
		void dirPressed(QModelIndex idx);
		void dirClicked(QModelIndex idx);
		void dirAppendClicked();
		void dirPlayClicked();
		void dirPlayNextClicked();
		void dirPlayInNewTabClicked();
		void dirDeleteClicked();
		void dirCopyRequested(const QStringList& files, const QString& target);
		void dirMoveRequested(const QStringList& files, const QString& target);
		void dirRenameRequested(const QString& oldName, const QString& newName);
		void dirCopyToLibRequested(LibraryId libraryId);
		void dirMoveToLibRequested(LibraryId libraryId);
		void dirSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		void fileDoubleClicked(QModelIndex idx);
		void fileEnterPressed();
		void filePressed(QModelIndex idx);
		void fileAppendClicked();
		void filePlayClicked();
		void filePlayNextClicked();
		void filePlayNewTabClicked();
		void fileDeleteClicked();
		void fileRenameRequested(const QString& oldName, const QString& newName);
		void fileRenameByExpressionRequested(const QString& oldName, const QString& expression);
		void fileCopyToLibraryRequested(LibraryId libraryId);
		void fileMoveToLibraryRequested(LibraryId libraryId);
		void fileSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		void fileOperationStarted();
		void fileOperationFinished();

		void splitterMoved(int pos, int index);
		void createDirectoryClicked();

	protected:		
		void languageChanged() override;
		void skinChanged() override;
		void showEvent(QShowEvent* event) override;
};

#endif // GUI_DIRECTORYVIEW_H
