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

#include "Components/Engine/EngineHandler.h"

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"

#include "Utils/Algorithm.h"
#include "Utils/EqualizerSetting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QLineEdit>
#include <array>

namespace Algorithm=Util::Algorithm;

using Gui::ContextMenu;
using Gui::MenuButton;
using Gui::MenuToolButton;
using Gui::EqualizerSlider;

using SliderArray=std::array<EqualizerSlider*, 10>;
using ValueArray=std::array<int, 10>;

static QString calc_lab(int val)
{
	if(val > 0) {
		double v = val / 2.0;
		if(val % 2 == 0)
			return QString::number(v) + ".0";
		else
			return QString::number(v);
	}

	return QString::number(val) + ".0";
}


struct GUI_Equalizer::Private
{
	QList<EqualizerSetting>	presets;
	QAction*				actionGauss=nullptr;
	SliderArray				sliders;
	ValueArray				oldVal;
	int						activeIndex;

	Private() :
		activeIndex(-1)
	{
		oldVal.fill(0);
	}

	static int find_combo_text(const QComboBox* comboPresets, QString text)
	{
		int ret = -1;

		for(int i=0; i<comboPresets->count(); i++)
		{
			if(comboPresets->itemText(i).compare(text, Qt::CaseInsensitive) == 0){
				ret = i;
			}
		}

		return ret;
	}
};

GUI_Equalizer::GUI_Equalizer(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}

GUI_Equalizer::~GUI_Equalizer()
{
	if(ui)
	{
		delete ui; ui=nullptr;
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
		ui->sli_5, ui->sli_6, ui->sli_7, ui->sli_8,	ui->sli_9
	}};

	m->actionGauss = new QAction(ui->btn_tool);
	m->actionGauss->setCheckable(true);
	m->actionGauss->setChecked(GetSetting(Set::Eq_Gauss));
	m->actionGauss->setText(tr("Linked sliders"));

	ui->btn_tool->registerAction(m->actionGauss);

	for(EqualizerSlider* s : Algorithm::AsConst(m->sliders))
	{
		connect(s, &EqualizerSlider::sigValueChanged, this, &GUI_Equalizer::sliderValueChanged);
		connect(s, &EqualizerSlider::sigSliderGotFocus, this, &GUI_Equalizer::sliderPressed);
		connect(s, &EqualizerSlider::sigSliderLostFocus, this, &GUI_Equalizer::sliderReleased);
	}

	connect(ui->btn_tool, &MenuToolButton::sigSave, this, &GUI_Equalizer::btnSaveClicked);
	connect(ui->btn_tool, &MenuToolButton::sigSaveAs, this, &GUI_Equalizer::btnSaveAsClicked);
	connect(ui->btn_tool, &MenuToolButton::sigDelete, this, &GUI_Equalizer::btnDeleteClicked);
	connect(ui->btn_tool, &MenuToolButton::sigUndo, this, &GUI_Equalizer::btnUndoClicked);
	connect(ui->btn_tool, &MenuToolButton::sigDefault, this, &GUI_Equalizer::btnDefaultClicked);

	connect(m->actionGauss, &QAction::toggled, this, &GUI_Equalizer::checkboxGaussToggled);

	fillEqualizerPresets();
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
	auto* sli = dynamic_cast<EqualizerSlider*>(sender());
	int idx = sli->index();

	m->activeIndex= idx;

	int i=0;
	for(const EqualizerSlider* slider : Algorithm::AsConst(m->sliders))
	{
		m->oldVal[i] = slider->value();
		i++;
	}
}

void GUI_Equalizer::sliderReleased()
{
	m->activeIndex = -1;
}

static double scale[] = {1.0, 0.6, 0.20, 0.06, 0.01};

void GUI_Equalizer::sliderValueChanged(int idx, int new_val)
{
	int slider_size = int(m->sliders.size());

	if(idx < 0 || idx >= slider_size){
		return;
	}

	auto uidx = static_cast<size_t>(idx);

	bool gauss_on = GetSetting(Set::Eq_Gauss);
	ui->btn_tool->showAction(ContextMenu::EntryUndo, true);

	EqualizerSlider* s = m->sliders[uidx];
	s->label()->setText(calc_lab(new_val));

	Engine::Handler* engine = Engine::Handler::instance();
	engine->setEqualizer(idx, new_val);

	// this slider has been changed actively
	if( idx == m->activeIndex && gauss_on )
	{
		int delta = new_val - m->oldVal[uidx];
		int most_left = std::max(idx - 4, 0);
		int most_right = std::min(idx + 4, static_cast<int>(m->sliders.size()));

		for(int i=most_left; i < most_right; i++)
		{
			if(i == idx) {
				continue;
			}

			// how far is the slider away from me?
			int x = abs(m->activeIndex - i);
			double new_val = m->oldVal[i] + (delta * scale[x]);

			m->sliders[i]->setValue(static_cast<int>(new_val));
		}
	}
}


void GUI_Equalizer::fillEqualizerPresets()
{
	if(!isUiInitialized()){
		return;
	}

	int last_idx = GetSetting(Set::Eq_Last);

	m->presets = GetSetting(Set::Eq_List);

	QStringList items;
	for(const EqualizerSetting& s : Algorithm::AsConst(m->presets))
	{
		items << s.name();
	}

	ui->combo_presets->insertItems(0, items);

	connect(ui->combo_presets, combo_current_index_changed_int, this, &GUI_Equalizer::presetChanged);

	if(last_idx < m->presets.size() && last_idx >= 0 ) {
		ui->combo_presets->setCurrentIndex(last_idx);
	}

	else{
		last_idx = 0;
	}

	presetChanged(last_idx);
}


void GUI_Equalizer::presetChanged(int index)
{
	if(index >= m->presets.size()) {
		ui->btn_tool->showActions(ContextMenu::EntryNone);
		return;
	}

	EqualizerSetting setting = m->presets[index];

	ui->btn_tool->showAction(ContextMenu::EntryUndo, false);

	bool is_default = setting.isDefault();
	ui->btn_tool->showAction(ContextMenu::EntryDefault, !is_default);

	bool is_default_name = setting.isDefaultName();
	ui->btn_tool->showAction(ContextMenu::EntryDelete, !is_default_name);

	ui->btn_tool->showAction(ContextMenu::EntrySave, !is_default_name);
	ui->btn_tool->showAction(ContextMenu::EntrySaveAs, true);

	EqualizerSetting::ValueArray values = setting.values();

	for(size_t i=0; i<values.size(); i++)
	{
		m->sliders[i]->setValue(values[i]);
		m->oldVal[i] = values[i];
	}

	SetSetting(Set::Eq_Last, index);
}


void GUI_Equalizer::checkboxGaussToggled(bool b)
{
	SetSetting(Set::Eq_Gauss, b);
}

void GUI_Equalizer::btnDefaultClicked()
{
	int cur_idx = ui->combo_presets->currentIndex();
	QString cur_text = ui->combo_presets->currentText();

	if(cur_text.trimmed().isEmpty()){
		return;
	}

	if( !EqualizerSetting::isDefaultName(cur_text) ){
		return;
	}

	m->presets[cur_idx].setValues( EqualizerSetting::getDefaultValues(cur_text) );
	presetChanged(cur_idx);
}


void GUI_Equalizer::btnSaveClicked()
{
	saveCurrentPreset(ui->combo_presets->currentText());
}

void GUI_Equalizer::btnSaveAsClicked()
{
	Gui::LineInputDialog* dialog = new Gui::LineInputDialog(Lang::get(Lang::SaveAs), Lang::get(Lang::SaveAs), QString(), this);
	dialog->setModal(true);

	connect(dialog, &QDialog::accepted, this, &GUI_Equalizer::saveAsOkClicked);
	dialog->show();
}


void GUI_Equalizer::btnDeleteClicked()
{
	ui->btn_tool->showAction(ContextMenu::EntryUndo, false);
	int idx = ui->combo_presets->currentIndex();

	m->presets.removeAt(idx);
	ui->combo_presets->removeItem(idx);

	if(ui->combo_presets->count() == 0)
	{
		EqualizerSetting s;
		s.setName(Lang::get(Lang::Default));

		m->presets << s;
		ui->combo_presets->addItem(s.name());
	}

	ui->combo_presets->setCurrentIndex(0);

	SetSetting(Set::Eq_List, m->presets);
}


void GUI_Equalizer::btnUndoClicked()
{
	ui->btn_tool->showAction(ContextMenu::EntryUndo, false);
	QString text = ui->combo_presets->currentText();

	int found_idx = m->find_combo_text(ui->combo_presets, text);
	if(found_idx <= 0)
	{
		for(EqualizerSlider* sli : Algorithm::AsConst(m->sliders)){
			sli->setValue(0);
		}
	}

	else
	{
		for(unsigned i=0; i<m->sliders.size(); i++)
		{
			m->sliders[i]->setValue( m->presets[found_idx].value(i) );
		}
	}
}

#include "Utils/Message/Message.h"
void GUI_Equalizer::saveCurrentPreset(const QString& name)
{
	if(name.isEmpty()){
		return;
	}

	int found_idx = m->find_combo_text(ui->combo_presets, name);
	if(found_idx < 0)
	{
		EqualizerSetting s;
		s.setName(name);
		m->presets << s;

		ui->combo_presets->addItem(name);
		found_idx = ui->combo_presets->count() - 1;
	}

	else
	{
		const EqualizerSetting& s = m->presets[found_idx];
		if(s.isDefaultName())
		{
			Message::error(tr("Name %1 not allowed").arg(s.name()), this->displayName());
			return;
		}
	}

	for(unsigned i=0; i<m->sliders.size(); i++)
	{
		m->presets[found_idx].setValue(i, m->sliders[i]->value());
	}

	SetSetting(Set::Eq_List, m->presets);

	ui->combo_presets->setCurrentIndex(found_idx);
	presetChanged(found_idx);
}

void GUI_Equalizer::saveAsOkClicked()
{
	auto dialog = static_cast<Gui::LineInputDialog*>(sender());
	if(!dialog){
		return;
	}

	QString name = dialog->text();
	dialog->deleteLater();

	saveCurrentPreset(name);
}
