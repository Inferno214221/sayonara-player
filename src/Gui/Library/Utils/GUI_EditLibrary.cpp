/* GUI_EditLibrary.cpp */

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

#include "GUI_EditLibrary.h"
#include "Gui/Utils/ui_GUI_EditLibrary.h"
#include "Gui/Utils/Widgets/DirectoryChooser.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"

#include <QSizePolicy>
#include <QStringList>

struct GUI_EditLibrary::Private
{
	QString oldName;
	QString oldPath;

	EditMode editMode {EditMode::New};
	bool nameEdited {false};
};

GUI_EditLibrary::GUI_EditLibrary(QWidget* parent) :
	Dialog(parent),
	ui(std::make_shared<Ui::GUI_EditLibrary>())
{
	ui->setupUi(this);

	m = Pimpl::make<Private>();

	ui->btnChooseDir->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	ui->lePath->setFocus();

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_EditLibrary::okClicked);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_EditLibrary::cancelClicked);
	connect(ui->btnChooseDir, &QPushButton::clicked, this, &GUI_EditLibrary::chooseDirClicked);
	connect(ui->leName, &QLineEdit::textEdited, this, &GUI_EditLibrary::nameEdited);
}

GUI_EditLibrary::GUI_EditLibrary(const QString& name, const QString& path, QWidget* parent) :
	GUI_EditLibrary(parent)
{
	m->editMode = EditMode::Edit;
	m->nameEdited = true;

	m->oldName = name;
	m->oldPath = path;

	ui->leName->setText(name);
	ui->lePath->setText(path);
	ui->labTitle->setText(Lang::get(Lang::Edit));

	this->setWindowTitle(ui->labTitle->text());
	this->setAttribute(Qt::WA_DeleteOnClose);
}

GUI_EditLibrary::~GUI_EditLibrary() = default;

void GUI_EditLibrary::okClicked()
{
	close();
	emit sigAccepted();
}

void GUI_EditLibrary::cancelClicked()
{
	ui->lePath->clear();
	ui->leName->clear();
	close();

	emit sigRejected();
}

void GUI_EditLibrary::chooseDirClicked()
{
	const auto oldDir = (m->oldPath.isEmpty())
	                    ? QDir::homePath()
	                    : m->oldPath;

	auto newDir = Gui::DirectoryChooser::getDirectory(Lang::get(Lang::OpenDir), oldDir, true, this);
	if(newDir.isEmpty())
	{
		newDir = m->oldPath;
	}

	if(m->editMode == EditMode::New)
	{
		if(!m->nameEdited)
		{
			const auto pureFilename = Util::File::getFilenameOfPath(newDir);
			ui->leName->setText(pureFilename);
		}
	}

	ui->lePath->setText(newDir);
}

void GUI_EditLibrary::nameEdited(const QString& text)
{
	m->nameEdited = (text.size() > 0);
}

QString GUI_EditLibrary::name() const
{
	return ui->leName->text();
}

QString GUI_EditLibrary::path() const
{
	return ui->lePath->text();
}

bool GUI_EditLibrary::hasNameChanged() const
{
	return (name() != m->oldName);
}

bool GUI_EditLibrary::hasPathChanged() const
{
	return (path() != m->oldPath);
}

GUI_EditLibrary::EditMode GUI_EditLibrary::editMode() const
{
	return m->editMode;
}

void GUI_EditLibrary::reset()
{
	ui->leName->setText(QString());
	ui->lePath->setText(QString());

	m->oldName = QString();
	m->oldPath = QString();
	m->editMode = EditMode::New;
	m->nameEdited = false;
}

void GUI_EditLibrary::languageChanged()
{
	Dialog::languageChanged();

	ui->labPath->setText(Lang::get(Lang::Directory));
	ui->labName->setText(Lang::get(Lang::Name));

	const auto text = (m->editMode == EditMode::New)
	                  ? Lang::get(Lang::New)
	                  : Lang::get(Lang::Edit);
	ui->labTitle->setText(text);

	setWindowTitle(ui->labTitle->text());
}
