/* GUI_EditLibrary.cpp */

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

#include "GUI_EditLibrary.h"
#include "Gui/Utils/ui_GUI_EditLibrary.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"

#include <QFileDialog>
#include <QSizePolicy>
#include <QStringList>

struct GUI_EditLibrary::Private
{
	QString oldName;
	QString oldPath;

	EditMode editMode;
	bool nameEdited;

	Private() :
		editMode(EditMode::New),
		nameEdited(false)
	{}
};

GUI_EditLibrary::GUI_EditLibrary(QWidget* parent) :
	Dialog (parent),
	ui(new Ui::GUI_EditLibrary)
{
	ui->setupUi(this);

	m = Pimpl::make<Private>();

	ui->btnChooseDir->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	ui->lePath->setFocus();

	connect(ui->btnOk, &QPushButton::clicked, this, &GUI_EditLibrary::okClicked);
	connect(ui->btnCancel, &QPushButton::clicked, this, &GUI_EditLibrary::cancelClicked);
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


GUI_EditLibrary::~GUI_EditLibrary()
{
	delete ui; ui = nullptr;
}

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
	QString oldDir = m->oldPath;
	if(oldDir.isEmpty()){
		oldDir = QDir::homePath();
	}

	QString newDir =
		QFileDialog::getExistingDirectory(this,
			Lang::get(Lang::Directory),
			oldDir,
			QFileDialog::ShowDirsOnly
		);

	if(newDir.isEmpty()) {
		newDir = m->oldPath;
	}

	if(m->editMode == EditMode::New)
	{
		QString str = Util::File::getFilenameOfPath(newDir);

		if(!m->nameEdited)
		{
			ui->leName->setText(str);
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

	ui->btnOk->setText(Lang::get(Lang::OK));
	ui->btnCancel->setText(Lang::get(Lang::Cancel));
	ui->labPath->setText(Lang::get(Lang::Directory));
	ui->labName->setText(Lang::get(Lang::Name));

	if(m->editMode == EditMode::New) {
		ui->labTitle->setText(Lang::get(Lang::New));
	} else {
		ui->labTitle->setText(Lang::get(Lang::Edit));
	}

	this->setWindowTitle(ui->labTitle->text());
}

void GUI_EditLibrary::skinChanged()
{
	Dialog::skinChanged();
}
