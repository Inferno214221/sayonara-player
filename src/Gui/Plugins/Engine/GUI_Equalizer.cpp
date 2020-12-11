/* GUI_Equalizer.cpp */

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


/*
 * GUI_Equalizer.cpp
 *
 *  Created on: May 18, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "GUI_Equalizer.h"
#include "EqualizerSlider.h"
#include "Gui/Plugins/ui_GUI_Equalizer.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include "Components/Equalizer/Equalizer.h"

#include "Utils/Algorithm.h"
#include "Utils/EqualizerSetting.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"

#include <QLineEdit>
#include <array>

namespace Algorithm = Util::Algorithm;

using Gui::ContextMenu;
using Gui::MenuToolButton;
using Gui::EqualizerSlider;

using SliderArray = std::array<EqualizerSlider*, 10>;

struct GUI_Equalizer::Private
{
	Equalizer* equalizer = nullptr;
	QAction* actionGauss = nullptr;
	SliderArray sliders;

	Private(GUI_Equalizer* parent) :
		equalizer(new Equalizer(parent)) {}

	void applySetting(const EqualizerSetting& equalizer)
	{
		int band = 0;
		for(const auto value : equalizer)
		{
			auto* slider = this->sliders[band];

			slider->setSilent(true);
			slider->setEqualizerValue(value);
			slider->setSilent(false);

			band++;
		}
	}
};

GUI_Equalizer::GUI_Equalizer(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->equalizer, &Equalizer::sigValueChanged,
	        this, &GUI_Equalizer::valueChanged);
}

GUI_Equalizer::~GUI_Equalizer()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_Equalizer::initUi()
{
	setupParent(this, &ui);

	ui->sli_0->setLabel(0, ui->label);
	ui->sli_1->setLabel(1, ui->label_2);
	ui->sli_2->setLabel(2, ui->label_3);
	ui->sli_3->setLabel(3, ui->label_4);
	ui->sli_4->setLabel(4, ui->label_5);
	ui->sli_5->setLabel(5, ui->label_6);
	ui->sli_6->setLabel(6, ui->label_7);
	ui->sli_7->setLabel(7, ui->label_8);
	ui->sli_8->setLabel(8, ui->label_9);
	ui->sli_9->setLabel(9, ui->label_10);

	m->sliders = SliderArray
		{{
			 ui->sli_0, ui->sli_1, ui->sli_2, ui->sli_3, ui->sli_4,
			 ui->sli_5, ui->sli_6, ui->sli_7, ui->sli_8, ui->sli_9
		 }};

	m->actionGauss = new QAction(ui->btn_tool);
	m->actionGauss->setCheckable(true);
	m->actionGauss->setChecked(m->equalizer->isGaussEnabled());
	m->actionGauss->setText(tr("Linked sliders"));

	ui->btn_tool->registerAction(m->actionGauss);

	for(auto* slider : Algorithm::AsConst(m->sliders))
	{
		connect(slider, &EqualizerSlider::sigValueChanged,
		        this, &GUI_Equalizer::sliderValueChanged);
		connect(slider, &EqualizerSlider::sigSliderGotFocus,
		        this, &GUI_Equalizer::sliderPressed);
		connect(slider, &EqualizerSlider::sigSliderLostFocus,
		        this, &GUI_Equalizer::sliderReleased);
	}

	connect(ui->btn_tool, &MenuToolButton::sigSaveAs,
	        this, &GUI_Equalizer::btnSaveAsClicked);
	connect(ui->btn_tool, &MenuToolButton::sigDelete,
	        this, &GUI_Equalizer::btnDeleteClicked);
	connect(ui->btn_tool, &MenuToolButton::sigDefault,
	        this, &GUI_Equalizer::btnDefaultClicked);
	connect(ui->btn_tool, &MenuToolButton::sigRename,
	        this, &GUI_Equalizer::btnRenameClicked);

	connect(m->actionGauss, &QAction::toggled,
	        this, &GUI_Equalizer::checkboxGaussToggled);

	fillEqualizerPresets();

	currentEqualizerChanged(m->equalizer->currentIndex());
}

QString GUI_Equalizer::name() const
{
	return "Equalizer";
}

QString GUI_Equalizer::displayName() const
{
	return tr("Equalizer");
}

void GUI_Equalizer::retranslate()
{
	ui->retranslateUi(this);

	if(m->actionGauss)
	{
		m->actionGauss->setText(tr("Linked sliders"));
	}
}

void GUI_Equalizer::sliderPressed()
{
	m->equalizer->startValueChange();
}

void GUI_Equalizer::sliderReleased()
{
	m->equalizer->endValueChange();
}

void GUI_Equalizer::sliderValueChanged(int band, int newValue)
{
	if(!Util::between(band, m->sliders))
	{
		return;
	}

	ui->btn_tool->showAction(ContextMenu::EntryDefault, true);
	m->equalizer->changeValue(m->equalizer->currentIndex(), band, newValue);
}

void GUI_Equalizer::valueChanged(int band, int newValue)
{
	auto* slider = m->sliders[size_t(band)];

	slider->setSilent(true);
	slider->setEqualizerValue(newValue);
	slider->setSilent(false);
}

void GUI_Equalizer::fillEqualizerPresets()
{
	if(!isUiInitialized())
	{
		return;
	}

	const auto presetNames = m->equalizer->names();
	for(const auto& preset : presetNames)
	{
		ui->combo_presets->addItem(preset);
	}

	const auto activeIndex = m->equalizer->currentIndex();
	ui->combo_presets->setCurrentIndex(activeIndex);

	connect(ui->combo_presets,
	        combo_current_index_changed_int,
	        this,
	        &GUI_Equalizer::currentEqualizerChanged);
}

void GUI_Equalizer::currentEqualizerChanged(int index)
{
	const auto presetNames = m->equalizer->names();
	if(!Util::between(index, presetNames))
	{
		ui->btn_tool->showActions(ContextMenu::EntryNone);
		return;
	}

	m->equalizer->setCurrentIndex(index);

	const auto& currentEqualizer = m->equalizer->currentSetting();
	const auto isDefault = currentEqualizer.isDefault();
	const auto isFactoryPreset = m->equalizer->defaultNames().contains(
		currentEqualizer.name());

	ui->btn_tool->showAction(ContextMenu::EntryDefault, !isDefault);
	ui->btn_tool->showAction(ContextMenu::EntryDelete, !isFactoryPreset);
	ui->btn_tool->showAction(ContextMenu::EntryRename, !isFactoryPreset);
	ui->btn_tool->showAction(ContextMenu::EntrySaveAs, true);

	m->applySetting(currentEqualizer);
}

void GUI_Equalizer::checkboxGaussToggled(bool b)
{
	m->equalizer->setGaussEnabled(b);
}

void GUI_Equalizer::btnDefaultClicked()
{
	m->equalizer->resetPreset(ui->combo_presets->currentIndex());
	ui->btn_tool->showAction(ContextMenu::EntryDefault, false);
}

template<typename Callback>
static void showDialog(GUI_Equalizer* parent, Lang::Term term, Callback callback)
{
	auto* dialog =
		new Gui::LineInputDialog(Lang::get(term), Lang::get(term), "", parent);

	dialog->setModal(true);
	parent->connect(dialog, &QDialog::accepted, parent, callback);
	dialog->show();
}

void GUI_Equalizer::btnSaveAsClicked()
{
	showDialog(this, Lang::SaveAs, &GUI_Equalizer::saveAsOkClicked);
}

void GUI_Equalizer::btnRenameClicked()
{
	showDialog(this, Lang::Rename, &GUI_Equalizer::renameOkClicked);
}

void GUI_Equalizer::btnDeleteClicked()
{
	const auto currentIndex = ui->combo_presets->currentIndex();
	if(m->equalizer->deletePreset(currentIndex))
	{
		ui->combo_presets->removeItem(currentIndex);
		ui->combo_presets->setCurrentIndex(m->equalizer->currentIndex());
	}
}

static QString getInputDialogText(QObject* possibleDialog)
{
	auto* dialog = dynamic_cast<Gui::LineInputDialog*>(possibleDialog);
	if(!dialog)
	{
		return QString();
	}

	const auto text = dialog->text();
	dialog->deleteLater();

	return text;
}

void GUI_Equalizer::saveAsOkClicked()
{
	const auto newName = getInputDialogText(sender());
	const auto err = m->equalizer->saveCurrentEqualizerAs(newName);

	if(err == Equalizer::RenameError::NoError)
	{
		const auto lastIndex = m->equalizer->count() - 1;
		const auto equalizer = m->equalizer->equalizerSetting(lastIndex);
		ui->combo_presets->addItem(equalizer.name(), equalizer.id());
	}
}

void GUI_Equalizer::renameOkClicked()
{
	const auto newName = getInputDialogText(sender());
	const auto currentIndex = m->equalizer->currentIndex();
	const auto err = m->equalizer->renamePreset(currentIndex, newName);

	if(err == Equalizer::RenameError::NoError)
	{
		ui->combo_presets->setItemText(currentIndex, newName);
	}
}