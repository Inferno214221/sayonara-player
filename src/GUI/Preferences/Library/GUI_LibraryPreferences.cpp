/* GUI_LibraryPreferences.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "GUI/Preferences/ui_GUI_LibraryPreferences.h"
#include "GUI_LibraryPreferences.h"
#include "LibraryListModel.h"

#include "GUI/Utils/Delegates/StyledItemDelegate.h"
#include "GUI/Utils/Library/GUI_EditLibrary.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QShowEvent>

#include <QItemSelectionModel>

struct GUI_LibraryPreferences::Private
{
	LibraryListModel* model = nullptr;
};

GUI_LibraryPreferences::GUI_LibraryPreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_LibraryPreferences::~GUI_LibraryPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_LibraryPreferences::init_ui()
{
	setup_parent(this, &ui);

	m->model = new LibraryListModel(ui->lv_libs);

	ui->lv_libs->setModel(m->model);
	ui->lv_libs->setItemDelegate(
				new Gui::StyledItemDelegate(ui->lv_libs)
	);

	ui->tab_widget->setCurrentIndex(0);

	QItemSelectionModel* sel_model = ui->lv_libs->selectionModel();
	connect(sel_model, &QItemSelectionModel::currentChanged, this, &GUI_LibraryPreferences::current_index_changed);

	connect(ui->btn_new, &QPushButton::clicked, this, &GUI_LibraryPreferences::new_clicked);
	connect(ui->btn_edit, &QPushButton::clicked, this, &GUI_LibraryPreferences::edit_clicked);
	connect(ui->btn_delete, &QPushButton::clicked, this, &GUI_LibraryPreferences::delete_clicked);
	connect(ui->btn_up, &QPushButton::clicked, this, &GUI_LibraryPreferences::up_clicked);
	connect(ui->btn_down, &QPushButton::clicked, this, &GUI_LibraryPreferences::down_clicked);

	revert();

	current_index_changed(m->model->index(current_row()));
}

QString GUI_LibraryPreferences::action_name() const
{
	return Lang::get(Lang::Library);
}


bool GUI_LibraryPreferences::commit()
{
	SetSetting(Set::Lib_DC_DoNothing, ui->rb_dc_do_nothing->isChecked());
	SetSetting(Set::Lib_DC_PlayIfStopped, ui->rb_dc_play_if_stopped->isChecked());
	SetSetting(Set::Lib_DC_PlayImmediately, ui->rb_dc_play_immediately->isChecked());
	SetSetting(Set::Lib_DD_DoNothing, ui->rb_dd_do_nothing->isChecked());
	SetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty, ui->rb_dd_start_if_stopped_and_empty->isChecked());
	SetSetting(Set::Lib_UseViewClearButton, ui->cb_show_clear_buttons->isChecked());

	return m->model->commit();
}

void GUI_LibraryPreferences::revert()
{
	ui->rb_dc_do_nothing->setChecked(GetSetting(Set::Lib_DC_DoNothing));
	ui->rb_dc_play_if_stopped->setChecked(GetSetting(Set::Lib_DC_PlayIfStopped));
	ui->rb_dc_play_immediately->setChecked(GetSetting(Set::Lib_DC_PlayImmediately));
	ui->rb_dd_do_nothing->setChecked(GetSetting(Set::Lib_DD_DoNothing));
	ui->rb_dd_start_if_stopped_and_empty->setChecked(GetSetting(Set::Lib_DD_PlayIfStoppedAndEmpty));
	ui->cb_show_clear_buttons->setChecked(GetSetting(Set::Lib_UseViewClearButton));

	m->model->reset();
}

void GUI_LibraryPreferences::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->btn_new->setText(Lang::get(Lang::New));
	ui->btn_edit->setText(Lang::get(Lang::Edit));
	ui->btn_delete->setText(Lang::get(Lang::Remove));
	ui->btn_down->setText(Lang::get(Lang::MoveDown));
	ui->btn_up->setText(Lang::get(Lang::MoveUp));
}

void GUI_LibraryPreferences::showEvent(QShowEvent* e)
{
	Base::showEvent(e);
	this->revert();
}


QString GUI_LibraryPreferences::error_string() const
{
	return tr("Cannot edit library");
}


int GUI_LibraryPreferences::current_row() const
{
	return ui->lv_libs->selectionModel()->currentIndex().row();
}


void GUI_LibraryPreferences::new_clicked()
{
	GUI_EditLibrary* edit_dialog = new GUI_EditLibrary(this);

	connect(edit_dialog, &GUI_EditLibrary::sig_accepted, this, &GUI_LibraryPreferences::edit_dialog_accepted);

	edit_dialog->show();
}

void GUI_LibraryPreferences::edit_clicked()
{
	int cur_row = current_row();
	if(cur_row < 0){
		return;
	}

	QString name = m->model->name(cur_row);
	QString path = m->model->path(cur_row);

	GUI_EditLibrary* edit_dialog = new GUI_EditLibrary(name, path, this);

	connect(edit_dialog, &GUI_EditLibrary::sig_accepted, this, &GUI_LibraryPreferences::edit_dialog_accepted);

	edit_dialog->show();
}

void GUI_LibraryPreferences::delete_clicked()
{
	QModelIndex idx = ui->lv_libs->currentIndex();
	if(!idx.isValid()){
		return;
	}

	m->model->remove_row(idx.row());
}


void GUI_LibraryPreferences::up_clicked()
{
	int row = ui->lv_libs->currentIndex().row();

	m->model->move_row(row, row-1);
	ui->lv_libs->setCurrentIndex(m->model->index(row - 1));
}

void GUI_LibraryPreferences::down_clicked()
{
	int row = ui->lv_libs->currentIndex().row();

	m->model->move_row(row, row+1);
	ui->lv_libs->setCurrentIndex(m->model->index(row + 1));
}


void GUI_LibraryPreferences::edit_dialog_accepted()
{
	GUI_EditLibrary* edit_dialog = static_cast<GUI_EditLibrary*>(sender());

	GUI_EditLibrary::EditMode edit_mode = edit_dialog->edit_mode();

	QString name = edit_dialog->name();
	QString path = edit_dialog->path();

	switch(edit_mode)
	{
	case GUI_EditLibrary::EditMode::New:
	{
		if(!name.isEmpty() && !path.isEmpty()) {
			m->model->append_row(name, path);
		}

	} break;

	case GUI_EditLibrary::EditMode::Edit:
	{
		if(!name.isEmpty()) {
			if(edit_dialog->has_name_changed()){
				m->model->rename_row(current_row(), name);
			}
		}

		if(!path.isEmpty()) {
			if(edit_dialog->has_path_changed())	{
				m->model->change_path(current_row(), path);
			}
		}

	} break;

	default:
		break;
	}

	edit_dialog->deleteLater();
}

void GUI_LibraryPreferences::current_index_changed(const QModelIndex& idx)
{
	int cur_row = idx.row();
	int row_count = ui->lv_libs->model()->rowCount();

	ui->btn_up->setDisabled(cur_row <= 0 || cur_row >= row_count);
	ui->btn_down->setDisabled(cur_row < 0 || cur_row >= row_count - 1);
	ui->btn_delete->setDisabled(cur_row < 0 || cur_row >= row_count);
	ui->btn_edit->setDisabled(cur_row < 0 || cur_row >= row_count);

	ui->lab_current_path->setVisible(cur_row >= 0 || cur_row < row_count);
	if(cur_row < 0 || cur_row >= row_count){
		return;
	}

	QString path = m->model->path(cur_row);
	ui->lab_current_path->setText(path);
}

