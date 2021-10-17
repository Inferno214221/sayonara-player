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
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/DirectoryChooser.h"

#include "Components/Library/LocalLibrary.h"

#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"

#include <QScrollBar>
#include <QShowEvent>

struct GUI_ImportDialog::Private
{
	Library::Importer* importer = nullptr;
	GUI_TagEdit* tagEditor = nullptr;
	LocalLibrary* library = nullptr;
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

	ui->labTargetPath->setText(library->info().path());
	ui->labTargetPath->setVisible(copyEnabled);
	ui->labTargetInfo->setVisible(copyEnabled);

	ui->btnEdit->setText(Lang::get(Lang::Edit));
	ui->btnEdit->setIcon(Gui::Icons::icon(Gui::Icons::Edit));

	ui->pbProgress->setValue(0);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_ImportDialog::accept);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_ImportDialog::reject);
	connect(ui->btnChooseDirectory, &QPushButton::clicked, this, &GUI_ImportDialog::chooseDirectory);
	connect(ui->btnEdit, &QPushButton::clicked, this, &GUI_ImportDialog::editPressed);

	setModal(true);
}

GUI_ImportDialog::~GUI_ImportDialog()
{
	delete ui;
	ui = nullptr;
}

void GUI_ImportDialog::setTargetDirectory(const QString& targetDirectory)
{
	auto subdir = targetDirectory;
	subdir.remove(m->library->info().path() + "/");

	ui->leDirectory->setText(subdir);
}

void GUI_ImportDialog::setMetadata(const MetaDataList& tracks)
{
	if(!tracks.isEmpty())
	{
		ui->labStatus->setText
			(
				Lang::getWithNumber(Lang::NrTracksFound, tracks.count())
			);
	}

	m->tagEditor->setMetadata(tracks);
	ui->btnEdit->setVisible(!tracks.isEmpty());
}

void GUI_ImportDialog::setStatus(Library::Importer::ImportStatus status)
{
	using Status = Library::Importer::ImportStatus;

	ui->labStatus->show();
	ui->pbProgress->setVisible(status == Status::Importing);
	ui->btnEdit->setVisible(status == Status::CachingFinished);

	btnOk()->setEnabled((status == Status::CachingFinished) || (status == Status::Imported));
	btnCancel()->setEnabled(true);

	switch(status)
	{
		case Status::Caching:
			ui->labStatus->setText(tr("Loading tracks") + "...");
			show();
			break;

		case Status::CachingFinished:
		{
			int cachedFileCount = m->importer->cachedFileCount();
			ui->labStatus->setText(Lang::getWithNumber(Lang::NrTracksFound, cachedFileCount));
		}
			break;

		case Status::NoTracks:
			ui->labStatus->setText(tr("No tracks"));
			break;

		case Status::Importing:
			ui->labStatus->setText(tr("Importing") + "...");
			break;

		case Status::Imported:
			ui->labStatus->setText(tr("Finished"));
			close();
			break;

		case Status::Rollback:
			ui->labStatus->setText(tr("Rollback"));
			btnCancel()->setEnabled(false);
			break;

		case Status::Cancelled:
			ui->labStatus->setText(tr("Cancelled"));
			close();
			break;

		default:
			ui->labStatus->clear();
			break;
	}
}

void GUI_ImportDialog::setProgress(int val)
{
	if(val >= 100)
	{
		val = 0;
	}

	ui->pbProgress->setValue(val);

	emit sigProgress(val);
}

void GUI_ImportDialog::cachedFilesChanged()
{
	int count = m->importer->cachedFileCount();
	QString text = Lang::getWithNumber(Lang::NrTracksFound, count);

	ui->labStatus->setText(text);
}

QAbstractButton* GUI_ImportDialog::btnOk()
{
	return ui->buttonBox->button(QDialogButtonBox::Ok);
}

QAbstractButton* GUI_ImportDialog::btnCancel()
{
	return ui->buttonBox->button(QDialogButtonBox::Cancel);
}

void GUI_ImportDialog::accept()
{
	const auto targetDirectory = ui->leDirectory->text();
	m->importer->acceptImport(targetDirectory);
}

void GUI_ImportDialog::reject()
{
	m->importer->reset();

	using Status = Library::Importer::ImportStatus;
	if((m->importer->status() != Status::Rollback) &&
	   (m->importer->status() != Status::Importing))
	{
		close();
	}
}

void GUI_ImportDialog::chooseDirectory()
{
	const auto libraryPath = m->library->info().path();
	auto dir = Gui::DirectoryChooser::getDirectory(tr("Choose target directory"), libraryPath, true, this);
	if(dir.isEmpty())
	{
		ui->leDirectory->clear();
		return;
	}

	if(!dir.contains(libraryPath))
	{
		Message::warning(tr("%1<br />is no library directory").arg(dir));
		ui->leDirectory->clear();
		return;
	}

	dir.replace(libraryPath, "");
	dir = Util::File::cleanFilename(dir);

	ui->leDirectory->setText(dir);
}

void GUI_ImportDialog::editPressed()
{
	Dialog* dialog = m->tagEditor->boxIntoDialog();

	connect(m->tagEditor, &GUI_TagEdit::sigCancelled, dialog, &Dialog::reject);
	connect(m->tagEditor, &GUI_TagEdit::sigOkClicked, dialog, &Dialog::accept);

	m->tagEditor->show();
	dialog->exec();
}

void GUI_ImportDialog::showEvent(QShowEvent* e)
{
	Dialog::showEvent(e);
	ui->labTargetPath->setText(m->library->info().path());

	this->setStatus(m->importer->status());
}
