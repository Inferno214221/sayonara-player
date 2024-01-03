/* GUI_CoverPreferences.cpp */

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

#include "Gui/Preferences/ui_GUI_CoverPreferences.h"
#include "GUI_CoverPreferences.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/Fetcher/CoverFetcher.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/StandardPaths.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include <QListWidgetItem>
#include <QList>

using namespace Cover;

namespace
{
	bool checkCoverTemplate(const QString& coverTemplate)
	{
		if(coverTemplate.trimmed().isEmpty())
		{
			return false;
		}

		auto coverTemplateCopy(coverTemplate);
		coverTemplateCopy.remove("<h>");

		const auto invalidChars = QList<QChar>
			{
				'/', '\\', '|', ':', '\"', '?', '$', '<', '>', '*', '#', '%', '&'
			};

		const auto contains = Util::Algorithm::contains(invalidChars, [&](const auto& c) {
			return (coverTemplateCopy.contains(c));
		});

		return (!contains);
	}
}

GUI_CoverPreferences::GUI_CoverPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_CoverPreferences::~GUI_CoverPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

bool GUI_CoverPreferences::commit()
{
	QStringList activeItems;

	for(auto i = 0; i < ui->lvCoverSearchers->count(); i++)
	{
		auto* listWidgetItem = ui->lvCoverSearchers->item(i);
		activeItems << listWidgetItem->text().toLower();
	}

	SetSetting(Set::Cover_Server, activeItems);
	SetSetting(Set::Cover_FetchFromWWW, ui->cbFetchFromWWW->isChecked());
	SetSetting(Set::Cover_SaveToDB, ui->cbSaveToDatabase->isChecked());
	SetSetting(Set::Cover_SaveToLibrary, ui->cbSaveToLibrary->isChecked() && ui->cbSaveToLibrary->isEnabled());
	SetSetting(Set::Cover_SaveToSayonaraDir,
	           ui->cbSaveToSayonaraDir->isChecked() && ui->cbSaveToSayonaraDir->isEnabled());

	auto coverTemplate = ui->leCoverTemplate->text().trimmed();
	if(checkCoverTemplate(coverTemplate))
	{
		if(!Util::File::isImageFile(coverTemplate))
		{
			const auto extension = Util::File::getFileExtension(coverTemplate);
			if(extension.isEmpty())
			{
				coverTemplate.append(".jpg");
				coverTemplate.replace("..jpg", ".jpg");
			}

			else
			{
				coverTemplate.replace("." + extension, ".jpg");
			}

			ui->leCoverTemplate->setText(coverTemplate);
		}

		SetSetting(Set::Cover_TemplatePath, coverTemplate);
	}

	else
	{
		ui->leCoverTemplate->setText(GetSetting(Set::Cover_TemplatePath));
		ui->labTemplateError->setVisible(false);
	}

	return true;
}

void GUI_CoverPreferences::revert()
{
	auto* coverFetchManager = Cover::Fetcher::Manager::instance();
	const auto coverFetchers = coverFetchManager->coverfetchers();
	const auto coverServers = GetSetting(Set::Cover_Server);

	ui->lvCoverSearchers->clear();
	ui->lvInactiveCoverSearchers->clear();

	for(const auto& coverFetcher: coverFetchers)
	{
		const auto identifier = coverFetcher->identifier();
		if(!identifier.trimmed().isEmpty() && (coverFetcher->isWebserviceFetcher()))
		{
			auto* widget = (coverServers.contains(identifier))
			               ? ui->lvCoverSearchers
			               : ui->lvInactiveCoverSearchers;

			widget->addItem(Util::stringToVeryFirstUpper(identifier));
		}
	}

	ui->cbFetchFromWWW->setChecked(GetSetting(Set::Cover_FetchFromWWW));
	ui->cbSaveToDatabase->setChecked(GetSetting(Set::Cover_SaveToDB));
	ui->cbSaveToSayonaraDir->setChecked(GetSetting(Set::Cover_SaveToSayonaraDir));
	ui->cbSaveToLibrary->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->leCoverTemplate->setText(GetSetting(Set::Cover_TemplatePath));

	fetchCoversFromWWWTriggered(GetSetting(Set::Cover_FetchFromWWW));
	saveCoverToLibraryToggled(GetSetting(Set::Cover_SaveToLibrary));

	currentRowChanged(ui->lvCoverSearchers->currentRow());
}

QString GUI_CoverPreferences::actionName() const
{
	return Lang::get(Lang::Covers);
}

void GUI_CoverPreferences::initUi()
{
	if(ui)
	{
		return;
	}

	setupParent(this, &ui);

	ui->lvCoverSearchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lvCoverSearchers));
	ui->lvInactiveCoverSearchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lvInactiveCoverSearchers));
	ui->labTemplateError->setVisible(false);

	connect(ui->btnUp, &QPushButton::clicked, this, &GUI_CoverPreferences::upClicked);
	connect(ui->btnDown, &QPushButton::clicked, this, &GUI_CoverPreferences::downClicked);
	connect(ui->lvCoverSearchers, &QListWidget::currentRowChanged, this, &GUI_CoverPreferences::currentRowChanged);
	connect(ui->btnDeleteCovers, &QPushButton::clicked, this, &GUI_CoverPreferences::deleteCoversFromDb);
	connect(ui->btnDeleteFiles, &QPushButton::clicked, this, &GUI_CoverPreferences::deleteCoverFiles);
	connect(ui->cbFetchFromWWW, &QCheckBox::toggled, this, &GUI_CoverPreferences::fetchCoversFromWWWTriggered);
	connect(ui->btnAdd, &QPushButton::clicked, this, &GUI_CoverPreferences::addClicked);
	connect(ui->btnRemove, &QPushButton::clicked, this, &GUI_CoverPreferences::removeClicked);
	connect(ui->cbSaveToLibrary, &QCheckBox::toggled, this, &GUI_CoverPreferences::saveCoverToLibraryToggled);
	connect(ui->leCoverTemplate, &QLineEdit::textEdited, this, &GUI_CoverPreferences::coverTemplateEdited);

	ui->cbSaveToSayonaraDir->setToolTip(Util::coverDirectory());

	revert();
}

void GUI_CoverPreferences::retranslate()
{
	ui->retranslateUi(this);
	ui->btnUp->setText(Lang::get(Lang::MoveUp));
	ui->btnDown->setText(Lang::get(Lang::MoveDown));
}

void GUI_CoverPreferences::skinChanged()
{
	if(ui)
	{
		ui->btnDeleteFiles->setIcon(Gui::Icons::icon(Gui::Icons::Delete));
		ui->btnDeleteCovers->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	}
}

void GUI_CoverPreferences::upClicked()
{
	const auto currentRow = ui->lvCoverSearchers->currentRow();

	auto* listWidgetItem = ui->lvCoverSearchers->takeItem(currentRow);
	ui->lvCoverSearchers->insertItem(currentRow - 1, listWidgetItem);
	ui->lvCoverSearchers->setCurrentRow(currentRow - 1);
}

void GUI_CoverPreferences::downClicked()
{
	const auto currentRow = ui->lvCoverSearchers->currentRow();

	auto* listWidgetItem = ui->lvCoverSearchers->takeItem(currentRow);
	ui->lvCoverSearchers->insertItem(currentRow + 1, listWidgetItem);
	ui->lvCoverSearchers->setCurrentRow(currentRow + 1);
}

void GUI_CoverPreferences::addClicked()
{
	auto* listWidgetItem = ui->lvInactiveCoverSearchers->takeItem(ui->lvInactiveCoverSearchers->currentRow());
	if(listWidgetItem)
	{
		ui->lvCoverSearchers->addItem(listWidgetItem->text());
		delete listWidgetItem;
	}
}

void GUI_CoverPreferences::removeClicked()
{
	auto* listWidgetItem = ui->lvCoverSearchers->takeItem(ui->lvCoverSearchers->currentRow());
	if(listWidgetItem)
	{
		ui->lvInactiveCoverSearchers->addItem(listWidgetItem->text());
		delete listWidgetItem;
	}
}

void GUI_CoverPreferences::currentRowChanged(int row)
{
	ui->btnUp->setDisabled(
		(row <= 0) || (row >= ui->lvCoverSearchers->count())
	);
	ui->btnDown->setDisabled(
		(row < 0) || (row >= ui->lvCoverSearchers->count() - 1)
	);
}

void GUI_CoverPreferences::deleteCoversFromDb()
{
	DB::Connector::instance()->coverConnector()->clear();
	Cover::ChangeNotfier::instance()->shout();
}

void GUI_CoverPreferences::deleteCoverFiles()
{
	::Util::File::removeFilesInDirectory(Util::coverDirectory());
}

void GUI_CoverPreferences::fetchCoversFromWWWTriggered(bool b)
{
	ui->lvCoverSearchers->setEnabled(b);
	ui->lvInactiveCoverSearchers->setEnabled(b);
	ui->btnDown->setEnabled(b);
	ui->btnUp->setEnabled(b);
	ui->btnAdd->setEnabled(b);
	ui->btnRemove->setEnabled(b);
	ui->cbSaveToSayonaraDir->setEnabled(b);
	ui->cbSaveToLibrary->setEnabled(b);
	ui->leCoverTemplate->setEnabled(b);
	ui->labCoverTemplate->setEnabled(b);
}

void GUI_CoverPreferences::saveCoverToLibraryToggled(bool b)
{
	ui->leCoverTemplate->setVisible(b);
	ui->labCoverTemplate->setVisible(b);
}

void GUI_CoverPreferences::coverTemplateEdited(const QString& text)
{
	const auto valid = checkCoverTemplate(text);
	ui->labTemplateError->setVisible(!valid);
	ui->labTemplateError->setText(Lang::get(Lang::Error) + ": " + Lang::get(Lang::InvalidChars));
}
