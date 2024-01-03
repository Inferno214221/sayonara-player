/* GUIImportFolder.cpp */

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
	Library::Importer* libraryImporter;
	QString libraryPath;
	std::shared_ptr<GUI_TagEdit> tagEditor;

	Private(LocalLibrary* library, QWidget* parent) :
		libraryImporter {library->importer()},
		libraryPath {library->info().path()},
		tagEditor {std::make_shared<GUI_TagEdit>(parent)}
	{
		tagEditor->hide();
	}
};

GUI_ImportDialog::GUI_ImportDialog(LocalLibrary* library, const bool copyEnabled, QWidget* parent) :
	Dialog(parent),
	m {Pimpl::make<Private>(library, this)},
	ui {std::make_shared<Ui::GUI_ImportDialog>()}
{
	ui->setupUi(this);

	connect(m->libraryImporter, &Library::Importer::sigStatusChanged, this, &GUI_ImportDialog::setStatus);
	connect(m->libraryImporter, &Library::Importer::sigProgress, this, &GUI_ImportDialog::setProgress);
	connect(m->libraryImporter, &Library::Importer::sigCachedFilesChanged, this, &GUI_ImportDialog::cachedFilesChanged);

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

GUI_ImportDialog::~GUI_ImportDialog() = default;

void GUI_ImportDialog::setTargetDirectory(QString targetDirectory)
{
	targetDirectory.remove(m->libraryPath + "/");;
	ui->leDirectory->setText(targetDirectory);
}

void GUI_ImportDialog::setMetadata(const MetaDataList& tracks)
{
	if(!tracks.isEmpty())
	{
		const auto numTracksFound = Lang::getWithNumber(Lang::NrTracksFound, tracks.count());
		ui->labStatus->setText(numTracksFound);
	}

	m->tagEditor->setMetadata(tracks);
	ui->btnEdit->setVisible(!tracks.isEmpty());
}

void GUI_ImportDialog::setStatus(const Library::Importer::ImportStatus status)
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
			const auto tracks = m->libraryImporter->cachedTracks();
			ui->labStatus->setText(Lang::getWithNumber(Lang::NrTracksFound, tracks.count()));
			setMetadata(tracks);
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
	if(val >= 100) // NOLINT(readability-magic-numbers)
	{
		val = 0;
	}

	ui->pbProgress->setValue(val);
	emit sigProgress(val);
}

void GUI_ImportDialog::cachedFilesChanged()
{
	const auto count = m->libraryImporter->cachedTracks().count();
	const auto text = Lang::getWithNumber(Lang::NrTracksFound, count);

	ui->labStatus->setText(text);
}

QAbstractButton* GUI_ImportDialog::btnOk() { return ui->buttonBox->button(QDialogButtonBox::Ok); }

QAbstractButton* GUI_ImportDialog::btnCancel() { return ui->buttonBox->button(QDialogButtonBox::Cancel); }

void GUI_ImportDialog::accept()
{
	const auto targetDirectory = ui->leDirectory->text();
	m->libraryImporter->copy(targetDirectory);

	Gui::Dialog::accept();
}

void GUI_ImportDialog::reject()
{

	m->libraryImporter->reset();

	using Status = Library::Importer::ImportStatus;
	if((m->libraryImporter->status() != Status::Rollback) &&
	   (m->libraryImporter->status() != Status::Importing))
	{
		Gui::Dialog::reject();
	}
}

void GUI_ImportDialog::chooseDirectory()
{
	auto dir = Gui::DirectoryChooser::getDirectory(tr("Choose target directory"), m->libraryPath, true, this);
	if(dir.isEmpty())
	{
		ui->leDirectory->clear();
		return;
	}

	if(!dir.contains(m->libraryPath))
	{
		Message::warning(tr("%1<br />is no library directory").arg(dir));
		ui->leDirectory->clear();
		return;
	}

	dir.replace(m->libraryPath, "");
	dir = Util::File::cleanFilename(dir);

	ui->leDirectory->setText(dir);
}

void GUI_ImportDialog::editPressed()
{
	auto* dialog = m->tagEditor->boxIntoDialog();

	connect(m->tagEditor.get(), &GUI_TagEdit::sigCancelled, dialog, &Dialog::reject);
	connect(m->tagEditor.get(), &GUI_TagEdit::sigOkClicked, dialog, &Dialog::accept);

	m->tagEditor->show();

	dialog->exec();
}

void GUI_ImportDialog::showEvent(QShowEvent* e)
{
	Dialog::showEvent(e);
	ui->labTargetPath->setText(m->libraryPath);
	setStatus(m->libraryImporter->status());
}
