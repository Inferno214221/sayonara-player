/* GUI_StyleSettings.cpp */

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

#include "GUI_StyleSettings.h"
#include "Gui/ui_GUI_Style.h"
#include "VisualStyleTypes.h"

#include "Utils/Algorithm.h"
#include "Utils/Message/Message.h"

#include "Database/Connector.h"
#include "Database/VisualStyles.h"

#include <QColorDialog>
#include <QCloseEvent>
#include <QList>

namespace Algorithm=Util::Algorithm;

struct GUI_StyleSettings::Private
{
	DB::VisualStyles*		db=nullptr;

	QList<RawColorStyle>	styles;
	QList<RawColorStyle>	styles_old;

	RawColorStyle			cur_style;
	QColor					colors[4];

	QString		cur_text;
	int			cur_idx;
	bool		sth_changed;

	Private()
	{
		db = DB::Connector::instance()->visual_style_connector();
	}
};

GUI_StyleSettings::GUI_StyleSettings(QWidget *parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_Style();
	ui->setupUi(this);

	connect_combo_idx_changed();

	connect(ui->combo_styles, &QComboBox::editTextChanged, this, &GUI_StyleSettings::combo_text_changed);
	connect(ui->btn_col1, &QPushButton::clicked, this, &GUI_StyleSettings::col1_activated);
	connect(ui->btn_col2, &QPushButton::clicked, this, &GUI_StyleSettings::col2_activated);
	connect(ui->btn_col3, &QPushButton::clicked, this, &GUI_StyleSettings::col3_activated);
	connect(ui->btn_col4, &QPushButton::clicked, this, &GUI_StyleSettings::col4_activated);
	connect(ui->btn_save, &QPushButton::clicked, this, &GUI_StyleSettings::save_pressed);
	connect(ui->btn_delete, &QPushButton::clicked, this, &GUI_StyleSettings::del_pressed);
	connect(ui->btn_undo, &QPushButton::clicked, this, &GUI_StyleSettings::undo_pressed);

	connect_spinboxes();

	set_sth_changed(false);

	init();
}

GUI_StyleSettings::~GUI_StyleSettings() {}

void GUI_StyleSettings::language_changed()
{
	ui->retranslateUi(this);
}

void GUI_StyleSettings::init()
{
	ui->combo_styles->clear();

	m->styles.clear();
	m->styles = m->db->get_raw_color_styles();

	RawColorStyle style;

	for(int i=0; i<4; i++)
	{
		m->colors[i] = QColor(0,0,0,0);
		style.col_list.colors << QColor(0,0,0,0);
	}

	m->styles.push_front(style);

	for(const RawColorStyle& style : Algorithm::AsConst(m->styles))
	{
		ui->combo_styles->addItem(style.col_list.name);
	}

	m->cur_idx = ui->combo_styles->currentIndex();
	m->cur_text = ui->combo_styles->currentText();
	m->cur_style = m->styles[m->cur_idx];
}

QIcon col2Icon(QColor col)
{
	QPixmap pm(18,18);
	pm.fill(col);

	QIcon icon(pm);
	return icon;
}


void GUI_StyleSettings::connect_spinbox(const QSpinBox* box)
{
	connect(box,
			static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
			this,
			&GUI_StyleSettings::spin_box_changed
	);
}

void GUI_StyleSettings::connect_spinboxes()
{
	connect_spinbox(ui->sb_fading_steps_lv);
	connect_spinbox(ui->sb_fading_steps_sp);
	connect_spinbox(ui->sb_h_spacing_lv);
	connect_spinbox(ui->sb_h_spacing_sp);
	connect_spinbox(ui->sb_rect_height_sp);
	connect_spinbox(ui->sb_rect_width_lv);
	connect_spinbox(ui->sb_rect_height_lv);
	connect_spinbox(ui->sb_v_spacing_lv);
	connect_spinbox(ui->sb_v_spacing_sp);
}

void GUI_StyleSettings::disconnect_spinbox(const QSpinBox *box)
{
	disconnect(box,
			static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
			this,
			&GUI_StyleSettings::spin_box_changed
	);
}


void GUI_StyleSettings::disconnect_spinboxes()
{
	disconnect_spinbox(ui->sb_fading_steps_lv);
	disconnect_spinbox(ui->sb_fading_steps_sp);
	disconnect_spinbox(ui->sb_h_spacing_lv);
	disconnect_spinbox(ui->sb_h_spacing_sp);
	disconnect_spinbox(ui->sb_rect_height_sp);
	disconnect_spinbox(ui->sb_rect_width_lv);
	disconnect_spinbox(ui->sb_rect_height_lv);
	disconnect_spinbox(ui->sb_v_spacing_lv);
	disconnect_spinbox(ui->sb_v_spacing_sp);
}

void GUI_StyleSettings::connect_combo_idx_changed()
{
	connect(ui->combo_styles, combo_current_index_changed_int, this, &GUI_StyleSettings::combo_styles_changed);
}

void GUI_StyleSettings::disconnect_combo_idx_changed()
{
	disconnect(ui->combo_styles, combo_current_index_changed_int, this, &GUI_StyleSettings::combo_styles_changed);
}


void GUI_StyleSettings::combo_styles_changed(int idx)
{
	int new_idx = idx;

	if(m->sth_changed)
	{
		Message::Answer ret = Message::question_yn(tr("There are some unsaved settings<br />Save now?"));

		if(ret == Message::Answer::Yes)
		{
			save_pressed();
			// save was not successful
			if(m->cur_text.isEmpty())
			{
				disconnect_combo_idx_changed();
				ui->combo_styles->setCurrentIndex(m->cur_idx);
				connect_combo_idx_changed();
				return;
			}

			else {
				set_sth_changed(false);

				disconnect_combo_idx_changed();
				ui->combo_styles->setCurrentIndex(new_idx);
				connect_combo_idx_changed();
			}
		}

		else {
			set_sth_changed(false);
		}
	}

	disconnect_spinboxes();

	m->cur_idx = ui->combo_styles->currentIndex();
	m->cur_text = ui->combo_styles->currentText();

	ui->btn_delete->setDisabled(idx == 0);
	ui->btn_undo->setDisabled(idx == 0);

	RawColorStyle style = m->styles[idx];
	ui->sb_fading_steps_lv->setValue(style.n_fading_steps_level);
	ui->sb_fading_steps_sp->setValue(style.n_fading_steps_spectrum);
	ui->sb_h_spacing_lv->setValue(style.hor_spacing_level);
	ui->sb_h_spacing_sp->setValue(style.hor_spacing_spectrum);
	ui->sb_v_spacing_lv->setValue(style.ver_spacing_level);
	ui->sb_v_spacing_sp->setValue(style.ver_spacing_spectrum);
	ui->sb_rect_height_sp->setValue(style.rect_height_spectrum);
	ui->sb_rect_width_lv->setValue(style.rect_width_level);
	ui->sb_rect_height_lv->setValue(style.rect_height_level);

	ui->cb_col3->setChecked(style.col_list.colors.size() > 2 && style.col_list.colors[2].isValid());
	ui->cb_col4->setChecked(style.col_list.colors.size() > 3 && style.col_list.colors[3].isValid());

	QList<QColor> col_list = style.col_list.colors;
	ui->btn_col1->setIcon(col2Icon(col_list[0]));
	ui->btn_col2->setIcon(col2Icon(col_list[1]));

	if(col_list.size() > 2) {
		ui->btn_col3->setIcon(col2Icon(col_list[2]));
	}

	else  {
		ui->btn_col3->setIcon(col2Icon(QColor(0, 0, 0, 0)));
	}

	if(col_list.size() > 3) {
		ui->btn_col4->setIcon(col2Icon(col_list[3]));
	}

	else {
		ui->btn_col4->setIcon(col2Icon(QColor(0, 0, 0, 0)));
	}

	m->colors[0] = col_list[0];
	m->colors[1] = col_list[1];
	m->colors[2] = (col_list.size() > 2) ? col_list[2] : QColor(0,0,0,0);
	m->colors[3] = (col_list.size() > 3) ? col_list[3] : QColor(0,0,0,0);

	connect_spinboxes();

	m->cur_idx = ui->combo_styles->currentIndex();
	m->cur_text = ui->combo_styles->currentText();
	m->cur_style = m->styles[m->cur_idx];
	set_sth_changed(false);
}

void GUI_StyleSettings::save_pressed()
{
	// we came from [0]
	if(m->cur_idx == 0 && m->cur_text.isEmpty()) {
		Message::warning(tr("Please specify a name"));
		return;
	}

	RawColorStyle style;
	style.n_fading_steps_level = ui->sb_fading_steps_lv->value();
	style.n_fading_steps_spectrum = ui->sb_fading_steps_sp->value();
	style.hor_spacing_level = ui->sb_h_spacing_lv->value();
	style.hor_spacing_spectrum = ui->sb_h_spacing_sp->value();
	style.ver_spacing_level = ui->sb_v_spacing_lv->value();
	style.ver_spacing_spectrum = ui->sb_v_spacing_sp->value();
	style.rect_height_spectrum = ui->sb_rect_height_sp->value();
	style.rect_width_level = ui->sb_rect_width_lv->value();
	style.rect_height_level = ui->sb_rect_height_lv->value();

	style.col_list.name = m->cur_text;
	style.col_list.colors << m->colors[0] << m->colors[1];
	if(ui->cb_col3->isChecked()) {
		style.col_list.colors << m->colors[2];
	}
	if(ui->cb_col4->isChecked()) {
		style.col_list.colors << m->colors[3];
	}

	m->db->update_raw_color_style(style);

	m->styles[m->cur_idx] = style;
	set_sth_changed(false);

	disconnect_combo_idx_changed();

	int cur_idx = m->cur_idx;
	RawColorStyle tmp_style = m->cur_style;
	init();
	ui->combo_styles->setCurrentIndex(cur_idx);
	m->cur_idx = cur_idx;
	m->cur_text = ui->combo_styles->currentText();

	connect_combo_idx_changed();
	combo_styles_changed(m->cur_idx);

	m->cur_style = tmp_style;

	emit sig_style_update();
}


void GUI_StyleSettings::col1_activated()
{
		int cur_style = ui->combo_styles->currentIndex();

		QColor col_new = QColorDialog::getColor(m->styles[cur_style].col_list.colors[0], this);
		ui->btn_col1->setIcon(col2Icon(col_new));
		m->colors[0] = col_new;

		col_changed();
}

void GUI_StyleSettings::col2_activated()
{
	int cur_style = ui->combo_styles->currentIndex();

	QColor col_new = QColorDialog::getColor(m->styles[cur_style].col_list.colors[1], this);
	ui->btn_col2->setIcon(col2Icon(col_new));
	m->colors[1] = col_new;

	col_changed();
}

void GUI_StyleSettings::col3_activated()
{
	int cur_style = ui->combo_styles->currentIndex();

	QColor col_old(255, 255, 255);
	if(m->styles[cur_style].col_list.colors.size() > 2)col_old = m->styles[cur_style].col_list.colors[2];
	QColor col_new = QColorDialog::getColor(col_old, this);

	ui->btn_col3->setIcon(col2Icon(col_new));
	m->colors[2] = col_new;

	col_changed();
}


void GUI_StyleSettings::col4_activated()
{
	int cur_style = ui->combo_styles->currentIndex();

	QColor col_old(255, 255, 255);
	if(m->styles[cur_style].col_list.colors.size() > 3) col_old = m->styles[cur_style].col_list.colors[3];

	QColor col_new = QColorDialog::getColor(col_old, this);

	ui->btn_col4->setIcon(col2Icon(col_new));
	m->colors[3] = col_new;

	col_changed();
}


void GUI_StyleSettings::del_pressed()
{
	setWindowTitle(windowTitle().remove("*"));

	disconnect_combo_idx_changed();

	m->db->delete_raw_color_style(m->cur_text);
	init();

	connect_combo_idx_changed();

	m->cur_idx = ui->combo_styles->currentIndex();
	m->cur_text = ui->combo_styles->currentText();

	set_sth_changed(false);
	emit sig_style_update();
}

void GUI_StyleSettings::undo_pressed()
{
	disconnect_spinboxes();

	ui->sb_fading_steps_lv->setValue(m->cur_style.n_fading_steps_level);
	ui->sb_fading_steps_sp->setValue(m->cur_style.n_fading_steps_spectrum);
	ui->sb_h_spacing_lv->setValue(m->cur_style.hor_spacing_level);
	ui->sb_h_spacing_sp->setValue(m->cur_style.hor_spacing_spectrum);
	ui->sb_v_spacing_lv->setValue(m->cur_style.ver_spacing_level);
	ui->sb_v_spacing_sp->setValue(m->cur_style.ver_spacing_spectrum);
	ui->sb_rect_height_sp->setValue(m->cur_style.rect_height_spectrum);
	ui->sb_rect_width_lv->setValue(m->cur_style.rect_width_level);
	ui->sb_rect_height_lv->setValue(m->cur_style.rect_height_level);

	ui->cb_col3->setChecked(m->cur_style.col_list.colors.size() > 2 && m->cur_style.col_list.colors[2].isValid());
	ui->cb_col4->setChecked(m->cur_style.col_list.colors.size() > 3 && m->cur_style.col_list.colors[3].isValid());

	QList<QColor> col_list = m->cur_style.col_list.colors;
	ui->btn_col1->setIcon(col2Icon(col_list[0]));
	ui->btn_col2->setIcon(col2Icon(col_list[1]));

	if(col_list.size() > 2) {
		ui->btn_col3->setIcon(col2Icon(col_list[2]));
	}
	else  {
		ui->btn_col3->setIcon(col2Icon(QColor(0, 0, 0, 0)));
	}

	if(col_list.size() > 3) {
		ui->btn_col4->setIcon(col2Icon(col_list[3]));
	}
	else {
		ui->btn_col4->setIcon(col2Icon(QColor(0, 0, 0, 0)));
	}

	m->colors[0] = col_list[0];
	m->colors[1] = col_list[1];
	m->colors[2] = (col_list.size() > 2) ? col_list[2] : QColor(0,0,0,0);
	m->colors[3] = (col_list.size() > 3) ? col_list[3] : QColor(0,0,0,0);

	set_sth_changed(false);

	connect_spinboxes();
}


void GUI_StyleSettings::closeEvent(QCloseEvent * e)
{
	if(m->sth_changed)
	{
		Message::Answer ret = Message::question_yn(tr("Save changes?"));
		if(ret == Message::Answer::Yes) {
			save_pressed();

		}
	}

	Dialog::closeEvent(e);
}

void GUI_StyleSettings::spin_box_changed(int v)
{
	Q_UNUSED(v);
	set_sth_changed(true);
}

void GUI_StyleSettings::col_changed()
{
	set_sth_changed(true);
}

void GUI_StyleSettings::combo_text_changed(const QString & str)
{
	Q_UNUSED(str)
	if(m->cur_idx != ui->combo_styles->currentIndex()){
		return;
	}

	m->cur_text = ui->combo_styles->currentText();
}


void GUI_StyleSettings::set_sth_changed(bool b)
{
	ui->btn_undo->setEnabled(b);
	m->sth_changed = b;

	if(b)
	{
		if(!windowTitle().endsWith("*")) {
			setWindowTitle(windowTitle() + "*");
		}
	}

	else{
		setWindowTitle(windowTitle().remove("*"));
	}
}


void GUI_StyleSettings::show(int idx)
{
	if(isVisible()) {
		return;
	}

	set_sth_changed(false);
	showNormal();
	if(idx < m->styles.size() - 1){
		ui->combo_styles->setCurrentIndex(idx + 1);
	}
	else{
		ui->combo_styles->setCurrentIndex(0);
	}
}


