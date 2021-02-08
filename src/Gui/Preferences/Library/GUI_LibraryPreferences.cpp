/* GUI_LibraryPreferences.cpp */

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

#include "GUI_LibraryPreferences.h"
#include "LibraryListModel.h"
#include "Gui/Preferences/ui_GUI_LibraryPreferences.h"

#include "Gui/Library/Utils/GUI_EditLibrary.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include "Utils/Library/SearchMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QShowEvent>
#include <QItemSelectionModel>

struct GUI_LibraryPreferences::Private
{
	Library::Manager* libraryManager;
	LibraryListModel* model = nullptr;

	Private(Library::Manager* libraryManager) :
		libraryManager{libraryManager}
	{}
};

GUI_LibraryPreferences::GUI_LibraryPreferences(Library::Manager* libraryManager, const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>(libraryManager);
}

GUI_LibraryPreferences::~GUI_LibraryPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_LibraryPreferences::initUi()
{
	setupParent(this, &ui);

	m->model = new LibraryListModel(m->libraryManager, ui->lvLibs);

	ui->lvLibs->setModel(m->model);
	ui->lvLibs->setItemDelegate(
				new Gui::StyledItemDelegate(ui->lvLibs)
	);

	ui->tab_widget->setCurrentIndex(0);

	QItemSelectionModel* sel_model = ui->lvLibs->selectionModel();
	connect(sel_model, &QItemSelectionModel::currentChanged, this, &GUI_LibraryPreferences::selectedIndexChanged);

	connect(ui->btnNew, &QPushButton::clicked, this, &GUI_LibraryPreferences::newClicked);
	connect(ui->btnEdit, &QPushButton::clicked, this, &GUI_LibraryPreferences::editClicked);
	connect(ui->btnDelete, &QPushButton::clicked, this, &GUI_LibraryPreferences::deleteClicked);
	connect(ui->btnUp, &QPushButton::clicked, this, &GUI_LibraryPreferences::upClicked);
	connect(ui->btnDown, &QPushButton::clicked, this, &GUI_LibraryPreferences::downClicked);

	revert();

	selectedIndexChanged(m->model->index(currentRow()));
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
	ui->cbIgnoreArticle->setChecked(GetSetting(Set::Lib_SortIgnoreArtistArticle));

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
}

void GUI_LibraryPreferences::skinChanged()
{
	if(!ui){
		return;
	}

	ui->btnNew->setIcon(Gui::Icons::icon(Gui::Icons::New));
	ui->btnEdit->setIcon(Gui::Icons::icon(Gui::Icons::Edit));
	ui->btnDelete->setIcon(Gui::Icons::icon(Gui::Icons::Remove));
}

void GUI_LibraryPreferences::showEvent(QShowEvent* e)
{
	Base::showEvent(e);
	this->revert();
}


QString GUI_LibraryPreferences::errorString() const
{
	return tr("Cannot edit library");
}


int GUI_LibraryPreferences::currentRow() const
{
	return ui->lvLibs->selectionModel()->currentIndex().row();
}


void GUI_LibraryPreferences::newClicked()
{
	GUI_EditLibrary* edit_dialog = new GUI_EditLibrary(this);

	connect(edit_dialog, &GUI_EditLibrary::sigAccepted, this, &GUI_LibraryPreferences::editDialogAccepted);

	edit_dialog->show();
}

void GUI_LibraryPreferences::editClicked()
{
	int cur_row = currentRow();
	if(cur_row < 0){
		return;
	}

	QString name = m->model->name(cur_row);
	QString path = m->model->path(cur_row);

	GUI_EditLibrary* edit_dialog = new GUI_EditLibrary(name, path, this);

	connect(edit_dialog, &GUI_EditLibrary::sigAccepted, this, &GUI_LibraryPreferences::editDialogAccepted);

	edit_dialog->show();
}

void GUI_LibraryPreferences::deleteClicked()
{
	QModelIndex idx = ui->lvLibs->currentIndex();
	if(!idx.isValid()){
		return;
	}

	m->model->removeRow(idx.row());
}


void GUI_LibraryPreferences::upClicked()
{
	int row = ui->lvLibs->currentIndex().row();

	m->model->moveRow(row, row-1);
	ui->lvLibs->setCurrentIndex(m->model->index(row - 1));
}

void GUI_LibraryPreferences::downClicked()
{
	int row = ui->lvLibs->currentIndex().row();

	m->model->moveRow(row, row+1);
	ui->lvLibs->setCurrentIndex(m->model->index(row + 1));
}


void GUI_LibraryPreferences::editDialogAccepted()
{
	auto* edit_dialog = static_cast<GUI_EditLibrary*>(sender());

	GUI_EditLibrary::EditMode edit_mode = edit_dialog->editMode();

	QString name = edit_dialog->name();
	QString path = edit_dialog->path();

	switch(edit_mode)
	{
	case GUI_EditLibrary::EditMode::New:
	{
		if(!name.isEmpty() && !path.isEmpty()) {
			m->model->appendRow(name, path);
		}

	} break;

	case GUI_EditLibrary::EditMode::Edit:
	{
		if(!name.isEmpty()) {
			if(edit_dialog->hasNameChanged()){
				m->model->renameRow(currentRow(), name);
			}
		}

		if(!path.isEmpty()) {
			if(edit_dialog->hasPathChanged())	{
				m->model->changePath(currentRow(), path);
			}
		}

	} break;

	default:
		break;
	}

	edit_dialog->deleteLater();
}

void GUI_LibraryPreferences::selectedIndexChanged(const QModelIndex& idx)
{
	int curentRow = idx.row();
	int rowCount = ui->lvLibs->model()->rowCount();

	ui->btnUp->setDisabled(curentRow <= 0 || curentRow >= rowCount);
	ui->btnDown->setDisabled(curentRow < 0 || curentRow >= rowCount - 1);
	ui->btnDelete->setDisabled(curentRow < 0 || curentRow >= rowCount);
	ui->btnEdit->setDisabled(curentRow < 0 || curentRow >= rowCount);

	ui->labCurrentPath->setVisible(curentRow >= 0 || curentRow < rowCount);
	if(curentRow < 0 || curentRow >= rowCount){
		return;
	}

	QString path = m->model->path(curentRow);
	ui->labCurrentPath->setText(path);
}

