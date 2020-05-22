/* GUI_IconPreferences.cpp */

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

namespace Algorithm=Util::Algorithm;

class IconRadioButton : public QRadioButton
{
private:
	QString mTheme;

public:
	IconRadioButton(const QString& text, QWidget* parent);
	~IconRadioButton();

	void setTheme(const QString& theme);
	QString theme() const;
};

static
IconRadioButton* addRadioButton(QWidget* widget, const QString& text, const QString& themeName, bool isSystemTheme)
{
	QIcon::setThemeName(themeName);
	QIcon icon = QIcon::fromTheme("media-playback-start");
	if(icon.isNull() && !isSystemTheme){
		return nullptr;
	}

	auto* rb = new IconRadioButton(text, widget);
	if(!isSystemTheme){
		rb->setTheme(themeName);
	}

	return rb;
}

struct GUI_IconPreferences::Private
{
	QList<IconRadioButton*> radioButtons;

	QString systemTheme;
	QString originalTheme;

	Private()
	{
		systemTheme = Gui::Icons::systemTheme();
		originalTheme = GetSetting(Set::Icon_Theme);
	}
};

GUI_IconPreferences::GUI_IconPreferences(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_IconPreferences::~GUI_IconPreferences() = default;

QString GUI_IconPreferences::actionName() const
{
	return tr("Icons");
}

bool GUI_IconPreferences::commit()
{
	if(!ui) {
		return true;
	}

	for(IconRadioButton* rb : m->radioButtons)
	{
		rb->setStyleSheet("font-weight: normal;");
		if(rb->isChecked())
		{
			SetSetting(Set::Icon_Theme, rb->theme());
			rb->setStyleSheet("font-weight: bold;");
			m->originalTheme = rb->theme();

			Gui::Icons::changeTheme();
		}
	}

	bool forceStandardIcons = ui->cbAlsoUseInDarkStyle->isChecked();
	Gui::Icons::forceStandardIcons(forceStandardIcons);
	SetSetting(Set::Icon_ForceInDarkTheme, forceStandardIcons);

	Set::shout<SetNoDB::Player_MetaStyle>();

	return true;
}

void GUI_IconPreferences::revert()
{
	if(!ui) {
		return;
	}

	for(IconRadioButton* rb : m->radioButtons)
	{
		rb->setChecked(rb->theme() == m->originalTheme);
	}

	bool b = GetSetting(Set::Icon_ForceInDarkTheme);
	ui->cbAlsoUseInDarkStyle->setChecked(b);

	QIcon::setThemeName(m->originalTheme);
}

static void applyIcon(const QString& iconName, const QString& themeName, QLabel* label)
{
	QIcon::setThemeName(themeName);
	QIcon icon = QIcon::fromTheme(iconName);
	label->setPixmap(icon.pixmap(32, 32)
		.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void GUI_IconPreferences::themeChanged(const QString& theme)
{
	applyIcon("media-playback-start", theme, ui->labPlay);
	applyIcon("media-skip-backward", theme, ui->labPrevious);
	applyIcon("media-skip-forward", theme, ui->labForward);
	applyIcon("media-playback-stop", theme, ui->labStop);
}

void GUI_IconPreferences::radioButtonToggled(bool b)
{
	auto rb = static_cast<IconRadioButton*>(sender());
	if(b && rb)
	{
		this->themeChanged(rb->theme());
	}
}

void GUI_IconPreferences::initUi()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_IconPreferences();
	ui->setupUi(this);

	QWidget* scrollWidget = ui->scrollAreaWidget;
	scrollWidget->setObjectName("IconThemeScrollWidget");

	QLayout* layout = scrollWidget->layout();
	{ // system theme
		IconRadioButton* rbSystemTheme = addRadioButton
		(
			scrollWidget,
			tr("System theme") + " (" + m->systemTheme + ")",
			QString(),
			true
		);

		rbSystemTheme->setStyleSheet("font-weight: bold;");
		connect(rbSystemTheme, &QRadioButton::toggled, this, &GUI_IconPreferences::radioButtonToggled);
		layout->addWidget(rbSystemTheme);

		m->radioButtons << rbSystemTheme;
	}

	QStringList iconPaths = QIcon::themeSearchPaths();
	Algorithm::sort(iconPaths, [](const QString& s1, const QString& s2){
		return (s1.size() < s2.size() || s1 < s2);
	});
	QIcon::setThemeSearchPaths(iconPaths);

	for(const QString& iconPath : Algorithm::AsConst(iconPaths))
	{
		const QDir d(iconPath);
		const QStringList subdirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		if(subdirs.isEmpty()){
			continue;
		}

		for(const QString& subdir : subdirs)
		{
			if(subdir.isEmpty() || (subdir == m->systemTheme)) {
				continue;
			}

			IconRadioButton* rb = addRadioButton(scrollWidget, subdir, subdir, false);
			if(!rb) {
				continue;
			}

			connect(rb, &QRadioButton::toggled, this, &GUI_IconPreferences::radioButtonToggled);

			if(m->originalTheme == QIcon::themeName()) {
				rb->setStyleSheet("font-weight: bold;");
			}

			layout->addWidget(rb);
			m->radioButtons << rb;
		}
	}

	revert();
}

void GUI_IconPreferences::languageChanged()
{
	auto itSystemTheme = Util::Algorithm::find(m->radioButtons, [](IconRadioButton* rb) {
		return (rb->theme().isEmpty());
	});

	if(itSystemTheme != m->radioButtons.end())
	{
		IconRadioButton* rb = *itSystemTheme;
		const QString standardTheme(Gui::Icons::systemTheme());

		rb->setText(tr("System theme") + " (" + standardTheme + ")");
	}
}

void GUI_IconPreferences::showEvent(QShowEvent* e)
{
	initUi();
	Gui::Widget::showEvent(e);
}

IconRadioButton::IconRadioButton(const QString& text, QWidget* parent) : QRadioButton(text, parent) {}

IconRadioButton::~IconRadioButton() = default;

void IconRadioButton::setTheme(const QString& data)
{
	mTheme = data;
}

QString IconRadioButton::theme() const
{
	return mTheme;
}
