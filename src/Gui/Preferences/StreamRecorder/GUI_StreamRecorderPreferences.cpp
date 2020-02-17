/* GUI_StreamRecorderPreferences.cpp

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
 * created by Michael Lugmair (Lucio Carreras),
 * May 13, 2012
 *
 */

#include "GUI_StreamRecorderPreferences.h"
#include "Gui/Preferences/ui_GUI_StreamRecorderPreferences.h"
#include "Gui/Utils/Icons.h"

#include "Database/Connector.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/Logger/Logger.h"

#include <QFileDialog>
#include <QDir>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QGridLayout>

namespace SR=StreamRecorder;

struct GUI_StreamRecorderPreferences::Private
{
	QString errorString;
};

GUI_StreamRecorderPreferences::GUI_StreamRecorderPreferences(const QString& identifier) :
	Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_StreamRecorderPreferences::~GUI_StreamRecorderPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_StreamRecorderPreferences::initUi()
{
	setupParent(this, &ui);

	QGridLayout* layout = new QGridLayout(ui->buttonWidget);

	ui->buttonWidget->setLayout(layout);
	ui->leResultPath->setReadOnly(true);
	ui->leTemplate->setClearButtonEnabled(true);
	ui->leTemplate->setMouseTracking(true);
	ui->leTemplate->setStyleSheet("font-family: mono;");
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(1, GetSetting(Set::Engine_SR_SessionPath));

	QList<QPair<QString, QString>> desc = StreamRecorder::Utils::descriptions();

	int i=0;
	for(const QPair<QString, QString>& keyval : desc)
	{
		auto* btn = new TagButton(keyval.first, this);
		btn->setText(
			Util::stringToFirstUpper(keyval.second)
		);

		connect(btn, &QPushButton::clicked, this, [=]()
		{
			int old_position = ui->leTemplate->cursorPosition();

			ui->leTemplate->insert("<" + keyval.first + ">");
			ui->leTemplate->setFocus();
			ui->leTemplate->setCursorPosition(old_position + keyval.first.size() + 2);
			ui->leTemplate->setModified(true);
		});

		int row = i / 2;
		int col = i % 2;

		layout->addWidget(btn, row, col);
		i++;
	}

	revert();

	connect(ui->cbActivate, &QCheckBox::toggled, this, &GUI_StreamRecorderPreferences::activeToggled);
	connect(ui->btnPath, &QPushButton::clicked, this, &GUI_StreamRecorderPreferences::pathButtonClicked);
	connect(ui->leTemplate, &QLineEdit::textChanged, this, &GUI_StreamRecorderPreferences::lineEditChanged);
	connect(ui->lePath, &QLineEdit::textChanged, this, &GUI_StreamRecorderPreferences::lineEditChanged);
	connect(ui->cbCreateSessionPath, &QCheckBox::toggled, this, [=](bool b){
		ui->tabWidget->setTabEnabled(1, b);
	});

	connect(ui->btnDefault, &QPushButton::clicked, this, &GUI_StreamRecorderPreferences::defaultButtonClicked);
	connect(ui->btnUndo, &QPushButton::clicked, this, [=](){
		ui->leTemplate->undo();
	});

	lineEditChanged(ui->leTemplate->text());
}

void GUI_StreamRecorderPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->labActive->setText(Lang::get(Lang::Active));
	ui->btnUndo->setText(Lang::get(Lang::Undo));
	ui->btnDefault->setText(Lang::get(Lang::Default));
}

void GUI_StreamRecorderPreferences::skinChanged()
{
	if(!ui){
		return;
	}

	ui->btnUndo->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
	ui->btnDefault->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
}

QString GUI_StreamRecorderPreferences::errorString() const
{
	return m->errorString;
}


void GUI_StreamRecorderPreferences::activeToggled(bool b)
{
	ui->lePath->setEnabled(b);
	ui->btnPath->setEnabled(b);
	ui->cbAutoRecord->setEnabled(b);
	ui->cbCreateSessionPath->setEnabled(b);
	ui->leTemplate->setEnabled(b);

	bool create_session_path = GetSetting(Set::Engine_SR_SessionPath);
	ui->cbCreateSessionPath->setChecked(create_session_path);
	ui->tabWidget->setTabEnabled(1, (b && create_session_path));
}


void GUI_StreamRecorderPreferences::pathButtonClicked()
{
	QString path = ui->lePath->text();
	if(path.isEmpty()) {
		path = QDir::homePath();
	}

	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose target directory"), path, QFileDialog::ShowDirsOnly);
	if(dir.size() > 0) {
		ui->lePath->setText(dir);
	}
}

void GUI_StreamRecorderPreferences::defaultButtonClicked()
{
	QString default_template = SR::Utils::targetPathTemplateDefault(true);

	ui->leTemplate->setText(default_template);
}

void GUI_StreamRecorderPreferences::lineEditChanged(const QString& new_text)
{
	Q_UNUSED(new_text)

	QString template_text = ui->leTemplate->text();

	MetaData md;
	md.setTitle("Happy Song");
	md.setArtist("Al White");
	md.setAlbum("Rock Radio");
	md.setTrackNumber(1);

	int err_idx;
	SR::Utils::ErrorCode err = SR::Utils::validateTemplate(template_text, &err_idx);

	if(err == SR::Utils::ErrorCode::OK)
	{
		SR::Utils::TargetPaths target_path =
		SR::Utils::fullTargetPath(ui->lePath->text(),
									template_text,
									md,
									QDate::currentDate(),
									QTime::currentTime());

		ui->leResultPath->setText(target_path.first);
	}

	else
	{
		QString error_string = SR::Utils::parseErrorCode(err);

		int max_sel = std::min(err_idx + 5, template_text.size());
		ui->leResultPath->setText(
			error_string + ": '..." + template_text.mid(err_idx, max_sel - err_idx) + "...'"
		);

	}
}

bool GUI_StreamRecorderPreferences::commit()
{
	bool everything_ok = true;

	bool active = ui->cbActivate->isChecked();
	QString path = ui->lePath->text();

	if(active)
	{
		if(!::Util::File::exists(path))
		{
			if(path.isEmpty())
			{
				m->errorString = tr("Target directory is empty").arg(path) + "\n" + tr("Please choose another directory");
				everything_ok = false;
			}

			else if(!QDir::root().mkpath(path))
			{
				m->errorString = tr("Cannot create %1").arg(path) + "\n" + tr("Please choose another directory");
				everything_ok = false;
			}
		}

		if(everything_ok)
		{
			int invalid_idx;
			SR::Utils::ErrorCode err = SR::Utils::validateTemplate(ui->leTemplate->text().trimmed(), &invalid_idx);
			if(err != SR::Utils::ErrorCode::OK)
			{
				m->errorString += tr("Template path is not valid") + "\n" + SR::Utils::parseErrorCode(err);
				everything_ok = false;
			}
		}
	}

	if(everything_ok)
	{
		SetSetting(Set::Engine_SR_Active, ui->cbActivate->isChecked());
		SetSetting(Set::Engine_SR_Path, path);
		SetSetting(Set::Engine_SR_AutoRecord, ui->cbAutoRecord->isChecked());
		SetSetting(Set::Engine_SR_SessionPath, ui->cbCreateSessionPath->isChecked());
		SetSetting(Set::Engine_SR_SessionPathTemplate, ui->leTemplate->text().trimmed());
	}

	return everything_ok;
}

void GUI_StreamRecorderPreferences::revert()
{
	bool lame_available = GetSetting(SetNoDB::MP3enc_found);

	QString path = GetSetting(Set::Engine_SR_Path);
	QString template_path = GetSetting(Set::Engine_SR_SessionPathTemplate);
	bool active = GetSetting(Set::Engine_SR_Active) && lame_available;
	bool create_session_path = GetSetting(Set::Engine_SR_SessionPath);
	bool auto_rec = GetSetting(Set::Engine_SR_AutoRecord);

	if(template_path.isEmpty()){
		template_path = SR::Utils::targetPathTemplateDefault(true);
	}

	ui->cbActivate->setEnabled(lame_available);
	ui->lePath->setText(path);
	ui->cbActivate->setChecked(active);
	ui->cbCreateSessionPath->setChecked(create_session_path);
	ui->cbAutoRecord->setChecked(auto_rec);

	ui->cbCreateSessionPath->setEnabled(active);
	ui->btnPath->setEnabled(active);
	ui->lePath->setEnabled(active);
	ui->cbAutoRecord->setEnabled(active);
	ui->leTemplate->setEnabled(active);
	ui->leTemplate->setText(template_path);

	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(1, active && create_session_path);

	if(!lame_available){
		ui->labWarning->setText(Lang::get(Lang::CannotFindLame));
	}

	else {
		ui->labWarning->clear();
	}
}

QString GUI_StreamRecorderPreferences::actionName() const
{
	return tr("Stream recorder");
}

struct TagButton::Private
{
	QString tagName;

	Private(const QString& tagName) :
		tagName(tagName)
	{}
};

TagButton::TagButton(const QString& tagName, QWidget* parent) :
	Gui::WidgetTemplate<QPushButton>(parent)
{
	m = Pimpl::make<Private>(tagName);
}

TagButton::~TagButton() = default;

void TagButton::languageChanged()
{
	QList<QPair<QString, QString>> descs = SR::Utils::descriptions();
	for(const QPair<QString, QString>& d : descs)
	{
		if(d.first.compare(m->tagName) == 0)
		{
			this->setText(Util::stringToFirstUpper(d.second));
			break;
		}
	}
}
