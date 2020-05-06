/* GUIImportFolder.cpp */

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

#include "GUI_ImportDialog.h"
#include "Gui/Library/ui_GUI_ImportDialog.h"
#include "Gui/Tagging/GUI_TagEdit.h"

#include "Components/Library/LocalLibrary.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"

#include <QPixmap>
#include <QScrollBar>
#include <QFileDialog>
#include <QCloseEvent>
#include <QShowEvent>


struct GUI_ImportDialog::Private
{
	Library::Importer*	importer=nullptr;
	GUI_TagEdit*		tagEditor=nullptr;
	LocalLibrary*		library=nullptr;
};

GUI_ImportDialog::GUI_ImportDialog(LocalLibrary* library, bool copyEnabled, QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>();
	ui = new Ui::GUI_ImportDialog();
	ui->setupUi(this);

	m->library = library;
	m->tagEditor = new GUI_TagEdit(this);
	m->tagEditor->hide();
	m->importer = m->library->importer();

	connect(m->importer, &Library::Importer::sigMetadataCached, this, &GUI_ImportDialog::setMetadata);
	connect(m->importer, &Library::Importer::sigStatusChanged, this, &GUI_ImportDialog::setStatus);
	connect(m->importer, &Library::Importer::sigProgress, this, &GUI_ImportDialog::setProgress);
	connect(m->importer, &Library::Importer::sigCachedFilesChanged, this, &GUI_ImportDialog::cachedFilesChanged);
	connect(m->importer, &Library::Importer::sigTriggered, this, &GUI_ImportDialog::show);

    ui->lab_targetPath->setText(library->path());
    ui->lab_targetPath->setVisible(copyEnabled);
    ui->lab_targetInfo->setVisible(copyEnabled);

    ui->pb_progress->setValue(0);

	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_ImportDialog::accept);
	connect(ui->btn_chooseDirectory, &QPushButton::clicked, this, &GUI_ImportDialog::chooseDirectory);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &GUI_ImportDialog::reject);
	connect(ui->btn_edit, &QPushButton::clicked, this, &GUI_ImportDialog::editPressed);

	setModal(true);
}

GUI_ImportDialog::~GUI_ImportDialog()
{
	delete ui; ui = nullptr;
}

void GUI_ImportDialog::setTargetDirectory(const QString& targetDirectory)
{
	QString subdir = targetDirectory;
	subdir.remove(m->library->path() + "/");

	ui->le_directory->setText(subdir);
}

void GUI_ImportDialog::languageChanged()
{
	ui->retranslateUi(this);
	ui->btn_edit->setText(Lang::get(Lang::Edit));
	ui->btn_cancel->setText(Lang::get(Lang::Cancel));
}

void GUI_ImportDialog::setMetadata(const MetaDataList& v_md)
{
	if(!v_md.isEmpty())
	{
		ui->lab_status->setText
		(
			Lang::getWithNumber(Lang::NrTracksFound, v_md.count())
		);
	}

	m->tagEditor->setMetadata(v_md);
	ui->btn_edit->setVisible( !v_md.isEmpty() );
}

void GUI_ImportDialog::setStatus(Library::Importer::ImportStatus status)
{
	using Status=Library::Importer::ImportStatus;

	ui->lab_status->show();
	ui->pb_progress->setVisible(status == Status::Importing);
	ui->btn_cancel->setText(Lang::get(Lang::Cancel));

	ui->btn_ok->setEnabled((status == Status::CachingFinished) || (status == Status::Imported));
	ui->btn_edit->setEnabled(status == Status::CachingFinished);
	ui->btn_cancel->setEnabled(true);

	switch(status)
	{
		case Status::Caching:
			ui->lab_status->setText(tr("Loading tracks") + "...");
			show();
			break;

        case Status::CachingFinished:
        {
			int cachedFileCount = m->importer->cachedFileCount();
			ui->lab_status->setText(Lang::getWithNumber(Lang::NrTracksFound, cachedFileCount));
			ui->btn_cancel->setText(Lang::get(Lang::Close));
        } break;

   		case Status::NoTracks:
			ui->lab_status->setText(tr("No tracks"));
			ui->btn_cancel->setText(Lang::get(Lang::Close));
			break;

		case Status::Importing:
			ui->lab_status->setText(tr("Importing") + "...");
			break;

		case Status::Imported:
			ui->lab_status->setText(tr("Finished"));
			ui->btn_cancel->setText(Lang::get(Lang::Close));
			close();
			break;

		case Status::Rollback:
			ui->lab_status->setText(tr("Rollback"));
			ui->btn_cancel->setEnabled(false);
			break;

		case Status::Cancelled:
			ui->lab_status->setText(tr("Cancelled"));
			ui->btn_cancel->setText(Lang::get(Lang::Close));
			close();
			break;

		default:
			ui->lab_status->setText(Lang::getWithNumber(Lang::NrTracksFound, 0));
			ui->btn_cancel->setText(Lang::get(Lang::Close));
			break;
    }
}

void GUI_ImportDialog::setProgress(int val)
{
	if(val >= 100) {
		val = 0;
	}

	ui->pb_progress->setValue(val);

	emit sigProgress(val);
}

void GUI_ImportDialog::cachedFilesChanged()
{
	int count = m->importer->cachedFileCount();
	QString text = Lang::getWithNumber(Lang::NrTracksFound, count);

	ui->lab_status->setText(text);
}

void GUI_ImportDialog::accept()
{
	QString targetDirectory = ui->le_directory->text();
	m->importer->acceptImport(targetDirectory);
}

void GUI_ImportDialog::reject()
{
	m->importer->reset();

	Library::Importer::ImportStatus status = m->importer->status();

	if( status == Library::Importer::ImportStatus::Cancelled ||
		status == Library::Importer::ImportStatus::NoTracks	 ||
		status == Library::Importer::ImportStatus::NoValidTracks ||
		status == Library::Importer::ImportStatus::Imported )
	{
		Gui::Dialog::reject();
		close();
	}
}

void GUI_ImportDialog::chooseDirectory()
{
	const QString libraryPath = m->library->path();
	QString dir = QFileDialog::getExistingDirectory(this,
		tr("Choose target directory"),
		libraryPath,
		QFileDialog::ShowDirsOnly
	);
	if(dir.isEmpty()){
		ui->le_directory->clear();
		return;
	}

	if(!dir.contains(libraryPath)) {
		Message::warning(tr("%1<br />is no library directory").arg(dir));
		ui->le_directory->clear();
		return;
	}

	dir.replace(libraryPath, "");
	dir = Util::File::cleanFilename(dir);

	ui->le_directory->setText(dir);
}

void GUI_ImportDialog::editPressed()
{
	Dialog* dialog = m->tagEditor->boxIntoDialog();

	connect(m->tagEditor, &GUI_TagEdit::sigCancelled, dialog, &Dialog::reject);
	connect(m->tagEditor, &GUI_TagEdit::sigOkClicked, dialog, &Dialog::accept);

	m->tagEditor->show();
	dialog->exec();
}

void GUI_ImportDialog::closeEvent(QCloseEvent* e)
{
	Dialog::closeEvent(e);
}

void GUI_ImportDialog::showEvent(QShowEvent* e)
{
	Dialog::showEvent(e);
	ui->lab_targetPath->setText( m->library->path() );

	this->setStatus(m->importer->status());
}
