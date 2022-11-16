/* GUIImportFolder.h */

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

#ifndef GUIIMPORTDIALOG_H_
#define GUIIMPORTDIALOG_H_

#include "Gui/Utils/Widgets/Dialog.h"
#include "Components/Library/Importer/LibraryImporter.h"
#include "Utils/Pimpl.h"

class GUI_TagEdit;
class LocalLibrary;

UI_FWD(GUI_ImportDialog)

class GUI_ImportDialog :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_ImportDialog)
	UI_CLASS_SHARED_PTR(GUI_ImportDialog)

	signals:
		void sigProgress(int);

	public:
		GUI_ImportDialog(LocalLibrary* library, bool copy_enabled, QWidget* parent);
		~GUI_ImportDialog() override;

		void setTargetDirectory(QString targetDirectory);

	private slots:
		void accept() override;
		void reject() override;

		void chooseDirectory();
		void editPressed();
		void setMetadata(const MetaDataList& tracks);
		void setStatus(Library::Importer::ImportStatus status);
		void setProgress(int percent);
		void cachedFilesChanged();

	private:
		QAbstractButton* btnOk();
		QAbstractButton* btnCancel();

	protected:
		void showEvent(QShowEvent* e) override;
};

#endif /* GUIIMPORTFOLDER_H_ */
