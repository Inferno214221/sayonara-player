/* AlbumGUI_CoverView.cpp */

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

#include "GUI_CoverView.h"
#include "GUI/Library/ui_GUI_CoverView.h"

#include "GUI/Library/Utils/ActionPair.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"

using namespace Library;

GUI_CoverView::GUI_CoverView(QWidget* parent) :
	Gui::Widget(parent)
{
	ui = new Ui::GUI_CoverView();
	ui->setupUi(this);

	ui->lab_zoom->setText(Lang::get(Lang::Zoom).append(":"));
	ui->lab_sorting->setText(Lang::get(Lang::SortBy).append(":"));
	ui->combo_sorting->setEditable(true);

	ui->topbar->setVisible(_settings->get<Set::Lib_CoverShowUtils>());

	connect(ui->tb_view, &CoverView::sig_zoom_changed, this, &GUI_CoverView::zoom_changed);
	connect(ui->tb_view, &CoverView::sig_sortorder_changed, this, &GUI_CoverView::sortorder_changed);

	connect(ui->combo_sorting, combo_activated_int, this, &GUI_CoverView::combo_sorting_changed);
	connect(ui->combo_zoom, combo_activated_int, this, &GUI_CoverView::combo_zoom_changed);

	Set::listen<Set::Lib_CoverShowUtils>(this, &GUI_CoverView::show_utils_changed, false);

	init_sorting_actions();
	init_zoom_actions();
}


GUI_CoverView::~GUI_CoverView()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}


void GUI_CoverView::init(LocalLibrary* library)
{
	ui->tb_view->init(library);
}


ItemView* GUI_CoverView::cover_view()
{
	return ui->tb_view;
}


void GUI_CoverView::init_sorting_actions()
{
	ui->lab_sorting->setText(Lang::get(Lang::SortBy));
	ui->combo_sorting->clear();

	const QList<ActionPair> action_pairs = ui->tb_view->sorting_options();
	for(const ActionPair& ap : action_pairs)
	{
		ui->combo_sorting->addItem(ap.name, (int) ap.so);
	}

	ui->tb_view->init_sorting_actions();
}


void GUI_CoverView::combo_sorting_changed(int idx)
{
	Q_UNUSED(idx)

	int data = ui->combo_sorting->currentData().toInt();
	ui->tb_view->change_sortorder((Library::SortOrder) data);
}


void GUI_CoverView::sortorder_changed(SortOrder so)
{
	for(int i=0; i<ui->combo_sorting->count(); i++)
	{
		if(ui->combo_sorting->itemData(i).toInt() == (int) so)
		{
			ui->combo_sorting->setCurrentIndex(i);
		}
	}
}

void GUI_CoverView::init_zoom_actions()
{
	QStringList zoom_actions = ui->tb_view->zoom_actions();
	ui->combo_zoom->addItems(zoom_actions);

	ui->tb_view->init_zoom_actions();
}


void GUI_CoverView::combo_zoom_changed(int idx)
{
	Q_UNUSED(idx)

	ui->tb_view->change_zoom(ui->combo_zoom->currentText().toInt());
}


void GUI_CoverView::zoom_changed(int zoom)
{
	for(int i=0; i<ui->combo_zoom->count(); i++)
	{
		if(ui->combo_zoom->itemText(i).toInt() >= zoom)
		{
			ui->combo_zoom->setCurrentIndex(i);
			break;
		}
	}
}

void GUI_CoverView::show_utils_changed()
{
	bool b = _settings->get<Set::Lib_CoverShowUtils>();
	ui->topbar->setVisible(b);
}

void GUI_CoverView::language_changed()
{
	Gui::Widget::language_changed();
	init_sorting_actions();

	ui->combo_zoom->setToolTip(tr("Use Ctrl + mouse wheel to zoom"));
	ui->lab_sorting->setText(Lang::get(Lang::SortBy));
	ui->lab_zoom->setText(Lang::get(Lang::Zoom));
}

