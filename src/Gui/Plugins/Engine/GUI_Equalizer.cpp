/* GUI_Equalizer.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "GUI_Equalizer.h"
#include "EqualizerSlider.h"

#include "Components/Engine/EngineHandler.h"

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"
#include "Gui/Plugins/ui_GUI_Equalizer.h"

#include "Utils/Utils.h"
#include "Utils/EqualizerPresets.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"

#include <QLineEdit>
#include <array>

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
	QList<EQ_Setting>	presets;
	QList<EqualizerSlider*>	sliders;

	ValueArray			old_val;
	int					active_idx;

	Private() :
		active_idx(-1)
	{}

	int find_combo_text(const QComboBox* combo_presets, QString text)
	{
		int ret = -1;

		for(int i=0; i<combo_presets->count(); i++)
		{
			if(combo_presets->itemText(i).compare(text, Qt::CaseInsensitive) == 0){
				ret = i;
			}
		}

		return ret;
	}
};

GUI_Equalizer::GUI_Equalizer(QWidget *parent) :
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


void GUI_Equalizer::init_ui()
{
	setup_parent(this, &ui);

	ui->sli_0->set_label(0, ui->label);
	ui->sli_1->set_label(1, ui->label_2);
	ui->sli_2->set_label(2, ui->label_3);
	ui->sli_3->set_label(3, ui->label_4);
	ui->sli_4->set_label(4, ui->label_5);
	ui->sli_5->set_label(5, ui->label_6);
	ui->sli_6->set_label(6, ui->label_7);
	ui->sli_7->set_label(7, ui->label_8);
	ui->sli_8->set_label(8, ui->label_9);
	ui->sli_9->set_label(9, ui->label_10);

	m->sliders.push_back(ui->sli_0);
	m->sliders.push_back(ui->sli_1);
	m->sliders.push_back(ui->sli_2);
	m->sliders.push_back(ui->sli_3);
	m->sliders.push_back(ui->sli_4);
	m->sliders.push_back(ui->sli_5);
	m->sliders.push_back(ui->sli_6);
	m->sliders.push_back(ui->sli_7);
	m->sliders.push_back(ui->sli_8);
	m->sliders.push_back(ui->sli_9);

	QAction* action_gauss = new QAction("Kurve", ui->btn_tool);
	action_gauss->setCheckable(true);
	action_gauss->setChecked(GetSetting(Set::Eq_Gauss));

	ui->btn_tool->register_action(action_gauss);

	for(EqualizerSlider* s : Util::AsConst(m->sliders))
	{
		connect(s, &EqualizerSlider::sig_value_changed, this, &GUI_Equalizer::sli_changed);
		connect(s, &EqualizerSlider::sig_slider_got_focus, this, &GUI_Equalizer::sli_pressed);
		connect(s, &EqualizerSlider::sig_slider_lost_focus, this, &GUI_Equalizer::sli_released);
	}

	connect(ui->btn_tool, &MenuToolButton::sig_save, this, &GUI_Equalizer::btn_save_clicked);
	connect(ui->btn_tool, &MenuToolButton::sig_delete, this, &GUI_Equalizer::btn_delete_clicked);
	connect(ui->btn_tool, &MenuToolButton::sig_undo, this, &GUI_Equalizer::btn_undo_clicked);
	connect(ui->btn_tool, &MenuToolButton::sig_default, this, &GUI_Equalizer::btn_default_clicked);

	connect(action_gauss, &QAction::toggled, this, &GUI_Equalizer::cb_gauss_toggled);
	connect(ui->combo_presets, &QComboBox::editTextChanged, this, &GUI_Equalizer::text_changed);

	fill_eq_presets();
}


QString GUI_Equalizer::get_name() const
{
	return "Equalizer";
}

QString GUI_Equalizer::get_display_name() const
{
	return tr("Equalizer");
}


void GUI_Equalizer::retranslate_ui()
{
	ui->retranslateUi(this);

	QLineEdit* le = ui->combo_presets->lineEdit();
	le->setPlaceholderText(Lang::get(Lang::EnterName));
}

void GUI_Equalizer::sli_pressed()
{
	EqualizerSlider* sli = static_cast<EqualizerSlider*>(sender());
	int idx = sli->index();

	m->active_idx= idx;

	int i=0;
	for(const EqualizerSlider* slider : Util::AsConst(m->sliders))
	{
		m->old_val[i] = slider->value();
		i++;
	}
}

void GUI_Equalizer::sli_released()
{
	m->active_idx = -1;
}

static double scale[] = {1.0, 0.6, 0.20, 0.06, 0.01};

void GUI_Equalizer::sli_changed(int idx, int new_val)
{
	bool gauss_on = GetSetting(Set::Eq_Gauss);
	ui->btn_tool->show_action(ContextMenu::EntryUndo, true);

	EqualizerSlider* s = m->sliders[idx];
	s->label()->setText(calc_lab(new_val));

	Engine::Handler* engine = Engine::Handler::instance();
	engine->set_equalizer(idx, new_val);

	// this slider has been changed actively
	if( idx == m->active_idx && gauss_on )
	{
		int delta = new_val - m->old_val[idx];

		for(int i=idx-9; i<idx+9; i++)
		{
			if(i < 0) continue;
			if(i == idx) continue;
			if(i >= m->sliders.size()) break;

			// how far is the slider away from me?
			int x = abs(m->active_idx - i);

			if(x > 4) continue;

			double new_val = m->old_val[i] + (delta * scale[x]);

			m->sliders[i]->setValue(new_val);
		}
	}
}


void GUI_Equalizer::fill_eq_presets()
{
	if(!is_ui_initialized()){
		return;
	}

	int last_idx = GetSetting(Set::Eq_Last);

	m->presets = GetSetting(Set::Eq_List);
	m->presets.prepend(EQ_Setting());

	QStringList items;
	for(const EQ_Setting& s : Util::AsConst(m->presets))
	{
		items << s.name();
	}

	ui->combo_presets->insertItems(0, items);

	connect(ui->combo_presets, combo_current_index_changed_int, this, &GUI_Equalizer::preset_changed);

	if(last_idx < m->presets.size() && last_idx >= 0 ) {
		ui->combo_presets->setCurrentIndex(last_idx);
	}

	else{
		last_idx = 0;
	}

	preset_changed(last_idx);
}


void GUI_Equalizer::preset_changed(int index)
{
	if(index >= m->presets.size()) {
		ui->btn_tool->show_actions(ContextMenu::EntryNone);
		return;
	}

	EQ_Setting setting = m->presets[index];

	ui->btn_tool->show_action(ContextMenu::EntryUndo, false);

	bool is_default = setting.is_default();
	ui->btn_tool->show_action(ContextMenu::EntryDefault, !is_default);

	bool is_default_name = setting.is_default_name();
	ui->btn_tool->show_action(ContextMenu::EntryDelete, ((index > 0) && !is_default_name));

	QList<int> values = setting.values();

	for(int i=0; i<values.size(); i++)
	{
		if(i >= m->sliders.size()){
			break;
		}

		int value = values[i];

		m->sliders[i]->setValue(value);
		m->old_val[i] = value;
	}

	SetSetting(Set::Eq_Last, index);
}


void GUI_Equalizer::cb_gauss_toggled(bool b)
{
	SetSetting(Set::Eq_Gauss, b);
}

void GUI_Equalizer::btn_default_clicked()
{
	int cur_idx = ui->combo_presets->currentIndex();
	QString cur_text = ui->combo_presets->currentText();

	if(cur_text.trimmed().isEmpty()){
		return;
	}

	if( !EQ_Setting::is_default_name(cur_text) ){
		return;
	}

	m->presets[cur_idx].set_values( EQ_Setting::get_default_values(cur_text) );
	preset_changed(cur_idx);
}


void GUI_Equalizer::btn_save_clicked()
{
	QString text = ui->combo_presets->currentText();
	if(text.isEmpty()){
		return;
	}

	int found_idx = m->find_combo_text(ui->combo_presets, text);

	if(found_idx <= 0)
	{
		EQ_Setting s = EQ_Setting::fromString(text + ":0:0:0:0:0:0:0:0:0:0");
		m->presets << s;

		ui->combo_presets->addItem(text);
		found_idx = ui->combo_presets->count() - 1;
	}

	for(int i=0; i<m->sliders.size(); i++)
	{
		m->presets[found_idx].set_value(i, m->sliders[i]->value());
	}

	m->presets.removeFirst();
	SetSetting(Set::Eq_List, m->presets);
	m->presets.prepend(EQ_Setting());

	ui->combo_presets->setCurrentIndex(found_idx);
	preset_changed(found_idx);
}

void GUI_Equalizer::btn_delete_clicked()
{
	ui->btn_tool->show_action(ContextMenu::EntryUndo, false);
	int idx = ui->combo_presets->currentIndex();

	ui->combo_presets->setCurrentIndex(0);

	m->presets.removeAt(idx);
	ui->combo_presets->removeItem(idx);

	m->presets.removeFirst();
	SetSetting(Set::Eq_List, m->presets);
	m->presets.prepend(EQ_Setting());
}

void GUI_Equalizer::btn_undo_clicked()
{
	ui->btn_tool->show_action(ContextMenu::EntryUndo, false);
	QString text = ui->combo_presets->currentText();

	int found_idx = m->find_combo_text(ui->combo_presets, text);

	if(found_idx <= 0)
	{
		for(EqualizerSlider* sli : Util::AsConst(m->sliders)){
			sli->setValue(0);
		}
	}

	else
	{
		for(int i=0; i<m->sliders.size(); i++)
		{
			m->sliders[i]->setValue( m->presets[found_idx].value(i) );
		}
	}
}

void GUI_Equalizer::text_changed(const QString& str){
	ui->btn_tool->show_action(ContextMenu::EntrySave, str.size() > 0);
}
