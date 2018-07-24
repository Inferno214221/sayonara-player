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
#include "Utils/Utils.h"

using namespace Library;

GUI_CoverView::GUI_CoverView(QWidget* parent) :
	Gui::Widget(parent)
{}


GUI_CoverView::~GUI_CoverView()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}


void GUI_CoverView::init(LocalLibrary* library)
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_CoverView();
	ui->setupUi(this);

	ui->tb_view->init(library);
	ui->lab_zoom->setText(Lang::get(Lang::Zoom).append(":"));
	ui->lab_sorting->setText(Lang::get(Lang::SortBy).append(":"));
	ui->combo_sorting->setEditable(true);

	ui->topbar->setVisible(_settings->get<Set::Lib_CoverShowUtils>());

	connect(ui->combo_sorting, combo_activated_int, this, &GUI_CoverView::combo_sorting_changed);
	connect(ui->combo_zoom, combo_activated_int, this, &GUI_CoverView::combo_zoom_changed);

	Set::listen<Set::Lib_CoverShowUtils>(this, &GUI_CoverView::show_utils_changed, false);
	Set::listen<Set::Lib_Sorting>(this, &GUI_CoverView::sortorder_changed, false);
	Set::listen<Set::Lib_CoverZoom>(this, &GUI_CoverView::zoom_changed, false);

	init_sorting_actions();
	init_zoom_actions();

	connect(ui->tb_view, &ItemView::sig_delete_clicked, this, &GUI_CoverView::sig_delete_clicked);
}

bool GUI_CoverView::is_initialized() const
{
	return (ui != nullptr);
}


void GUI_CoverView::init_sorting_actions()
{
	ui->lab_sorting->setText(Lang::get(Lang::SortBy));
	ui->combo_sorting->clear();

	const QList<ActionPair> action_pairs = CoverView::sorting_actions();
	for(const ActionPair& ap : action_pairs)
	{
		ui->combo_sorting->addItem(ap.name, static_cast<int>(ap.so));
	}

	sortorder_changed();
}


void GUI_CoverView::combo_sorting_changed(int idx)
{
	Q_UNUSED(idx)

	int data = ui->combo_sorting->currentData().toInt();

	Library::SortOrder so = static_cast<Library::SortOrder>(data);
	ui->tb_view->change_sortorder(so);
}


void GUI_CoverView::sortorder_changed()
{
	Library::Sortings s = _settings->get<Set::Lib_Sorting>();
	Library::SortOrder so = s.so_albums;

	for(int i=0; i<ui->combo_sorting->count(); i++)
	{
		if(ui->combo_sorting->itemData(i).toInt() == static_cast<int>(so))
		{
			ui->combo_sorting->setCurrentIndex(i);
			break;
		}
	}
}

void GUI_CoverView::init_zoom_actions()
{
	ui->combo_zoom->addItems(CoverView::zoom_actions());

	zoom_changed();
}


void GUI_CoverView::combo_zoom_changed(int idx)
{
	Q_UNUSED(idx)

	int zoom = ui->combo_zoom->currentText().toInt();
	_settings->set<Set::Lib_CoverZoom>(zoom);
	ui->tb_view->change_zoom(zoom);
}


void GUI_CoverView::zoom_changed()
{
	QStringList zoom_actions = CoverView::zoom_actions();

	int zoom = _settings->get<Set::Lib_CoverZoom>();
	int idx = ::Util::indexOf(zoom_actions, [zoom](const QString& str){
		return (str == QString::number(zoom));
	});

	if(idx >= 0){
		ui->combo_zoom->setCurrentIndex(idx);
	}
}

void GUI_CoverView::show_utils_changed()
{
	bool b = _settings->get<Set::Lib_CoverShowUtils>();
	ui->topbar->setVisible(b);
}

void GUI_CoverView::language_changed()
{
	if(!ui){
		return;
	}

	Gui::Widget::language_changed();

	init_sorting_actions();

	ui->combo_zoom->setToolTip(tr("Use Ctrl + mouse wheel to zoom"));
	ui->lab_sorting->setText(Lang::get(Lang::SortBy));
	ui->lab_zoom->setText(Lang::get(Lang::Zoom));
}
