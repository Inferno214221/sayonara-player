/* GUI_IconPreferences.cpp */

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

#include "GUI_IconPreferences.h"
#include "Gui/Preferences/ui_GUI_IconPreferences.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>
#include <QIcon>
#include <QDir>
#include <QRadioButton>

namespace
{
	QString theme(QRadioButton* radioButton)
	{
		return radioButton->property("theme").toString();
	}

	void setTheme(QRadioButton* radioButton, const QString& theme)
	{
		radioButton->setProperty("theme", theme);
	}

	void setBold(QWidget* widget, const bool b)
	{
		widget->setStyleSheet(b ? "font-weight: bold;" : "font-weight: normal;");
	}

	QString systemThemeText()
	{
		return QString("%1 (%2)")
			.arg(QObject::tr("System theme"))
			.arg(Gui::Icons::defaultSystemTheme());
	}

	QRadioButton*
	addRadioButton(QWidget* widget, const QString& text, const QString& themeName, const bool isSystemTheme)
	{
		QIcon::setThemeName(themeName);
		const auto icon = QIcon::fromTheme("media-playback-start");
		if(icon.isNull() && !isSystemTheme)
		{
			return nullptr;
		}

		auto* radioButton = new QRadioButton(text, widget);
		if(!isSystemTheme)
		{
			setTheme(radioButton, themeName);
		}

		return radioButton;
	}

	QRadioButton* setupSystemThemeRadioButton(QWidget* parent)
	{
		auto* radioButton = addRadioButton(parent, systemThemeText(), {}, true);
		setBold(radioButton, true);
		parent->layout()->addWidget(radioButton);

		return radioButton;
	}

	QRadioButton* addStandardThemeRadioButton(QWidget* parent, const QString& theme, const bool isCurrentTheme)
	{
		const auto systemTheme = Gui::Icons::defaultSystemTheme();
		if(theme.isEmpty() || (theme == systemTheme))
		{
			return nullptr;
		}

		auto* radioButton = addRadioButton(parent, theme, theme, false);
		if(radioButton)
		{
			auto* layout = parent->layout();
			layout->addWidget(radioButton);
			setBold(radioButton, isCurrentTheme);
		}

		return radioButton;
	}

	QStringList setupThemeSearchPaths()
	{
		auto iconPaths = QIcon::themeSearchPaths();
		iconPaths.sort();
		QIcon::setThemeSearchPaths(iconPaths);

		return iconPaths;
	}

	void applyIcon(const QString& iconName, const QString& themeName, QLabel* label)
	{
		constexpr const auto Size = 32;
		QIcon::setThemeName(themeName);

		const auto icon = QIcon::fromTheme(iconName);
		const auto pixmap = icon.pixmap(Size, Size)
			.scaled(Size, Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		label->setPixmap(pixmap);
	}
}

struct GUI_IconPreferences::Private
{
	QList<QRadioButton*> radioButtons;

	QString systemTheme {Gui::Icons::defaultSystemTheme()};
	QString originalTheme {GetSetting(Set::Icon_Theme)};
};

GUI_IconPreferences::GUI_IconPreferences(QWidget* parent) :
	Gui::Widget(parent),
	m {Pimpl::make<Private>()} {}

GUI_IconPreferences::~GUI_IconPreferences() = default;

QString GUI_IconPreferences::actionName() const { return tr("Icons"); }

bool GUI_IconPreferences::commit()
{
	if(!ui)
	{
		return true;
	}
	
	for(auto* radioButton: m->radioButtons)
	{
		if(radioButton->isChecked())
		{
			SetSetting(Set::Icon_Theme, theme(radioButton));
			m->originalTheme = theme(radioButton);

			Gui::Icons::changeTheme();
		}

		setBold(radioButton, radioButton->isChecked());
	}

	const auto forceStandardIcons = ui->cbAlsoUseInDarkStyle->isChecked();
	SetSetting(Set::Icon_ForceInDarkTheme, forceStandardIcons);

	Set::shout<Set::Player_Style>();

	return true;
}

void GUI_IconPreferences::revert()
{
	if(ui)
	{
		for(auto* radioButton: m->radioButtons)
		{
			const auto isCurrentTheme = (theme(radioButton) == m->originalTheme);
			radioButton->setChecked(isCurrentTheme);
		}

		const auto b = GetSetting(Set::Icon_ForceInDarkTheme);
		ui->cbAlsoUseInDarkStyle->setChecked(b);

		QIcon::setThemeName(m->originalTheme);
	}
}

void GUI_IconPreferences::themeChanged(const QString& theme)
{
	applyIcon("media-playback-start", theme, ui->labPlay);
	applyIcon("media-skip-backward", theme, ui->labPrevious);
	applyIcon("media-skip-forward", theme, ui->labForward);
	applyIcon("media-playback-stop", theme, ui->labStop);
}

void GUI_IconPreferences::radioButtonToggled(const bool b)
{
	const auto radioButton = dynamic_cast<QRadioButton*>(sender());
	if(b && radioButton)
	{
		themeChanged(theme(radioButton));
	}
}

void GUI_IconPreferences::initUi()
{
	if(ui)
	{
		return;
	}

	ui = std::make_shared<Ui::GUI_IconPreferences>();
	ui->setupUi(this);

	auto* scrollWidget = ui->scrollAreaWidget;
	scrollWidget->setObjectName("IconThemeScrollWidget");

	auto* rbSystemTheme = setupSystemThemeRadioButton(scrollWidget);
	connect(rbSystemTheme, &QRadioButton::toggled, this, &GUI_IconPreferences::radioButtonToggled);

	auto iconPaths = setupThemeSearchPaths();
	for(const auto& iconPath: iconPaths)
	{
		const auto subdirs = QDir(iconPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for(const auto& subdir: subdirs)
		{
			const auto isCurrentTheme = (m->originalTheme == QIcon::themeName());
			auto* radioButton = addStandardThemeRadioButton(ui->scrollAreaWidget, subdir, isCurrentTheme);
			if(radioButton)
			{
				connect(radioButton, &QRadioButton::toggled, this, &GUI_IconPreferences::radioButtonToggled);

				m->radioButtons << radioButton;
			}
		}
	}

	revert();
}

void GUI_IconPreferences::languageChanged()
{
	const auto index = Util::Algorithm::indexOf(m->radioButtons, [](auto* radioButton) {
		return theme(radioButton).isEmpty();
	});

	if(index >= 0)
	{
		auto* rb = m->radioButtons[index];
		rb->setText(systemThemeText());
	}
}

void GUI_IconPreferences::showEvent(QShowEvent* e)
{
	initUi();
	Gui::Widget::showEvent(e);
}
