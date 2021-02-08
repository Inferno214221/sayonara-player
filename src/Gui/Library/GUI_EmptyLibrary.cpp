/* GUI_EmptyLibrary.cpp */

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

#include "GUI_EmptyLibrary.h"
#include "Gui/Library/Utils/GUI_EditLibrary.h"

#include "Components/LibraryManagement/LibraryManager.h"

#include "Gui/Utils/ui_GUI_EmptyLibrary.h"

#include "Utils/Library/LibraryInfo.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"

#include <QFileDialog>

using namespace Library;

struct GUI_EmptyLibrary::Private
{
	Library::Manager* libraryManager;
	Private(Library::Manager* libraryManager) :
		libraryManager{libraryManager}
	{}
};

GUI_EmptyLibrary::GUI_EmptyLibrary(Library::Manager* libraryManager, QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>(libraryManager);

	ui = new Ui::GUI_EmptyLibrary();
	ui->setupUi(this);

	bool ok = checkName() && checkPath();

	ui->btnOk->setEnabled(ok);
	ui->labError->setVisible(ok);

	connect(ui->leName, &QLineEdit::textChanged, this, &GUI_EmptyLibrary::nameChanged);
	connect(ui->lePath, &QLineEdit::textChanged, this, &GUI_EmptyLibrary::pathChanged);
	connect(ui->btnChooseDir, &QPushButton::clicked, this, &GUI_EmptyLibrary::chooseDirClicked);
	connect(ui->btnOk, &QPushButton::clicked, this, &GUI_EmptyLibrary::okClicked);
}

GUI_EmptyLibrary::~GUI_EmptyLibrary() = default;

QFrame* GUI_EmptyLibrary::headerFrame() const
{
	return ui->headerFrame;
}

void GUI_EmptyLibrary::okClicked()
{
	QString name = ui->leName->text();
	QString path = ui->lePath->text();

	m->libraryManager->addLibrary(name, path);
}

void GUI_EmptyLibrary::chooseDirClicked()
{
	static QString oldDir = QDir::homePath();

	QString newDir = QFileDialog::getExistingDirectory(this,
	                                                   Lang::get(Lang::Directory),
	                                                   oldDir,
	                                                   QFileDialog::ShowDirsOnly);

	if(newDir.isEmpty())
	{
		return;
	}

	oldDir = newDir;
	ui->lePath->setText(newDir);
}

bool GUI_EmptyLibrary::checkName()
{
	const auto name = ui->leName->text();
	if(name.isEmpty())
	{
		ui->labError->setText(tr("Please choose a name for your library"));
		return false;
	}

	const auto infos = m->libraryManager->allLibraries();
	const auto contains = Util::Algorithm::contains(infos, [&name](const Library::Info& info) {
		return (name.toLower() == info.name().toLower());
	});

	if(contains)
	{
		ui->labError->setText(tr("Please choose another name for your library"));
		return false;
	}

	return true;
}

void GUI_EmptyLibrary::nameChanged(const QString& str)
{
	Q_UNUSED(str)

	bool ok = checkPath() && checkName();

	ui->btnOk->setEnabled(ok);
	ui->labError->setVisible(!ok);
}

bool GUI_EmptyLibrary::checkPath()
{
	const auto path = ui->lePath->text();

	if(!Util::File::exists(path))
	{
		ui->labError->setText(tr("The file path is invalid"));
		return false;
	}

	Library::Info info = m->libraryManager->libraryInfoByPath(path);
	if(Util::File::isSamePath(info.path(), path))
	{
		ui->labError->setText(tr("A library with the same file path already exists"));
	}

	else if(Util::File::isSubdir(path, info.path()))
	{
		ui->labError->setText
			(
				tr("A library which contains this file path already exists") + ":<br>" +
				"<b>" + info.name() + "</b> (" + info.path() + ")"
			);
	}

	return (!m->libraryManager->libraryInfoByPath(path).valid());
}

void GUI_EmptyLibrary::pathChanged(const QString& newPath)
{
	Q_UNUSED(newPath)

	const auto path = ui->lePath->text();
	const auto name = Manager::requestLibraryName(path);
	ui->leName->setText(name);

	const auto ok = checkPath() && checkName();

	ui->btnOk->setEnabled(ok);
	ui->labError->setVisible(!ok);
}

void Library::GUI_EmptyLibrary::languageChanged()
{
	ui->retranslateUi(this);
	ui->labPath->setText(Lang::get(Lang::Directory));
	ui->labName->setText(Lang::get(Lang::Name));
	ui->btnOk->setText(Lang::get(Lang::OK) + "!");
	ui->labTitle->setText(Lang::get(Lang::CreateNewLibrary));
}
