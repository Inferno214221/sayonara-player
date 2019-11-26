/* GUI_CoverPreferences.cpp */

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

#include "Gui/Preferences/ui_GUI_CoverPreferences.h"
#include "GUI_CoverPreferences.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverUtils.h"
#include "Components/Covers/Fetcher/CoverFetcher.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include <QListWidgetItem>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QCheckBox>

using namespace Cover;

GUI_CoverPreferences::GUI_CoverPreferences(const QString& identifier) :
	Base (identifier) {}


GUI_CoverPreferences::~GUI_CoverPreferences()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}

static bool check_cover_template(const QString& cover_template)
{
	if(cover_template.trimmed().isEmpty()){
		return false;
	}

	QString str(cover_template);
	str.remove("<h>");

	QList<QChar> invalid_chars
	{
		'/', '\\', '|', ':', '\"', '?', '$', '<', '>', '*', '#', '%', '&'
	};

	for(const QChar& c : invalid_chars)
	{
		if(str.contains(c)){
			return false;
		}
	}

	return true;
}

bool GUI_CoverPreferences::commit()
{
	QStringList active_items;

	for(int i=0; i<ui->lv_cover_searchers->count(); i++)
	{
		QListWidgetItem* item = ui->lv_cover_searchers->item(i);
		active_items << item->text().toLower();
	}

	SetSetting(Set::Cover_Server, active_items);
	SetSetting(Set::Cover_FetchFromWWW, ui->cb_fetch_from_www->isChecked());
	SetSetting(Set::Cover_SaveToDB, ui->cb_save_to_db->isChecked());
	SetSetting(Set::Cover_SaveToLibrary, ui->cb_save_to_library->isChecked() && ui->cb_save_to_library->isEnabled());
	SetSetting(Set::Cover_SaveToSayonaraDir, ui->cb_save_to_sayonara_dir->isChecked() && ui->cb_save_to_sayonara_dir->isEnabled());

	QString cover_template = ui->le_cover_template->text().trimmed();
	if(check_cover_template(cover_template))
	{
		if(!Util::File::is_imagefile(cover_template))
		{
			QString ext = Util::File::get_file_extension(cover_template);
			if(ext.isEmpty()){
				cover_template.append(".jpg");
				cover_template.replace("..jpg", ".jpg");
			}

			else {
				cover_template.replace("." + ext, ".jpg");
			}

			ui->le_cover_template->setText(cover_template);
		}

		SetSetting(Set::Cover_TemplatePath, cover_template);
	}

	else
	{
		ui->le_cover_template->setText(GetSetting(Set::Cover_TemplatePath));
		ui->lab_template_error->setVisible(false);
	}

	return true;
}

void GUI_CoverPreferences::revert()
{
	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();

	QStringList cover_servers = GetSetting(Set::Cover_Server);

	ui->lv_cover_searchers->clear();
	ui->lv_cover_searchers_inactive->clear();


	QList<Cover::Fetcher::Base*> cover_fetchers = cfm->coverfetchers();
	for(const Cover::Fetcher::Base* b : cover_fetchers)
	{
		QString name = b->identifier();


		if(name.trimmed().isEmpty()) {
			continue;
		}

		if(cover_servers.contains(name))
		{
			ui->lv_cover_searchers->addItem(Util::cvt_str_to_very_first_upper(name));
		}

		else {
			ui->lv_cover_searchers_inactive->addItem(Util::cvt_str_to_very_first_upper(name));
		}
	}

	ui->cb_fetch_from_www->setChecked(GetSetting(Set::Cover_FetchFromWWW));
	ui->cb_save_to_db->setChecked(GetSetting(Set::Cover_SaveToDB));
	ui->cb_save_to_sayonara_dir->setChecked(GetSetting(Set::Cover_SaveToSayonaraDir));
	ui->cb_save_to_library->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->le_cover_template->setText(GetSetting(Set::Cover_TemplatePath));

	fetch_covers_www_triggered(GetSetting(Set::Cover_FetchFromWWW));
	cb_save_to_library_toggled(GetSetting(Set::Cover_SaveToLibrary));

	current_row_changed(ui->lv_cover_searchers->currentRow());
}

QString GUI_CoverPreferences::action_name() const
{
	return Lang::get(Lang::Covers);
}

void GUI_CoverPreferences::init_ui()
{
	if(ui){
		return;
	}

	setup_parent(this, &ui);

	ui->lv_cover_searchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lv_cover_searchers));
	ui->lv_cover_searchers_inactive->setItemDelegate(new Gui::StyledItemDelegate(ui->lv_cover_searchers_inactive));
	ui->lab_template_error->setVisible(false);

	connect(ui->btn_up, &QPushButton::clicked, this, &GUI_CoverPreferences::up_clicked);
	connect(ui->btn_down, &QPushButton::clicked, this, &GUI_CoverPreferences::down_clicked);
	connect(ui->lv_cover_searchers, &QListWidget::currentRowChanged, this, &GUI_CoverPreferences::current_row_changed);
	connect(ui->btn_delete_album_covers, &QPushButton::clicked, this, &GUI_CoverPreferences::delete_covers_from_db);
	connect(ui->btn_delete_files, &QPushButton::clicked, this, &GUI_CoverPreferences::delete_cover_files);
	connect(ui->cb_fetch_from_www, &QCheckBox::toggled, this, &GUI_CoverPreferences::fetch_covers_www_triggered);
	connect(ui->btn_add, &QPushButton::clicked, this, &GUI_CoverPreferences::add_clicked);
	connect(ui->btn_remove, &QPushButton::clicked, this, &GUI_CoverPreferences::remove_clicked);
	connect(ui->cb_save_to_library, &QCheckBox::toggled, this, &GUI_CoverPreferences::cb_save_to_library_toggled);
	connect(ui->le_cover_template, &QLineEdit::textEdited, this, &GUI_CoverPreferences::le_cover_template_edited);

	ui->cb_save_to_sayonara_dir->setToolTip(Cover::Utils::cover_directory());

	revert();
}

void GUI_CoverPreferences::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->btn_up->setText(Lang::get(Lang::MoveUp));
	ui->btn_down->setText(Lang::get(Lang::MoveDown));
}

void GUI_CoverPreferences::skin_changed()
{
	if(!ui){
		return;
	}

	ui->btn_delete_files->setIcon(Gui::Icons::icon(Gui::Icons::Delete));
	ui->btn_delete_album_covers->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
}

void GUI_CoverPreferences::up_clicked()
{
	int cur_row = ui->lv_cover_searchers->currentRow();

	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(cur_row);
	ui->lv_cover_searchers->insertItem(cur_row - 1, item);
	ui->lv_cover_searchers->setCurrentRow(cur_row - 1);
}

void GUI_CoverPreferences::down_clicked()
{
	int cur_row = ui->lv_cover_searchers->currentRow();

	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(cur_row);
	ui->lv_cover_searchers->insertItem(cur_row + 1, item);
	ui->lv_cover_searchers->setCurrentRow(cur_row + 1);
}

void GUI_CoverPreferences::add_clicked()
{
	QListWidgetItem* item = ui->lv_cover_searchers_inactive->takeItem(ui->lv_cover_searchers_inactive->currentRow());
	if(!item){
		return;
	}

	ui->lv_cover_searchers->addItem(item->text());
	delete item; item=nullptr;
}

void GUI_CoverPreferences::remove_clicked()
{
	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(ui->lv_cover_searchers->currentRow());
	if(!item){
		return;
	}

	ui->lv_cover_searchers_inactive->addItem(item->text());
	delete item; item=nullptr;
}

void GUI_CoverPreferences::current_row_changed(int row)
{
	ui->btn_up->setDisabled(row <= 0 || row >= ui->lv_cover_searchers->count());
	ui->btn_down->setDisabled(row < 0 || row >= ui->lv_cover_searchers->count() - 1);
}

void GUI_CoverPreferences::delete_covers_from_db()
{
	DB::Connector::instance()->cover_connector()->clear();
	Cover::ChangeNotfier::instance()->shout();
}

void GUI_CoverPreferences::delete_cover_files()
{
	::Util::File::remove_files_in_directory(Cover::Utils::cover_directory());
}

void GUI_CoverPreferences::fetch_covers_www_triggered(bool b)
{
	ui->lv_cover_searchers->setEnabled(b);
	ui->lv_cover_searchers_inactive->setEnabled(b);
	ui->btn_down->setEnabled(b);
	ui->btn_up->setEnabled(b);
	ui->btn_add->setEnabled(b);
	ui->btn_remove->setEnabled(b);

	ui->cb_save_to_sayonara_dir->setEnabled(b);

	ui->cb_save_to_library->setEnabled(b);
	ui->le_cover_template->setEnabled(b);
	ui->lab_cover_template->setEnabled(b);
}

void GUI_CoverPreferences::cb_save_to_library_toggled(bool b)
{
	ui->le_cover_template->setVisible(b);
	ui->lab_cover_template->setVisible(b);
}


void GUI_CoverPreferences::le_cover_template_edited(const QString& text)
{
	bool valid = check_cover_template(text);
	ui->lab_template_error->setVisible(!valid);
	ui->lab_template_error->setText(Lang::get(Lang::Error) + ": " + Lang::get(Lang::InvalidChars));
}
