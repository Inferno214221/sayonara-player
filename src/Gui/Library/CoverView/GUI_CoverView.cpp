/* AlbumGUI_CoverView.cpp */

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

#include "GUI_CoverView.h"
#include "Gui/Library/ui_GUI_CoverView.h"
#include "Gui/Library/Header/ActionPair.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

namespace
{
	void initSortingActions(QComboBox* comboBox)
	{
		comboBox->clear();

		static const auto actionPairs = Library::CoverView::sortingActions();
		for(const auto& actionPair: actionPairs)
		{
			comboBox->addItem(actionPair.name(), static_cast<int>(actionPair.sortorder()));
		}
	}

	void initZoomActions(QComboBox* comboBox)
	{
		static const auto zoomActions = Library::CoverView::zoomFactors();
		for(const auto& zoom: zoomActions)
		{
			comboBox->addItem(QString::number(zoom) + '%', zoom);
		}
	}
}

namespace Library
{
	GUI_CoverView::GUI_CoverView(QWidget* parent) :
		Gui::Widget(parent) {}

	GUI_CoverView::~GUI_CoverView() = default;

	void GUI_CoverView::init(LocalLibrary* library)
	{
		if(ui)
		{
			return;
		}

		ui = std::make_shared<Ui::GUI_CoverView>();
		ui->setupUi(this);

		ui->coverView->init(library);
		this->setFocusProxy(ui->coverView);

		ui->topbar->setVisible(GetSetting(Set::Lib_CoverShowUtils));
		ui->cbShowArtist->setChecked(GetSetting(Set::Lib_CoverShowArtist));

		// do this before connecting
		initSortingActions(ui->comboSorting);
		initZoomActions(ui->comboZoom);

		connect(ui->comboSorting, combo_activated_int, this, &GUI_CoverView::comboSortingChanged);
		connect(ui->comboZoom, combo_activated_int, this, &GUI_CoverView::comboZoomChanged);
		connect(ui->btnClose, &QPushButton::clicked, this, &GUI_CoverView::closeClicked);
		connect(ui->cbShowArtist, &QCheckBox::toggled, this, &GUI_CoverView::showArtistTriggered);
		connect(ui->coverView, &CoverView::sigReloadClicked, this, &GUI_CoverView::sigReloadClicked);

		ListenSettingNoCall(Set::Lib_CoverShowUtils, GUI_CoverView::showUtilsChanged);
		ListenSettingNoCall(Set::Lib_CoverShowArtist, GUI_CoverView::showArtistChanged);
		ListenSetting(Set::Lib_Sorting, GUI_CoverView::sortorderChanged);
		ListenSetting(Set::Lib_CoverZoom, GUI_CoverView::zoomChanged);

		connect(ui->coverView, &ItemView::sigDeleteClicked, this, &GUI_CoverView::sigDeleteClicked);
	}

	bool GUI_CoverView::isInitialized() const
	{
		return (ui != nullptr);
	}

	IndexSet GUI_CoverView::selectedItems() const
	{
		return (ui)
		       ? ui->coverView->selectedItems()
		       : IndexSet {};
	}

	void GUI_CoverView::clearSelections() const
	{
		if(ui)
		{
			ui->coverView->clearSelection();
		}
	}

	void GUI_CoverView::comboSortingChanged([[maybe_unused]] int index)
	{
		const auto data = ui->comboSorting->currentData().toInt();
		const auto sortOrder = static_cast<SortOrder>(data);

		ui->coverView->changeSortorder(sortOrder);
	}

	void GUI_CoverView::sortorderChanged()
	{
		const auto sortings = GetSetting(Set::Lib_Sorting);
		const auto sortOrder = sortings.so_albums;

		for(auto i = 0; i < ui->comboSorting->count(); i++)
		{
			if(ui->comboSorting->itemData(i).toInt() == static_cast<int>(sortOrder))
			{
				ui->comboSorting->setCurrentIndex(i);
				break;
			}
		}
	}

	void GUI_CoverView::showArtistTriggered(bool showArtist)
	{
		SetSetting(Set::Lib_CoverShowArtist, showArtist);
		ui->coverView->reload();
	}

	void GUI_CoverView::showArtistChanged()
	{
		const auto showArtist = GetSetting(Set::Lib_CoverShowArtist);
		ui->cbShowArtist->setChecked(showArtist);
	}

	void GUI_CoverView::comboZoomChanged([[maybe_unused]] int index)
	{
		const auto zoom = ui->comboZoom->currentData().toInt();
		ui->coverView->changeZoom(zoom);
	}

	void GUI_CoverView::closeClicked()
	{
		SetSetting(Set::Lib_CoverShowUtils, false);
	}

	void GUI_CoverView::zoomChanged()
	{
		static const auto zoomFactors = CoverView::zoomFactors();

		const auto zoom = GetSetting(Set::Lib_CoverZoom);
		const auto index = Util::Algorithm::indexOf(zoomFactors, [zoom](const auto& zoomFactor) {
			return (zoomFactor == zoom);
		});

		if(index >= 0)
		{
			ui->comboZoom->setCurrentIndex(index);
		}
	}

	void GUI_CoverView::showUtilsChanged()
	{
		const auto showUtils = GetSetting(Set::Lib_CoverShowUtils);
		ui->topbar->setVisible(showUtils);
	}

	void GUI_CoverView::languageChanged()
	{
		if(ui)
		{
			Gui::Widget::languageChanged();

			initSortingActions(ui->comboSorting);
			sortorderChanged();

			ui->comboZoom->setToolTip(tr("Use Ctrl + mouse wheel to zoom"));
			ui->btnClose->setText(Lang::get(Lang::Hide));
			ui->cbShowArtist->setText(Lang::get(Lang::ShowAlbumArtists));
		}
	}

	void GUI_CoverView::showEvent(QShowEvent* e)
	{
		Gui::Widget::showEvent(e);
		ui->coverView->changeZoom();
	}
}