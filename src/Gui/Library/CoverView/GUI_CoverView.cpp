/* AlbumGUI_CoverView.cpp */

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

#include "GUI_CoverView.h"
#include "Gui/Library/ui_GUI_CoverView.h"
#include "Gui/Library/Header/ActionPair.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

namespace Algorithm=Util::Algorithm;
using Library::GUI_CoverView;

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
	this->setFocusProxy(ui->tb_view);

	ui->topbar->setVisible(GetSetting(Set::Lib_CoverShowUtils));
	ui->cb_show_artist->setChecked(GetSetting(Set::Lib_CoverShowArtist));

	connect(ui->combo_sorting, combo_activated_int, this, &GUI_CoverView::comboSortingChanged);
	connect(ui->combo_zoom, combo_activated_int, this, &GUI_CoverView::comboZoomChanged);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_CoverView::closeClicked);
	connect(ui->cb_show_artist, &QCheckBox::toggled, this, &GUI_CoverView::showArtistTriggered);
	connect(ui->tb_view, &CoverView::sigReloadClicked, this, &GUI_CoverView::sigReloadClicked);

	ListenSettingNoCall(Set::Lib_CoverShowUtils, GUI_CoverView::showUtilsChanged);
	ListenSettingNoCall(Set::Lib_Sorting, GUI_CoverView::sortorderChanged);
	ListenSettingNoCall(Set::Lib_CoverZoom, GUI_CoverView::zoomChanged);
	ListenSettingNoCall(Set::Lib_CoverShowArtist, GUI_CoverView::showArtistChanged);

	initSortingActions();
	initZoomActions();

	connect(ui->tb_view, &ItemView::sigDeleteClicked, this, &GUI_CoverView::sigDeleteClicked);

	ui->tb_view->changeZoom(GetSetting(Set::Lib_CoverZoom));
}

bool GUI_CoverView::isInitialized() const
{
	return (ui != nullptr);
}

IndexSet GUI_CoverView::selectedItems() const
{
	if(ui){
		return ui->tb_view->selectedItems();
	}

	return IndexSet();
}

void GUI_CoverView::clearSelections() const
{
	if(ui){
		ui->tb_view->clearSelection();
	}
}

void GUI_CoverView::initSortingActions()
{
	ui->combo_sorting->clear();

	const QList<ActionPair> action_pairs = CoverView::sortingActions();
	for(const ActionPair& ap : action_pairs)
	{
		ui->combo_sorting->addItem(ap.name(), int(ap.sortorder()));
	}

	sortorderChanged();
}


void GUI_CoverView::comboSortingChanged(int idx)
{
	Q_UNUSED(idx)

	int data = ui->combo_sorting->currentData().toInt();

	auto so = Library::SortOrder(data);
	ui->tb_view->changeSortorder(so);
}


void GUI_CoverView::sortorderChanged()
{
	Library::Sortings s = GetSetting(Set::Lib_Sorting);
	Library::SortOrder so = s.so_albums;

	for(int i=0; i<ui->combo_sorting->count(); i++)
	{
		if(ui->combo_sorting->itemData(i).toInt() == int(so))
		{
			ui->combo_sorting->setCurrentIndex(i);
			break;
		}
	}
}


void GUI_CoverView::showArtistTriggered(bool b)
{
	SetSetting(Set::Lib_CoverShowArtist, b);
	ui->tb_view->reload();
}


void GUI_CoverView::showArtistChanged()
{
	bool b = GetSetting(Set::Lib_CoverShowArtist);
	ui->cb_show_artist->setChecked(b);
}

void GUI_CoverView::initZoomActions()
{
	const QStringList zoom_data = CoverView::zoomActions();

	for(const QString& zoom : zoom_data)
	{
		ui->combo_zoom->addItem(zoom + "%", zoom);
	}

	zoomChanged();
}


void GUI_CoverView::comboZoomChanged(int idx)
{
	Q_UNUSED(idx)

	int zoom = ui->combo_zoom->currentData().toInt();
	SetSetting(Set::Lib_CoverZoom, zoom);
	ui->tb_view->changeZoom(zoom);
}

void GUI_CoverView::closeClicked()
{
	SetSetting(Set::Lib_CoverShowUtils, false);
}


void GUI_CoverView::zoomChanged()
{
	QStringList zoom_actions = CoverView::zoomActions();

	int zoom = GetSetting(Set::Lib_CoverZoom);
	int idx = Algorithm::indexOf(zoom_actions, [zoom](const QString& str){
		return (str == QString::number(zoom));
	});

	if(idx >= 0){
		ui->combo_zoom->setCurrentIndex(idx);
	}
}

void GUI_CoverView::showUtilsChanged()
{
	bool b = GetSetting(Set::Lib_CoverShowUtils);
	ui->topbar->setVisible(b);
}

void GUI_CoverView::languageChanged()
{
	if(!ui){
		return;
	}

	Gui::Widget::languageChanged();

	initSortingActions();

	ui->combo_zoom->setToolTip(tr("Use Ctrl + mouse wheel to zoom"));
	ui->btn_close->setText(Lang::get(Lang::Hide));
	ui->cb_show_artist->setText(Lang::get(Lang::ShowAlbumArtists));
}
