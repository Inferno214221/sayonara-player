/* GUI_StreamRecorderPreferences.cpp

 * Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Gui/Utils/Widgets/DirectoryChooser.h"

#include "Utils/Utils.h"
#include "Utils/FileSystem.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"

#include <QDate>
#include <QDir>
#include <QGridLayout>
#include <QTime>
#include <utility>

namespace
{
	MetaData createDemoTrack()
	{
		auto track = MetaData {};
		track.setTitle("Happy Song");
		track.setArtist("Al White");
		track.setAlbum("Rock Radio");
		track.setTrackNumber(1);

		return track;
	}
}

struct GUI_StreamRecorderPreferences::Private
{
	QString errorString;
	Util::FileSystemPtr fileSystem;

	Private(Util::FileSystemPtr fileSystem) :
		fileSystem {std::move(fileSystem)} {}
};

GUI_StreamRecorderPreferences::GUI_StreamRecorderPreferences(const QString& identifier,
                                                             const Util::FileSystemPtr& fileSystem) :
	Base(identifier),
	m {Pimpl::make<Private>(fileSystem)} {}

GUI_StreamRecorderPreferences::~GUI_StreamRecorderPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_StreamRecorderPreferences::initUi()
{
	setupParent(this, &ui);

	auto* layout = new QGridLayout(ui->buttonWidget);

	ui->buttonWidget->setLayout(layout);
	ui->leResultPath->setReadOnly(true);
	ui->leTemplate->setClearButtonEnabled(true);
	ui->leTemplate->setMouseTracking(true);
	ui->leTemplate->setStyleSheet("font-family: mono;");
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(1, GetSetting(Set::Engine_SR_SessionPath));

	const auto descriptions = StreamRecorder::Utils::descriptions();

	int i = 0;
	for(const auto& [key, description]: descriptions)
	{
		auto* btn = new TagButton(key, this);
		btn->setText(
			Util::stringToFirstUpper(description)
		);

		connect(btn, &QPushButton::clicked, this, [this, key = key]() {
			const auto oldPosition = ui->leTemplate->cursorPosition();

			ui->leTemplate->insert("<" + key + ">");
			ui->leTemplate->setFocus();
			ui->leTemplate->setCursorPosition(oldPosition + key.size() + 2);
			ui->leTemplate->setModified(true);
		});

		const auto row = i / 2;
		const auto col = i % 2;
		layout->addWidget(btn, row, col);

		i++;
	}

	revert();

	connect(ui->cbActivate, &QCheckBox::toggled, this, &GUI_StreamRecorderPreferences::activeToggled);
	connect(ui->btnPath, &QPushButton::clicked, this, &GUI_StreamRecorderPreferences::pathButtonClicked);
	connect(ui->leTemplate, &QLineEdit::textChanged, this, &GUI_StreamRecorderPreferences::lineEditChanged);
	connect(ui->lePath, &QLineEdit::textChanged, this, &GUI_StreamRecorderPreferences::lineEditChanged);
	connect(ui->cbCreateSessionPath, &QCheckBox::toggled, this, [this](const auto isEnabled) {
		ui->tabWidget->setTabEnabled(1, isEnabled);
	});

	connect(ui->btnDefault, &QPushButton::clicked, this, &GUI_StreamRecorderPreferences::defaultButtonClicked);
	connect(ui->btnUndo, &QPushButton::clicked, this, [this]() {
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
	if(ui)
	{
		ui->btnUndo->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
		ui->btnDefault->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
	}
}

QString GUI_StreamRecorderPreferences::errorString() const { return m->errorString; }

void GUI_StreamRecorderPreferences::activeToggled(bool b)
{
	ui->lePath->setEnabled(b);
	ui->btnPath->setEnabled(b);
	ui->cbAutoRecord->setEnabled(b);
	ui->cbCreateSessionPath->setEnabled(b);
	ui->leTemplate->setEnabled(b);

	const auto createSessionPath = GetSetting(Set::Engine_SR_SessionPath);
	ui->cbCreateSessionPath->setChecked(createSessionPath);
	ui->tabWidget->setTabEnabled(1, (b && createSessionPath));
}

void GUI_StreamRecorderPreferences::pathButtonClicked()
{
	const auto path = ui->lePath->text().isEmpty()
	                  ? QDir::homePath()
	                  : ui->lePath->text();

	const auto dir = Gui::DirectoryChooser::getDirectory(tr("Choose target directory"), path, true, this);
	if(!dir.isEmpty())
	{
		ui->lePath->setText(dir);
	}
}

void GUI_StreamRecorderPreferences::defaultButtonClicked()
{
	const QString defaultTemplate = StreamRecorder::Utils::targetPathTemplateDefault(true);
	ui->leTemplate->setText(defaultTemplate);
}

void GUI_StreamRecorderPreferences::lineEditChanged(const QString& /*newText*/)
{
	const auto templateText = ui->leTemplate->text();
	const auto demoTrack = createDemoTrack();

	int errorIndex; // NOLINT(cppcoreguidelines-init-variables)
	const auto err = StreamRecorder::Utils::validateTemplate(templateText, &errorIndex);
	if(err == StreamRecorder::Utils::ErrorCode::OK)
	{
		const auto targetPath = StreamRecorder::Utils::fullTargetPath(
			ui->lePath->text(),
			templateText,
			demoTrack,
			QDate::currentDate(),
			QTime::currentTime());

		ui->leResultPath->setText(targetPath.filename);
	}

	else
	{
		const auto errorString = StreamRecorder::Utils::parseErrorCode(err);
		const auto maxSelectedIndex = std::min(errorIndex + 5, templateText.size());

		ui->leResultPath->setText(
			QString("%1: '...%2...'")
				.arg(errorString)
				.arg(templateText.mid(errorIndex, maxSelectedIndex - errorIndex)));
	}
}

bool GUI_StreamRecorderPreferences::commit()
{
	auto everythingOk = true;

	const auto isActive = ui->cbActivate->isChecked();
	const auto path = ui->lePath->text();

	if(isActive)
	{
		if(!m->fileSystem->exists(path))
		{
			if(path.isEmpty())
			{
				m->errorString =
					tr("Target directory is empty").arg(path) + "\n" + tr("Please choose another directory");
				everythingOk = false;
			}

			else if(!QDir::root().mkpath(path))
			{
				m->errorString = tr("Cannot create %1").arg(path) + "\n" + tr("Please choose another directory");
				everythingOk = false;
			}
		}

		if(everythingOk)
		{
			int invalidIndex; // NOLINT(cppcoreguidelines-init-variables)
			const auto err = StreamRecorder::Utils::validateTemplate(ui->leTemplate->text().trimmed(), &invalidIndex);
			if(err != StreamRecorder::Utils::ErrorCode::OK)
			{
				m->errorString += tr("Template path is not valid") + "\n" + StreamRecorder::Utils::parseErrorCode(err);
				everythingOk = false;
			}
		}
	}

	if(everythingOk)
	{
		SetSetting(Set::Engine_SR_Active, ui->cbActivate->isChecked());
		SetSetting(Set::Engine_SR_Path, path);
		SetSetting(Set::Engine_SR_AutoRecord, ui->cbAutoRecord->isChecked());
		SetSetting(Set::Engine_SR_SessionPath, ui->cbCreateSessionPath->isChecked());
		SetSetting(Set::Engine_SR_SessionPathTemplate, ui->leTemplate->text().trimmed());
	}

	return everythingOk;
}

void GUI_StreamRecorderPreferences::revert()
{
	const auto isLameAvailable = GetSetting(SetNoDB::MP3enc_found);
	const auto path = GetSetting(Set::Engine_SR_Path);
	const auto active = GetSetting(Set::Engine_SR_Active) && isLameAvailable;
	const auto createSessionPath = GetSetting(Set::Engine_SR_SessionPath);
	const auto autoRecord = GetSetting(Set::Engine_SR_AutoRecord);

	auto templatePath = GetSetting(Set::Engine_SR_SessionPathTemplate);
	if(templatePath.isEmpty())
	{
		templatePath = StreamRecorder::Utils::targetPathTemplateDefault(true);
	}

	ui->cbActivate->setEnabled(isLameAvailable);
	ui->lePath->setText(path);
	ui->cbActivate->setChecked(active);
	ui->cbCreateSessionPath->setChecked(createSessionPath);
	ui->cbAutoRecord->setChecked(autoRecord);

	ui->cbCreateSessionPath->setEnabled(active);
	ui->btnPath->setEnabled(active);
	ui->lePath->setEnabled(active);
	ui->cbAutoRecord->setEnabled(active);
	ui->leTemplate->setEnabled(active);
	ui->leTemplate->setText(templatePath);

	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(1, active && createSessionPath);

	if(!isLameAvailable)
	{
		ui->labWarning->setText(Lang::get(Lang::CannotFindLame));
	}

	else
	{
		ui->labWarning->clear();
	}
}

QString GUI_StreamRecorderPreferences::actionName() const { return tr("Stream recorder"); }

struct TagButton::Private
{
	QString tagName;

	explicit Private(QString tagName) :
		tagName(std::move(tagName)) {}
};

TagButton::TagButton(const QString& tagName, QWidget* parent) :
	Gui::WidgetTemplate<QPushButton>(parent),
	m {Pimpl::make<Private>(tagName)} {}

TagButton::~TagButton() = default;

void TagButton::languageChanged()
{
	const auto descriptions = StreamRecorder::Utils::descriptions();
	for(const auto& description: descriptions)
	{
		if(description.first == m->tagName)
		{
			this->setText(Util::stringToFirstUpper(description.second));
			break;
		}
	}
}
