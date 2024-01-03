/* GUI_FileExtensionPreferences.cpp */
/*
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

#include "GUI_FileExtensionPreferences.h"
#include "Gui/Preferences/ui_GUI_FileExtensionPreferences.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include <QRegExp>

#include <memory>

namespace
{
	QStringList splitExtensions(const QString& str)
	{
		auto extensions = str.split(",");
		for(auto& extension: extensions)
		{
			extension = extension.trimmed().toLower();
		}

		return extensions;
	}

	bool checkExtensions(const QStringList& extensions)
	{
		const auto regExp = QRegExp("[a-zA-Z0-9]{1,4}");
		const auto containsIllegal = Util::Algorithm::contains(extensions, [&](const auto& extension) {
			return !regExp.exactMatch(extension);
		});

		return !containsIllegal;
	}

	bool contains(QListWidget* listWidget, const QString& extension)
	{
		for(auto row = 0; row < listWidget->count(); row++)
		{
			auto* item = listWidget->item(row);
			if(item->text().toLower() == extension.toLower())
			{
				return true;
			}
		}

		if(Util::soundfileExtensions(false).contains(extension, Qt::CaseInsensitive))
		{
			return true;
		}

		return false;
	}

	void addExtensionsToList(QListWidget* listWidget, const QStringList& extensions)
	{
		for(const auto& extension: extensions)
		{
			if(!contains(listWidget, extension))
			{
				listWidget->addItem(extension);
			}
		}

		listWidget->sortItems();
	}
}

GUI_FileExtensionPreferences::GUI_FileExtensionPreferences(const QString& identifier) :
	Preferences::Base(identifier) {}

GUI_FileExtensionPreferences::~GUI_FileExtensionPreferences()
{
	delete ui;
}

void GUI_FileExtensionPreferences::initUi()
{
	setupParent(this, &ui);

	connect(ui->btnAdd, &QPushButton::clicked, this, &GUI_FileExtensionPreferences::addClicked);
	connect(ui->btnRemove, &QPushButton::clicked, this, &GUI_FileExtensionPreferences::removeClicked);

	revert();
}

bool GUI_FileExtensionPreferences::commit()
{
	auto extensions = QStringList {};
	for(auto row = 0; row < ui->listWidget->count(); row++)
	{
		extensions << ui->listWidget->item(row)->text();
	}

	SetSetting(Set::Engine_SoundFileExtensions, extensions);

	return true;
}

void GUI_FileExtensionPreferences::revert()
{
	ui->listWidget->clear();

	const auto extensions = GetSetting(Set::Engine_SoundFileExtensions);
	for(const auto& extension: extensions)
	{
		if(!extension.isNull())
		{
			ui->listWidget->addItem(extension);
		}
	}

	ui->btnRemove->setEnabled(ui->listWidget->count() > 0);
}

QString GUI_FileExtensionPreferences::actionName() const
{
	return tr("Additional file extensions");
}

void GUI_FileExtensionPreferences::retranslate()
{
	ui->retranslateUi(this);
	ui->btnAdd->setText(Lang::get(Lang::Add));
	ui->btnRemove->setText(Lang::get(Lang::Remove));
}

void GUI_FileExtensionPreferences::addClicked()
{
	const auto title = tr("Add new file extension");
	auto extensionString = QString();

	while(true)
	{
		auto dialog =
			std::make_shared<Gui::LineInputDialog>(title, title, extensionString, this);
		dialog->setPlaceholderText("abc,xyz");
		dialog->exec();

		if(dialog->returnValue() == Gui::LineInputDialog::Ok)
		{
			extensionString = dialog->text();
			const auto extensions = splitExtensions(extensionString);

			if(const auto isValid = checkExtensions(extensions); !isValid)
			{
				Message::error("Maximal length is 4. Allowed characters are a-z, A-Z and 0-9");
			}

			else
			{
				addExtensionsToList(ui->listWidget, extensions);
				break;
			}
		}

		else
		{
			break;
		}
	}

	ui->btnRemove->setEnabled(ui->listWidget->count() > 0);
}

void GUI_FileExtensionPreferences::removeClicked()
{
	delete ui->listWidget->currentItem();
	ui->btnRemove->setEnabled(ui->listWidget->count() > 0);
}