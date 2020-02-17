/* GUI_DirectoryWidget.h */

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

#ifndef GUI_DIRECTORYWIDGET_H
#define GUI_DIRECTORYWIDGET_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Utils/Pimpl.h"

class QPoint;
class QFrame;
class QComboBox;

namespace Library
{
	class Info;
}

UI_FWD(GUI_DirectoryWidget)

/**
 * @brief The GUI_DirectoryWidget class
 * @ingroup GuiDirectories
 */
class GUI_DirectoryWidget :
		public Gui::Widget,
		public InfoDialogContainer
{
	Q_OBJECT
	PIMPL(GUI_DirectoryWidget)
	UI_CLASS(GUI_DirectoryWidget)

	public:
		explicit GUI_DirectoryWidget(QWidget* parent=nullptr);
		~GUI_DirectoryWidget() override;

		QFrame* headerFrame() const;

	private:
		void initShortcuts();
		void initLibraryCombobox();
		void initMenuButton();

	private slots:
		void searchButtonClicked();
		void searchTextEdited(const QString& text);

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
		void dirRenameRequested(const QString& oldName, const QString& new_name);
		void dirCopyToLibRequested(LibraryId libraryId);
		void dirMoveToLibRequested(LibraryId libraryId);

		void fileDoubleClicked(QModelIndex idx);
		void fileEnterPressed();
		void filePressed(QModelIndex idx);
		void fileAppendClicked();
		void filePlayClicked();
		void filePlayNextClicked();
		void filePlayNewTabClicked();
		void fileDeleteClicked();
		void fileRenameRequested(const QString& oldName, const QString& new_name);
		void fileRenameByExpressionRequested(const QString& oldName, const QString& expression);
		void fileCopyToLibraryRequested(LibraryId libraryId);
		void fileMoveToLibraryRequested(LibraryId libraryId);

		void fileOperationStarted();
		void fileOperationFinished();

		void newDirectoryClicked();
		void viewInFileManagerClicked();

		void importRequested(LibraryId libraryId, const QStringList& paths, const QString& targetDirectory);
		void importDialogRequested(const QString& targetDirectory);

		void currentLibraryChanged(int index);
		void setLibraryPathClicked();
		void checkLibraries();

		void splitterMoved(int pos, int index);

	protected:
		MD::Interpretation metadataInterpretation() const override;
		MetaDataList infoDialogData() const override;
		bool hasMetadata() const override;
		QStringList pathlist() const override;

		void languageChanged() override;
		void skinChanged() override;
};

#endif // GUI_DIRECTORYWIDGET_H
