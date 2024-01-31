/* GUI_LibraryPreferences.cpp */

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

#include "GUI_LibraryPreferences.h"
#include "LibraryListModel.h"
#include "Gui/Preferences/ui_GUI_LibraryPreferences.h"

#include "Gui/Library/Utils/GUI_EditLibrary.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Settings/Settings.h"

#include <QItemSelectionModel>
#include <QShowEvent>

namespace
{
	MetaDataSorting::SortModeMask updateSortModeMask(QCheckBox* cb, const MetaDataSorting::SortMode sortMode,
	                                                 MetaDataSorting::SortModeMask sortModeMask)
	{
		if(cb->isChecked())
		{
			sortModeMask |= +sortMode;
		}

		return sortModeMask;
	}

}

struct GUI_LibraryPreferences::Private
{
	Library::Manager* libraryManager;
	LibraryListModel* model = nullptr;

	explicit Private(Library::Manager* libraryManager) :
		libraryManager {libraryManager} {}
};

GUI_LibraryPreferences::GUI_LibraryPreferences(Library::Manager* libraryManager, const QString& identifier) :
	Preferences::Base(identifier),
	m {Pimpl::make<Private>(libraryManager)} {}

GUI_LibraryPreferences::~GUI_LibraryPreferences() = default;

void GUI_LibraryPreferences::initUi()
{
	ui = std::make_shared<Ui::GUI_LibraryPreferences>();
	ui->setupUi(this);

	m->model = new LibraryListModel(m->libraryManager, ui->lvLibs);

	ui->lvLibs->setModel(m->model);
	ui->lvLibs->setItemDelegate(new Gui::StyledItemDelegate(ui->lvLibs));
	ui->tab_widget->setCurrentIndex(0);

	auto* selectionModel = ui->lvLibs->selectionModel();
	connect(selectionModel, &QItemSelectionModel::currentChanged, this, &GUI_LibraryPreferences::selectedIndexChanged);

	connect(ui->btnNew, &QPushButton::clicked, this, &GUI_LibraryPreferences::newClicked);
	connect(ui->btnEdit, &QPushButton::clicked, this, &GUI_LibraryPreferences::editClicked);
	connect(ui->btnDelete, &QPushButton::clicked, this, &GUI_LibraryPreferences::deleteClicked);
	connect(ui->btnUp, &QPushButton::clicked, this, &GUI_LibraryPreferences::upClicked);
	connect(ui->btnDown, &QPushButton::clicked, this, &GUI_LibraryPreferences::downClicked);

	selectedIndexChanged(ui->lvLibs->currentIndex());
}

QString GUI_LibraryPreferences::actionName() const
{
	return Lang::get(Lang::Library);
}

bool GUI_LibraryPreferences::commit()
{
	SetSetting(Set::Lib_DC_DoNothing, ui->rbDcDoNothing->isChecked());
	SetSetting(Set::Lib_DC_PlayIfStopped, ui->rbDcPlayIfStopped->isChecked());
	SetSetting(Set::Lib_DC_PlayImmediately, ui->rbDcPlayImmediatly->isChecked());
	SetSetting(Set::Lib_DD_DoNothing, ui->rbDdDoNothing->isChecked());
	SetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty, ui->rbDdStartIfStopped->isChecked());
	SetSetting(Set::Lib_UseViewClearButton, ui->cbShowClearSelectionButton->isChecked());
	SetSetting(Set::Lib_SortIgnoreArtistArticle, ui->cbIgnoreArticle->isChecked());

	using MetaDataSortMode = MetaDataSorting::SortMode;
	auto sortModeMask = +MetaDataSorting::SortMode::None;
	sortModeMask = updateSortModeMask(ui->cbIgnoreArticle, MetaDataSortMode::IgnoreArticle, sortModeMask);
	sortModeMask = updateSortModeMask(ui->cbIgnoreSpecialChars, MetaDataSortMode::IgnoreSpecialChars, sortModeMask);
	sortModeMask = updateSortModeMask(ui->cbIgnoreAccents, MetaDataSortMode::IgnoreDiacryticChars, sortModeMask);
	sortModeMask = updateSortModeMask(ui->cbCaseInsensitive, MetaDataSortMode::CaseInsensitive, sortModeMask);

	SetSetting(Set::Lib_SortModeMask, sortModeMask);

	return m->model->commit();
}

void GUI_LibraryPreferences::revert()
{
	ui->rbDcDoNothing->setChecked(GetSetting(Set::Lib_DC_DoNothing));
	ui->rbDcPlayIfStopped->setChecked(GetSetting(Set::Lib_DC_PlayIfStopped));
	ui->rbDcPlayImmediatly->setChecked(GetSetting(Set::Lib_DC_PlayImmediately));
	ui->rbDdDoNothing->setChecked(GetSetting(Set::Lib_DD_DoNothing));
	ui->rbDdStartIfStopped->setChecked(GetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty));
	ui->cbShowClearSelectionButton->setChecked(GetSetting(Set::Lib_UseViewClearButton));

	using MetaDataSortMode = MetaDataSorting::SortMode;

	const auto sortModeMask = GetSetting(Set::Lib_SortModeMask);
	const auto ignoreArticle = sortModeMask & +MetaDataSortMode::IgnoreArticle;
	const auto ignoreAccents = sortModeMask & +MetaDataSortMode::IgnoreDiacryticChars;
	const auto caseInsensitive = sortModeMask & +MetaDataSortMode::CaseInsensitive;
	const auto ignoreSpecialChars = sortModeMask & +MetaDataSortMode::IgnoreSpecialChars;

	ui->cbIgnoreArticle->setChecked(ignoreArticle);
	ui->cbCaseInsensitive->setChecked(caseInsensitive);
	ui->cbIgnoreAccents->setChecked(ignoreAccents);
	ui->cbIgnoreSpecialChars->setChecked(ignoreSpecialChars);

	m->model->reset();
}

void GUI_LibraryPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->btnNew->setText(Lang::get(Lang::New));
	ui->btnEdit->setText(Lang::get(Lang::Edit));
	ui->btnDelete->setText(Lang::get(Lang::Remove));
	ui->btnDown->setText(Lang::get(Lang::MoveDown));
	ui->btnUp->setText(Lang::get(Lang::MoveUp));

	ui->cbCaseInsensitive->setText(Lang::get(Lang::CaseInsensitive));
	ui->cbIgnoreAccents->setText(Lang::get(Lang::IgnoreAccents));
	ui->cbIgnoreSpecialChars->setText(Lang::get(Lang::IgnoreSpecialChars));
}

void GUI_LibraryPreferences::skinChanged()
{
	if(ui)
	{
		ui->btnNew->setIcon(Gui::Icons::icon(Gui::Icons::New));
		ui->btnEdit->setIcon(Gui::Icons::icon(Gui::Icons::Edit));
		ui->btnDelete->setIcon(Gui::Icons::icon(Gui::Icons::Remove));
	}
}

void GUI_LibraryPreferences::showEvent(QShowEvent* e)
{
	Base::showEvent(e);
	revert();
}

QString GUI_LibraryPreferences::errorString() const
{
	return tr("Cannot edit library");
}

void GUI_LibraryPreferences::newClicked()
{
	auto* editDialog = new GUI_EditLibrary(this);
	connect(editDialog, &GUI_EditLibrary::sigAccepted, this, &GUI_LibraryPreferences::editDialogAccepted);
	editDialog->show();
}

void GUI_LibraryPreferences::editClicked()
{
	const auto modelIndex = ui->lvLibs->currentIndex();
	if(modelIndex.isValid())
	{
		const auto currentRow = modelIndex.row();
		const auto name = m->model->name(currentRow);
		const auto path = m->model->path(currentRow);

		auto* editDialog = new GUI_EditLibrary(name, path, this);
		connect(editDialog, &GUI_EditLibrary::sigAccepted, this, &GUI_LibraryPreferences::editDialogAccepted);
		editDialog->show();
	}
}

void GUI_LibraryPreferences::deleteClicked()
{
	const auto modelIndex = ui->lvLibs->currentIndex();
	if(modelIndex.isValid())
	{
		m->model->removeRow(modelIndex.row());
	}
}

void GUI_LibraryPreferences::upClicked()
{
	const auto currentRow = ui->lvLibs->currentIndex().row();
	m->model->moveRow(currentRow, currentRow - 1);
	ui->lvLibs->setCurrentIndex(m->model->index(currentRow - 1));
}

void GUI_LibraryPreferences::downClicked()
{
	const auto currentRow = ui->lvLibs->currentIndex().row();
	m->model->moveRow(currentRow, currentRow + 1);
	ui->lvLibs->setCurrentIndex(m->model->index(currentRow + 1));
}

void GUI_LibraryPreferences::editDialogAccepted()
{
	auto* editDialog = dynamic_cast<GUI_EditLibrary*>(sender());

	const auto editMode = editDialog->editMode();
	const auto name = editDialog->name();
	const auto path = editDialog->path();
	const auto currentRow = ui->lvLibs->currentIndex().row();

	switch(editMode)
	{
		case GUI_EditLibrary::EditMode::New:
			if(!name.isEmpty() && !path.isEmpty())
			{
				m->model->appendRow(name, path);
			}
			break;

		case GUI_EditLibrary::EditMode::Edit:
			if(!name.isEmpty() && editDialog->hasNameChanged())
			{
				m->model->renameRow(currentRow, name);
			}

			if(!path.isEmpty() && editDialog->hasPathChanged())
			{
				m->model->changePath(currentRow, path);
			}
			break;

		default:
			break;
	}

	editDialog->deleteLater();
}

void GUI_LibraryPreferences::selectedIndexChanged(const QModelIndex& modelIndex)
{
	const auto currentRow = modelIndex.row();
	const auto rowCount = ui->lvLibs->model()->rowCount();
	const auto isNotFirstRow = (currentRow > 0) && (currentRow < rowCount);
	const auto isNotLastRow = (currentRow >= 0) && (currentRow < (rowCount - 1));
	const auto isValidRow = (currentRow >= 0) && (currentRow < rowCount);

	ui->btnUp->setEnabled(isNotFirstRow);
	ui->btnDown->setEnabled(isNotLastRow);
	ui->btnDelete->setEnabled(isValidRow);
	ui->btnEdit->setEnabled(isValidRow);
	ui->labCurrentPath->setVisible(isValidRow);

	if(isValidRow)
	{
		const auto path = m->model->path(currentRow);
		ui->labCurrentPath->setText(path);
	}
}
