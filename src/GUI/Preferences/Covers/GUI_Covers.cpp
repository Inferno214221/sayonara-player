/* GUI_Covers.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "GUI/Preferences/ui_GUI_Covers.h"

#include "GUI_Covers.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverFetcherInterface.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverUtils.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "GUI/Utils/Delegates/StyledItemDelegate.h"

#include <QListWidgetItem>
#include <QList>
#include <QDir>
#include <QFileInfo>

using namespace Cover;

GUI_Covers::GUI_Covers(const QString& identifier) :
	Base (identifier) {}


GUI_Covers::~GUI_Covers()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}

bool GUI_Covers::commit()
{
	Settings* settings = Settings::instance();
	QStringList active_items;

	for(int i=0; i<ui->lv_cover_searchers->count(); i++)
	{
		QListWidgetItem* item = ui->lv_cover_searchers->item(i);
		active_items << item->text();
	}

	settings->set<Set::Cover_Server>(active_items);
	settings->set<Set::Cover_LoadFromFile>(ui->cb_load_covers_from_file->isChecked());

	return true;
}

void GUI_Covers::revert()
{
	Settings* settings = Settings::instance();

	QStringList cover_servers = settings->get<Set::Cover_Server>();

	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();
	QList<Cover::Fetcher::Base*> cover_fetchers = cfm->coverfetchers();

	if(cover_servers.size() != cover_fetchers.size())
	{
		cover_servers.clear();
		for(const Cover::Fetcher::Base* b : cover_fetchers)
		{
			cover_servers << b->keyword();
		}
	}

	ui->lv_cover_searchers->clear();

	for(const QString& cover_server : cover_servers)
	{
		if(!cover_server.isEmpty())
		{
			ui->lv_cover_searchers->addItem(cover_server);
		}
	}

	ui->cb_load_covers_from_file->setChecked(settings->get<Set::Cover_LoadFromFile>());

	current_row_changed(ui->lv_cover_searchers->currentRow());
}

QString GUI_Covers::action_name() const
{
	return Lang::get(Lang::Covers);
}

void GUI_Covers::init_ui()
{
	if(ui){
		return;
	}

	setup_parent(this, &ui);

	ui->lv_cover_searchers->clear();
	ui->lv_cover_searchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lv_cover_searchers));

	connect(ui->btn_up, &QPushButton::clicked, this, &GUI_Covers::up_clicked);
	connect(ui->btn_down, &QPushButton::clicked, this, &GUI_Covers::down_clicked);
	connect(ui->lv_cover_searchers, &QListWidget::currentRowChanged, this, &GUI_Covers::current_row_changed);
	connect(ui->btn_delete_album_covers, &QPushButton::clicked, this, &GUI_Covers::delete_covers_from_db);
	connect(ui->btn_delete_files, &QPushButton::clicked, this, &GUI_Covers::delete_cover_files);

	revert();
}

void GUI_Covers::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->btn_up->setText(Lang::get(Lang::MoveUp));
	ui->btn_down->setText(Lang::get(Lang::MoveDown));
}

void GUI_Covers::up_clicked()
{
	int cur_row = ui->lv_cover_searchers->currentRow();

	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(cur_row);
	ui->lv_cover_searchers->insertItem(cur_row - 1, item);
	ui->lv_cover_searchers->setCurrentRow(cur_row - 1);
}

void GUI_Covers::down_clicked()
{
	int cur_row = ui->lv_cover_searchers->currentRow();

	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(cur_row);
	ui->lv_cover_searchers->insertItem(cur_row + 1, item);
	ui->lv_cover_searchers->setCurrentRow(cur_row + 1);
}

void GUI_Covers::current_row_changed(int row)
{
	ui->btn_up->setDisabled(row <= 0 || row >= ui->lv_cover_searchers->count());
	ui->btn_down->setDisabled(row < 0 || row >= ui->lv_cover_searchers->count() - 1);
}

void GUI_Covers::delete_covers_from_db()
{
	DB::Connector::instance()->cover_connector()->clear();
	Cover::ChangeNotfier::instance()->shout();
}

void GUI_Covers::delete_cover_files()
{
	::Util::File::remove_files_in_directory(Cover::Utils::cover_directory());
}
