/* GUI_PlaylistPreferences.cpp */

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


/* GUI_PlaylistPreferences.cpp */

#include "GUI_PlaylistPreferences.h"
#include "Gui/Preferences/ui_GUI_PlaylistPreferences.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QColorDialog>
#include <QPainter>
#include <QPalette>

namespace
{
	QStringList extractExpressionsBetweenPercents(const QString& expr)
	{
		QStringList result;

		auto re = QRegExp("%(.*)%");
		re.setMinimal(true);

		auto idx = re.indexIn(expr);
		while((idx >= 0) && (idx < expr.size()))
		{
			const auto cap = re.cap(1);
			result << cap;
			idx = re.indexIn(expr, idx + 2 + cap.size());
		}

		return result;
	}

	bool evaluateExpression(const QString& expr)
	{
		if(expr.isEmpty())
		{
			return false;
		}

		auto starCount = 0;
		auto apostrophCount = 0;
		auto percentCount = 0;
		for(const auto& c: expr)
		{
			if(c == '\'')
			{
				apostrophCount++;
			}

			else if(c == '*')
			{
				starCount++;
			}

			else if(c == '%')
			{
				percentCount++;
			}
		}

		if((apostrophCount % 2 == 1) ||
		   (percentCount % 2 == 1) ||
		   (starCount % 2 == 1))
		{
			return false;
		}

		const auto betweenPercents = extractExpressionsBetweenPercents(expr);

		const auto isIncorrect = Util::Algorithm::contains(betweenPercents, [](const auto& betweenPercent) {
			return ((betweenPercent != QStringLiteral("nr")) &&
			        (betweenPercent != QStringLiteral("title")) &&
			        (betweenPercent != QStringLiteral("artist")) &&
			        (betweenPercent != QStringLiteral("filename")) &&
			        (betweenPercent != QStringLiteral("album")));
		});

		return (!isIncorrect);
	}

	void applyColorToButton(QPushButton* button, const QColor& color, const QPalette& standardPalette)
	{
		const auto newColor = color.isValid()
		                      ? color
		                      : standardPalette.color(QPalette::ColorGroup::Active, QPalette::ColorRole::WindowText);

		const auto colorName = newColor.name(QColor::NameFormat::HexRgb);
		const auto stylesheet = QString("color: %1").arg(colorName);

		button->setText(colorName);
		button->setStyleSheet(stylesheet);
	}
}

GUI_PlaylistPreferences::GUI_PlaylistPreferences(const QString& identifier) :
	Base(identifier) {}

GUI_PlaylistPreferences::~GUI_PlaylistPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

bool GUI_PlaylistPreferences::commit()
{
	SetSetting(Set::PL_LoadSavedPlaylists, ui->cbLoadSavedPlaylists->isChecked());
	SetSetting(Set::PL_LoadTemporaryPlaylists, ui->cbLoadTemporaryPlaylists->isChecked());
	SetSetting(Set::PL_LoadLastTrack, (ui->cbLoadLastTrack->isChecked() && ui->cbLoadLastTrack->isEnabled()));
	SetSetting(Set::PL_RememberTime, (ui->cbRememberTime->isChecked() && ui->cbRememberTime->isEnabled()));
	SetSetting(Set::PL_StartPlaying, (ui->cbStartPlaying->isChecked() && ui->cbStartPlaying->isEnabled()));

	SetSetting(Set::PL_ShowNumbers, ui->cbShowNumbers->isChecked());
	SetSetting(Set::PL_ShowCovers, ui->cbShowCovers->isChecked());
	SetSetting(Set::PL_ShowRating, ui->cbShowRating->isChecked());
	SetSetting(Set::PL_ShowBottomBar, ui->cbShowBottomBar->isChecked());

	SetSetting(Set::PL_ShowClearButton, ui->cbShowClearButton->isChecked());
	SetSetting(Set::PL_RememberTrackAfterStop, ui->cbRememberAfterStop->isChecked());

	const auto hasCustomColorStandard = ui->cbCustomColorStandard->isChecked();
	SetSetting(Set::PL_CurrentTrackCustomColorStandard, hasCustomColorStandard);
	SetSetting(Set::PL_CurrentTrackColorStringStandard,
	           hasCustomColorStandard ? ui->btnCustomColorStandard->text() : QString());

	const auto hasCustomColorDark = ui->cbCustomColorDark->isChecked();
	SetSetting(Set::PL_CurrentTrackCustomColorDark, hasCustomColorDark);
	SetSetting(Set::PL_CurrentTrackColorStringDark, hasCustomColorDark ? ui->btnCustomColorDark->text() : QString());
	SetSetting(Set::PL_JumpToCurrentTrack, ui->cbJumpToCurrentTrack->isChecked());
	SetSetting(Set::PL_PlayTrackAfterSearch, ui->cbPlayTrackAfterSearch->isChecked());
	SetSetting(Set::PL_StartPlayingWorkaround_Issue263, ui->cb_startupPlaybackWorkaround263->isChecked());

	SetSetting(Set::PL_CreateFilesystemPlaylist, ui->cbCreateFileystemPlaylist->isChecked());
	SetSetting(Set::PL_FilesystemPlaylistName, ui->leFilesystemPlaylistName->text());
	SetSetting(Set::PL_SpecifyFileystemPlaylistName,
	           ui->cbChooseFilesystemPlaylistName->isVisible() &&
	           ui->cbChooseFilesystemPlaylistName->isChecked());
	SetSetting(Set::PL_ShowConfirmationOnClose, ui->cbConfirmOnClose->isChecked());
	SetSetting(Set::PL_ModificatorAllowRearrangeMethods, ui->cbAllowRearrange->isChecked());
	SetSetting(Set::PL_ModificatorAllowDynamicPlayback, ui->cbAllowDynamicPlayback->isChecked());

	const auto success = evaluateExpression(ui->leExpression->text());
	if(success)
	{
		SetSetting(Set::PL_EntryLook, ui->leExpression->text());
	}

	return success;
}

void GUI_PlaylistPreferences::revert()
{
	const auto loadSavedPlaylists = GetSetting(Set::PL_LoadSavedPlaylists);
	const auto loadTemporaryPlaylists = GetSetting(Set::PL_LoadTemporaryPlaylists);
	const auto loadLastTrack = GetSetting(Set::PL_LoadLastTrack);
	const auto rememberTime = GetSetting(Set::PL_RememberTime);
	const auto startPlaying = GetSetting(Set::PL_StartPlaying);

	ui->cbLoadSavedPlaylists->setChecked(loadSavedPlaylists);
	ui->cbLoadTemporaryPlaylists->setChecked(loadTemporaryPlaylists);
	ui->cbLoadLastTrack->setChecked(loadLastTrack);
	ui->cbRememberTime->setChecked(rememberTime);
	ui->cbStartPlaying->setChecked(startPlaying);

	ui->leExpression->setText(GetSetting(Set::PL_EntryLook));
	ui->cbShowNumbers->setChecked(GetSetting(Set::PL_ShowNumbers));
	ui->cbShowCovers->setChecked(GetSetting(Set::PL_ShowCovers));
	ui->cbShowRating->setChecked(GetSetting(Set::PL_ShowRating));
	ui->cbShowClearButton->setChecked(GetSetting(Set::PL_ShowClearButton));
	ui->cbRememberAfterStop->setChecked(GetSetting(Set::PL_RememberTrackAfterStop));
	ui->cbShowBottomBar->setChecked(GetSetting(Set::PL_ShowBottomBar));

	ui->cbCustomColorStandard->setChecked(GetSetting(Set::PL_CurrentTrackCustomColorStandard));
	ui->btnCustomColorStandard->setVisible(GetSetting(Set::PL_CurrentTrackCustomColorStandard));
	applyColorToButton(ui->btnCustomColorStandard,
	                   QColor(GetSetting(Set::PL_CurrentTrackColorStringStandard)),
	                   palette());

	ui->cbCustomColorDark->setChecked(GetSetting(Set::PL_CurrentTrackCustomColorDark));
	ui->btnCustomColorDark->setVisible(GetSetting(Set::PL_CurrentTrackCustomColorDark));
	applyColorToButton(ui->btnCustomColorDark, QColor(GetSetting(Set::PL_CurrentTrackColorStringDark)), palette());

	ui->cbJumpToCurrentTrack->setChecked(GetSetting(Set::PL_JumpToCurrentTrack));
	ui->cbPlayTrackAfterSearch->setChecked(GetSetting(Set::PL_PlayTrackAfterSearch));
	ui->cb_startupPlaybackWorkaround263->setChecked(GetSetting(Set::PL_StartPlayingWorkaround_Issue263));

	ui->cbCreateFileystemPlaylist->setChecked(GetSetting(Set::PL_CreateFilesystemPlaylist));
	ui->cbChooseFilesystemPlaylistName->setVisible(ui->cbCreateFileystemPlaylist->isChecked());
	ui->cbChooseFilesystemPlaylistName->setChecked(GetSetting(Set::PL_SpecifyFileystemPlaylistName));
	ui->leFilesystemPlaylistName->setVisible(ui->cbChooseFilesystemPlaylistName->isChecked());
	ui->leFilesystemPlaylistName->setText(GetSetting(Set::PL_FilesystemPlaylistName));
	ui->cbConfirmOnClose->setChecked(GetSetting(Set::PL_ShowConfirmationOnClose));
	ui->cbAllowDynamicPlayback->setChecked(GetSetting(Set::PL_ModificatorAllowDynamicPlayback));
	ui->cbAllowRearrange->setChecked(GetSetting(Set::PL_ModificatorAllowRearrangeMethods));
}

void GUI_PlaylistPreferences::initUi()
{
	if(isUiInitialized())
	{
		return;
	}

	setupParent(this, &ui);
	ui->tabWidget->setCurrentIndex(0);
	ui->leExpression->setStyleSheet("font-family: mono;");
	ui->widgetTemplateHelp->setVisible(false);

	revert();

	checkboxToggled(true);

	connect(ui->cbLoadLastTrack, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbLoadSavedPlaylists, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbLoadTemporaryPlaylists, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbRememberTime, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbStartPlaying, &QCheckBox::toggled, this, &GUI_PlaylistPreferences::checkboxToggled);
	connect(ui->cbCustomColorStandard, &QCheckBox::toggled, ui->btnCustomColorStandard, &QWidget::setVisible);
	connect(ui->cbCustomColorDark, &QCheckBox::toggled, ui->btnCustomColorDark, &QWidget::setVisible);
	connect(ui->btnCustomColorStandard, &QPushButton::clicked, this, &GUI_PlaylistPreferences::chooseColorClicked);
	connect(ui->btnCustomColorDark, &QPushButton::clicked, this, &GUI_PlaylistPreferences::chooseColorClicked);

	connect(ui->btnDefault, &QPushButton::clicked, this, [&]() {
		ui->leExpression->setText("*%title%* - %artist%");
	});
	connect(ui->btnTemplateHelp, &QPushButton::clicked, this, [&]() {
		ui->widgetTemplateHelp->setVisible(!ui->widgetTemplateHelp->isVisible());
	});

	connect(ui->cbChooseFilesystemPlaylistName, &QCheckBox::toggled,
	        ui->leFilesystemPlaylistName, &QWidget::setVisible);
	connect(ui->cbCreateFileystemPlaylist, &QCheckBox::toggled, this, [&](const auto b) {
		ui->cbChooseFilesystemPlaylistName->setVisible(b);
		ui->leFilesystemPlaylistName->setVisible(
			ui->cbChooseFilesystemPlaylistName->isVisible() &&
			ui->cbChooseFilesystemPlaylistName->isChecked());
	});
}

QString GUI_PlaylistPreferences::actionName() const
{
	return Lang::get(Lang::Playlist);
}

void GUI_PlaylistPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->labAlbum->setText(Lang::get(Lang::Album));
	ui->labArtist->setText(Lang::get(Lang::Artist));
	ui->labTitle->setText(Lang::get(Lang::Title));
	ui->labTrackNumber->setText(Lang::get(Lang::TrackNo));
	ui->btnDefault->setText(Lang::get(Lang::Default));
	ui->leFilesystemPlaylistName->setPlaceholderText(Lang::get(Lang::Files));

	const auto workaroundText = tr("Fix startup playback issue") + " " + "(#263)";
	ui->cb_startupPlaybackWorkaround263->setText(workaroundText);
}

void GUI_PlaylistPreferences::skinChanged()
{
	if(ui)
	{
		ui->btnDefault->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
	}
}

QString GUI_PlaylistPreferences::errorString() const
{
	return tr("Playlist look: Invalid expression");
}

void GUI_PlaylistPreferences::checkboxToggled([[maybe_unused]] bool b)
{
	const auto load = (ui->cbLoadSavedPlaylists->isChecked() || ui->cbLoadTemporaryPlaylists->isChecked());

	ui->cbLoadLastTrack->setEnabled(load);
	ui->cbRememberTime->setEnabled(load);
	ui->cbStartPlaying->setEnabled(load);

	const auto loadLastTrack = ui->cbLoadLastTrack->isChecked() && ui->cbLoadLastTrack->isEnabled();
	ui->cbRememberTime->setEnabled(loadLastTrack);
}

void GUI_PlaylistPreferences::chooseColorClicked()
{
	if(auto* button = dynamic_cast<QPushButton*>(sender()); button != nullptr)
	{
		const auto currentColor = QColor(button->text());
		const auto newColor = QColorDialog::getColor(currentColor.isValid() ? currentColor : Qt::black, this);
		if(newColor.isValid())
		{
			applyColorToButton(button, newColor, palette());
		}
	}
}
