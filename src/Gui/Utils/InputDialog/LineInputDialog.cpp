/* LineInputDialog.cpp */

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



#include "LineInputDialog.h"
#include "Gui/Utils/Widgets/Completer.h"
#include "Gui/Utils/ui_LineInputDialog.h"

#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"

using Gui::LineInputDialog;
using Gui::Completer;

struct LineInputDialog::Private
{
	Gui::Completer* completer = nullptr;
	QString infoPrefix;
	QList<QChar> invalidChars;
	LineInputDialog::ReturnValue returnValue;
	bool showPrefix;

	Private() :
		returnValue(LineInputDialog::Ok),
		showPrefix(false) {}
};

LineInputDialog::LineInputDialog(const QString& windowTitle, const QString& infoText, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::LineInputDialog();
	ui->setupUi(this);

	this->setWindowTitle(windowTitle);
	this->setHeaderText(infoText);
	//this->setInfoText(infoText);

	m->completer = new Completer(QStringList(), ui->leInput);
	ui->leInput->setCompleter(m->completer);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LineInputDialog::okClicked);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LineInputDialog::cancelClicked);
	connect(ui->leInput, &QLineEdit::textEdited, this, &LineInputDialog::textEdited);
}

LineInputDialog::LineInputDialog(const QString& window_title, const QString& info_text, const QString& preset,
                                 QWidget* parent) :
	LineInputDialog(window_title, info_text, parent)
{
	ui->leInput->setText(preset);
}

LineInputDialog::~LineInputDialog()
{
	delete ui;
	ui = nullptr;
}

QString LineInputDialog::getRenameFilename(QWidget* parent, const QString& oldName, const QString& parentPath)
{
	LineInputDialog dialog(Lang::get(Lang::Rename), Lang::get(Lang::EnterNewName), oldName, parent);
	dialog.setInvalidChars(Util::File::invalidFilenameChars());
	dialog.showInfo(!parentPath.isEmpty(), parentPath + "/");
	dialog.exec();

	return dialog.text();
}

QString LineInputDialog::getNewFilename(QWidget* parent, const QString& info, const QString& parentPath)
{
	LineInputDialog dialog(info, Lang::get(Lang::EnterNewName), parent);
	dialog.setInvalidChars(Util::File::invalidFilenameChars());
	dialog.showInfo(!parentPath.isEmpty(), parentPath + "/");
	dialog.exec();

	return dialog.text();
}

void Gui::LineInputDialog::setHeaderText(const QString& text)
{
	ui->labHeader->setText(text);
}

void Gui::LineInputDialog::setInfoText(const QString& text)
{
	ui->labHeader->setText(text);
}

void LineInputDialog::setCompleterText(const QStringList& lst)
{
	m->completer->setStringList(lst);
}

Gui::LineInputDialog::ReturnValue Gui::LineInputDialog::returnValue() const
{
	return m->returnValue;
}

QString LineInputDialog::text() const
{
	return ui->leInput->text().trimmed();
}

void LineInputDialog::setText(const QString& text)
{
	ui->leInput->setText(text);
}

void LineInputDialog::setPlaceholderText(const QString& text)
{
	ui->leInput->setPlaceholderText(text);
}

void LineInputDialog::showInfo(bool b, const QString& infoPrefix)
{
	m->showPrefix = b;
	m->infoPrefix = infoPrefix;

	ui->labInfo->setVisible(b);
	ui->labInfo->setText(infoPrefix);
}

bool LineInputDialog::wasAccepted() const
{
	return (m->returnValue == LineInputDialog::Ok);
}

void LineInputDialog::setInvalidChars(const QList<QChar>& chars)
{
	m->invalidChars = chars;
}

void LineInputDialog::okClicked()
{
	m->returnValue = LineInputDialog::Ok;
	close();
	emit accepted();
}

void LineInputDialog::cancelClicked()
{
	ui->leInput->clear();
	close();
	emit rejected();
}

void LineInputDialog::textEdited(const QString& text)
{
	QString newText(text);
	for(QChar c: m->invalidChars)
	{
		if(!c.isPrint())
		{
			continue;
		}

		while(newText.contains(c))
		{
			newText.remove(c);
		}
	}

	if(newText != text)
	{
		ui->leInput->setText(newText);
	}

	ui->labInfo->setText(m->infoPrefix + newText);
}

void LineInputDialog::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);

	ui->leInput->setFocus();
	ui->labInfo->setVisible(m->showPrefix);
	ui->labInfo->setText(m->infoPrefix + ui->leInput->text());

	m->returnValue = LineInputDialog::Cancelled;
}

void LineInputDialog::closeEvent(QCloseEvent* e)
{
	Gui::Dialog::closeEvent(e);
}

