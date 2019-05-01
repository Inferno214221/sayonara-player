/* GUI_StreamRecorder.cpp

 * Copyright (C) 2011-2019 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * May 13, 2012
 *
 */

#include "GUI_StreamRecorder.h"
#include "GUI/Preferences/ui_GUI_StreamRecorder.h"

#include "Database/Connector.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/Logger/Logger.h"


#include <QFileDialog>
#include <QDir>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QGridLayout>

namespace SR=StreamRecorder;

struct GUI_StreamRecorder::Private
{
	QString error_string;
};

GUI_StreamRecorder::GUI_StreamRecorder(const QString& identifier) :
	Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_StreamRecorder::~GUI_StreamRecorder()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_StreamRecorder::init_ui()
{
	setup_parent(this, &ui);

	QGridLayout* layout = new QGridLayout(ui->button_widget);

	ui->button_widget->setLayout(layout);
	ui->le_result_path->setReadOnly(true);
	ui->le_template->setClearButtonEnabled(true);
	ui->le_template->setMouseTracking(true);
	ui->le_template->setStyleSheet("font-family: mono;");
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(1, GetSetting(Set::Engine_SR_SessionPath));

	QList<QPair<QString, QString>> desc = StreamRecorder::Utils::descriptions();

	int i=0;
	for(const QPair<QString, QString>& keyval : desc)
	{
		TagButton* btn = new TagButton(keyval.first, this);
		btn->setText(
			Util::cvt_str_to_first_upper(keyval.second)
		);

		connect(btn, &QPushButton::clicked, this, [=]()
		{
			int old_position = ui->le_template->cursorPosition();

			ui->le_template->insert("<" + keyval.first + ">");
			ui->le_template->setFocus();
			ui->le_template->setCursorPosition(old_position + keyval.first.size() + 2);
			ui->le_template->setModified(true);
		});

		int row = i / 2;
		int col = i % 2;

		layout->addWidget(btn, row, col);
		i++;
	}

	revert();

	connect(ui->cb_activate, &QCheckBox::toggled, this, &GUI_StreamRecorder::sl_cb_activate_toggled);
	connect(ui->btn_path, &QPushButton::clicked, this, &GUI_StreamRecorder::sl_btn_path_clicked);
	connect(ui->le_template, &QLineEdit::textChanged, this, &GUI_StreamRecorder::sl_line_edit_changed);
	connect(ui->le_path, &QLineEdit::textChanged, this, &GUI_StreamRecorder::sl_line_edit_changed);
	connect(ui->cb_create_session_path, &QCheckBox::toggled, this, [=](bool b){
		ui->tabWidget->setTabEnabled(1, b);
	});

	connect(ui->btn_default, &QPushButton::clicked, this, &GUI_StreamRecorder::sl_btn_default_clicked);
	connect(ui->btn_undo, &QPushButton::clicked, this, [=](){
		ui->le_template->undo();
	});

	sl_line_edit_changed(ui->le_template->text());
}

void GUI_StreamRecorder::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->lab_active->setText(Lang::get(Lang::Active));
	ui->btn_undo->setText(Lang::get(Lang::Undo));
	ui->btn_default->setText(Lang::get(Lang::Default));
}

QString GUI_StreamRecorder::error_string() const
{
	return m->error_string;
}


void GUI_StreamRecorder::sl_cb_activate_toggled(bool b)
{
	ui->le_path->setEnabled(b);
	ui->btn_path->setEnabled(b);
	ui->cb_auto_rec->setEnabled(b);
	ui->cb_create_session_path->setEnabled(b);
	ui->le_template->setEnabled(b);

	bool create_session_path = GetSetting(Set::Engine_SR_SessionPath);
	ui->cb_create_session_path->setChecked(create_session_path);
	ui->tabWidget->setTabEnabled(1, (b && create_session_path));
}


void GUI_StreamRecorder::sl_btn_path_clicked()
{
	QString path = ui->cb_create_session_path->text();
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose target directory"), path, QFileDialog::ShowDirsOnly);
	if(dir.size() > 0) {
		ui->le_path->setText(dir);
	}
}

void GUI_StreamRecorder::sl_btn_default_clicked()
{
	QString default_template = SR::Utils::target_path_template_default(true);

	ui->le_template->setText(default_template);
}

void GUI_StreamRecorder::sl_line_edit_changed(const QString& new_text)
{
	Q_UNUSED(new_text)

	QString template_text = ui->le_template->text();

	MetaData md;
	md.set_title("Happy Song");
	md.set_artist("Al White");
	md.set_album("Rock Radio");
	md.track_num = 1;

	int err_idx;
	SR::Utils::ErrorCode err = SR::Utils::validate_template(template_text, &err_idx);

	if(err == SR::Utils::ErrorCode::OK)
	{
		SR::Utils::TargetPaths target_path =
		SR::Utils::full_target_path(ui->le_path->text(),
									template_text,
									md,
									QDate::currentDate(),
									QTime::currentTime());

		ui->le_result_path->setText(target_path.first);
	}

	else
	{
		QString error_string = SR::Utils::parse_error_code(err);

		int max_sel = std::min(err_idx + 5, template_text.size());
		ui->le_result_path->setText(
			error_string + ": '..." + template_text.mid(err_idx, max_sel - err_idx) + "...'"
		);

	}
}

bool GUI_StreamRecorder::commit()
{
	bool everything_ok = true;

	bool active = ui->cb_activate->isChecked();
	QString path = ui->le_path->text();

	if(active)
	{
		if(!::Util::File::exists(path))
		{
			if(path.isEmpty())
			{
				m->error_string = tr("Target path is empty").arg(path) + "\n" + tr("Please choose another directory");
				everything_ok = false;
			}

			else if(!QDir::root().mkpath(path))
			{
				m->error_string = tr("Cannot create %1").arg(path) + "\n" + tr("Please choose another directory");
				everything_ok = false;
			}
		}

		if(everything_ok)
		{
			int invalid_idx;
			SR::Utils::ErrorCode err = SR::Utils::validate_template(ui->le_template->text().trimmed(), &invalid_idx);
			if(err != SR::Utils::ErrorCode::OK)
			{
				m->error_string += tr("Template path is not valid") + "\n" + SR::Utils::parse_error_code(err);
				everything_ok = false;
			}
		}
	}

	if(everything_ok)
	{
		SetSetting(Set::Engine_SR_Active, ui->cb_activate->isChecked());
		SetSetting(Set::Engine_SR_Path, path);
		SetSetting(Set::Engine_SR_AutoRecord, ui->cb_auto_rec->isChecked());
		SetSetting(Set::Engine_SR_SessionPath, ui->cb_create_session_path->isChecked());
		SetSetting(Set::Engine_SR_SessionPathTemplate, ui->le_template->text().trimmed());
	}

	return everything_ok;
}

void GUI_StreamRecorder::revert()
{
	bool lame_available = GetSetting(SetNoDB::MP3enc_found);

	QString path = GetSetting(Set::Engine_SR_Path);
	QString template_path = GetSetting(Set::Engine_SR_SessionPathTemplate);
	bool active = GetSetting(Set::Engine_SR_Active) && lame_available;
	bool create_session_path = GetSetting(Set::Engine_SR_SessionPath);
	bool auto_rec = GetSetting(Set::Engine_SR_AutoRecord);

	if(template_path.isEmpty()){
		template_path = SR::Utils::target_path_template_default(true);
	}

	ui->cb_activate->setEnabled(lame_available);
	ui->le_path->setText(path);
	ui->cb_activate->setChecked(active);
	ui->cb_create_session_path->setChecked(create_session_path);
	ui->cb_auto_rec->setChecked(auto_rec);

	ui->cb_create_session_path->setEnabled(active);
	ui->btn_path->setEnabled(active);
	ui->le_path->setEnabled(active);
	ui->cb_auto_rec->setEnabled(active);
	ui->le_template->setEnabled(active);
	ui->le_template->setText(template_path);

	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(1, active && create_session_path);

	if(!lame_available){
		ui->lab_warning->setText(Lang::get(Lang::CannotFindLame));
	}

	else {
		ui->lab_warning->clear();
	}
}

QString GUI_StreamRecorder::action_name() const
{
	return tr("Stream recorder");
}

struct TagButton::Private
{
	QString tag_name;

	Private(const QString& tag_name) :
		tag_name(tag_name)
	{}
};

TagButton::TagButton(const QString& tag_name, QWidget* parent) :
	Gui::WidgetTemplate<QPushButton>(parent)
{
	m = Pimpl::make<Private>(tag_name);
}

TagButton::~TagButton() {}

void TagButton::language_changed()
{
	QList<QPair<QString, QString>> descs = SR::Utils::descriptions();
	for(const QPair<QString, QString>& d : descs)
	{
		if(d.first.compare(m->tag_name) == 0){
			this->setText(Util::cvt_str_to_first_upper(d.second));
			break;
		}
	}
}
