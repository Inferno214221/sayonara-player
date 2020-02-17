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
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>

namespace Algorithm=Util::Algorithm;

class IconRadioButton : public QRadioButton
{
private:
	QString mData;

public:
	IconRadioButton(const QString& text, QWidget* parent);
	~IconRadioButton();

	void set_data(const QString& data);
	QString data() const;
};


static
IconRadioButton* addRadioButton(const QString& text, const QString& themeName, QWidget* widget, bool ignoreErrors)
{
	if(!ignoreErrors)
	{
		QIcon::setThemeName(themeName);
		QIcon icon = QIcon::fromTheme("media-playback-start");
		if(icon.isNull()){
			return nullptr;
		}
	}

	IconRadioButton* rb = new IconRadioButton(text, widget);
	rb->set_data(themeName);

	return rb;
}

struct GUI_IconPreferences::Private
{
	QHash<QString, IconRadioButton*> radioButtonMap;
	QString originalTheme;

	Private()
	{
		originalTheme = QIcon::themeName();
	}
};

GUI_IconPreferences::GUI_IconPreferences(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_IconPreferences::~GUI_IconPreferences() = default;

void GUI_IconPreferences::languageChanged()
{
	QString standardTheme(Gui::Icons::standardTheme());
	IconRadioButton* rb = m->radioButtonMap[standardTheme];
	if(rb){
		rb->setText(
			tr("System theme") + " (" + standardTheme + ")"
		);
	}
}


QString GUI_IconPreferences::actionName() const
{
	return tr("Icons");
}

bool GUI_IconPreferences::commit()
{
	if(!ui) {
		return true;
	}

	for(auto it=m->radioButtonMap.begin(); it != m->radioButtonMap.end(); it++)
	{
		const QString& key = it.key();
		IconRadioButton* rb = it.value();

		rb->setStyleSheet("font-weight: normal;");
		if(rb->isChecked())
		{
			SetSetting(Set::Icon_Theme, key);
			rb->setStyleSheet("font-weight: bold;");
			m->originalTheme = rb->data();

			Gui::Icons::changeTheme();
		}
	}

	bool force_std_icons = ui->cb_alsoUseInDarkStyle->isChecked();
	Gui::Icons::forceStandardIcons(force_std_icons);
	SetSetting(Set::Icon_ForceInDarkTheme, force_std_icons);

	Set::shout<SetNoDB::Player_MetaStyle>();

	return true;
}

void GUI_IconPreferences::revert()
{
	if(!ui) {
		return;
	}

	for(auto it=m->radioButtonMap.cbegin(); it != m->radioButtonMap.cend(); it++)
	{
		const QString& key = it.key();
		IconRadioButton* rb = it.value();

		rb->setChecked(key.compare(m->originalTheme) == 0);
	}

	bool b = GetSetting(Set::Icon_ForceInDarkTheme);
	ui->cb_alsoUseInDarkStyle->setChecked(b);

	QIcon::setThemeName(m->originalTheme);
}

static void applyIcon(const QString& n, const QString& theme_name, QLabel* label)
{
	QIcon::setThemeName(theme_name);
	QIcon icon = QIcon::fromTheme(n);
	label->setPixmap(icon.pixmap(32, 32).scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void GUI_IconPreferences::themeChanged(const QString& theme)
{
	applyIcon("media-playback-start", theme, ui->lab_play);
	applyIcon("media-skip-backward", theme, ui->lab_bwd);
	applyIcon("media-skip-forward", theme, ui->lab_fwd);
	applyIcon("media-playback-stop", theme, ui->lab_stop);
}

void GUI_IconPreferences::radioButtonToggled(bool b)
{
	auto rb = static_cast<IconRadioButton*>(sender());
	if(b && rb)
	{
		this->themeChanged(rb->data());
	}
}

void GUI_IconPreferences::initUi()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_IconPreferences();
	ui->setupUi(this);

	const QString systemTheme(Gui::Icons::standardTheme());

	QWidget* widget = ui->scroll_areaWidget;
	widget->setObjectName("IconThemeScrollWidget");

	QLayout* layout = widget->layout();

	IconRadioButton* rbSystemTheme = addRadioButton
	(
		tr("System theme") + " (" + systemTheme + ")",
		systemTheme,
		 widget,
		true
	);

	rbSystemTheme->setStyleSheet("font-weight: bold;");
	connect(rbSystemTheme, &QRadioButton::toggled, this, &GUI_IconPreferences::radioButtonToggled);
	layout->addWidget(rbSystemTheme);
	m->radioButtonMap[systemTheme] = rbSystemTheme;

	QStringList iconPaths = QIcon::themeSearchPaths();
	Algorithm::sort(iconPaths, [](const QString& s1, const QString& s2){
		return (s1.size() < s2.size() || s1 < s2);
	});
	QIcon::setThemeSearchPaths(iconPaths);

	for(const QString& icon_path : Algorithm::AsConst(iconPaths))
	{
		QDir d(icon_path);
		QStringList subdirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		if(subdirs.isEmpty()){
			continue;
		}

		QList<IconRadioButton*> buttons;
		for(const QString& subdir : subdirs)
		{
			if(subdir.isEmpty() || (subdir == systemTheme)) {
				continue;
			}

			IconRadioButton* rb = addRadioButton(subdir, subdir, widget, false);
			if(!rb){
				continue;
			}

			connect(rb, &QRadioButton::toggled, this, &GUI_IconPreferences::radioButtonToggled);

			m->radioButtonMap[subdir] = rb;

			if(m->originalTheme.compare(QIcon::themeName()) == 0){
				rb->setStyleSheet("font-weight: bold;");
			}

			buttons << rb;
		}

		for(IconRadioButton* rb : Algorithm::AsConst(buttons))
		{
			layout->addWidget(rb);
		}
	}

	revert();
}

void GUI_IconPreferences::showEvent(QShowEvent* e)
{
	if(!ui)
	{
		initUi();
	}

	Gui::Widget::showEvent(e);
}

IconRadioButton::IconRadioButton(const QString& text, QWidget* parent) : QRadioButton(text, parent) {}

IconRadioButton::~IconRadioButton() = default;

void IconRadioButton::set_data(const QString& data)
{
	mData = data;
}

QString IconRadioButton::data() const
{
	return mData;
}
