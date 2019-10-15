/* GUI_IconPreferences.cpp */

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
IconRadioButton* add_radio_button(const QString& text, const QString& theme_name, QWidget* widget, bool ignore_errors)
{
	if(!ignore_errors)
	{
		QIcon::setThemeName(theme_name);
		QIcon icon = QIcon::fromTheme("media-playback-start");
		if(icon.isNull()){
			return nullptr;
		}
	}

	IconRadioButton* rb = new IconRadioButton(text, widget);
	rb->set_data(theme_name);

	return rb;
}

struct GUI_IconPreferences::Private
{
	QHash<QString, IconRadioButton*> rb_map;
	QString original_theme;

	Private()
	{
		original_theme = QIcon::themeName();
	}
};

GUI_IconPreferences::GUI_IconPreferences(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_IconPreferences::~GUI_IconPreferences() = default;

void GUI_IconPreferences::language_changed()
{
	QString standard_theme(Gui::Icons::standard_theme());
	IconRadioButton* rb = m->rb_map[standard_theme];
	if(rb){
		rb->setText(
			tr("System theme") + " (" + standard_theme + ")"
		);
	}
}


QString GUI_IconPreferences::action_name() const
{
	return tr("Icons");
}

bool GUI_IconPreferences::commit()
{
	if(!ui) {
		return true;
	}

	for(auto it=m->rb_map.begin(); it != m->rb_map.end(); it++)
	{
		const QString& key = it.key();
		IconRadioButton* rb = it.value();

		rb->setStyleSheet("font-weight: normal;");
		if(rb->isChecked())
		{
			SetSetting(Set::Icon_Theme, key);
			rb->setStyleSheet("font-weight: bold;");
			m->original_theme = rb->data();

			Gui::Icons::change_theme();
		}
	}

	bool force_std_icons = ui->cb_also_use_in_dark_style->isChecked();
	Gui::Icons::force_standard_icons(force_std_icons);
	SetSetting(Set::Icon_ForceInDarkTheme, force_std_icons);

	Set::shout<SetNoDB::Player_MetaStyle>();

	return true;
}

void GUI_IconPreferences::revert()
{
	if(!ui) {
		return;
	}

	for(auto it=m->rb_map.cbegin(); it != m->rb_map.cend(); it++)
	{
		const QString& key = it.key();
		IconRadioButton* rb = it.value();

		rb->setChecked(key.compare(m->original_theme) == 0);
	}

	bool b = GetSetting(Set::Icon_ForceInDarkTheme);
	ui->cb_also_use_in_dark_style->setChecked(b);

	QIcon::setThemeName(m->original_theme);
}

static void apply_icon(const QString& n, const QString& theme_name, QLabel* label)
{
	QIcon::setThemeName(theme_name);
	QIcon icon = QIcon::fromTheme(n);
	label->setPixmap(icon.pixmap(32, 32).scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void GUI_IconPreferences::theme_changed(const QString& theme)
{
	apply_icon("media-playback-start", theme, ui->lab_play);
	apply_icon("media-skip-backward", theme, ui->lab_bwd);
	apply_icon("media-skip-forward", theme, ui->lab_fwd);
	apply_icon("media-playback-stop", theme, ui->lab_stop);
}

void GUI_IconPreferences::radio_button_toggled(bool b)
{
	auto rb = static_cast<IconRadioButton*>(sender());
	if(b && rb)
	{
		this->theme_changed(rb->data());
	}
}

void GUI_IconPreferences::init_ui()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_IconPreferences();
	ui->setupUi(this);

	QString system_theme(Gui::Icons::standard_theme());

	QWidget* widget = ui->scroll_area_widget;
	widget->setObjectName("IconThemeScrollWidget");

	QLayout* layout = widget->layout();

	IconRadioButton* rb_system_theme = add_radio_button
	(
		tr("System theme") + " (" + system_theme + ")",
		system_theme,
		 widget,
		true
	);

	rb_system_theme->setStyleSheet("font-weight: bold;");
	connect(rb_system_theme, &QRadioButton::toggled, this, &GUI_IconPreferences::radio_button_toggled);
	layout->addWidget(rb_system_theme);
	m->rb_map[system_theme] = rb_system_theme;

	QStringList icon_paths = QIcon::themeSearchPaths();
	Algorithm::sort(icon_paths, [](const QString& s1, const QString& s2){
		return (s1.size() < s2.size() || s1 < s2);
	});
	QIcon::setThemeSearchPaths(icon_paths);

	for(const QString& icon_path : Algorithm::AsConst(icon_paths))
	{
		QDir d(icon_path);
		QStringList subdirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		if(subdirs.isEmpty()){
			continue;
		}

		QList<IconRadioButton*> buttons;
		for(const QString& subdir : subdirs)
		{
			if(subdir.isEmpty() || (subdir == system_theme)) {
				continue;
			}

			IconRadioButton* rb = add_radio_button(subdir, subdir, widget, false);
			if(!rb){
				continue;
			}

			connect(rb, &QRadioButton::toggled, this, &GUI_IconPreferences::radio_button_toggled);

			m->rb_map[subdir] = rb;

			if(m->original_theme.compare(QIcon::themeName()) == 0){
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
		init_ui();
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
