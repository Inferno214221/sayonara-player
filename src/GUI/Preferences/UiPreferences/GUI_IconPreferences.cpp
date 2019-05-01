/* GUI_IconPreferences.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "GUI/Preferences/ui_GUI_IconPreferences.h"
#include "GUI/Utils/Icons.h"
#include "Components/Directories/DirectoryReader.h"

#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QStringList>
#include <QIcon>
#include <QDir>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>

class IconRadioButton : public QRadioButton
{
private:
	QString _data;

public:
	using QRadioButton::QRadioButton;

	void set_data(const QString& data)
	{
		_data = data;
	}

	QString data() const
	{
		return _data;
	}
};


static
void add_line(QWidget* widget)
{
	QLayout* layout = widget->layout();
	QFrame* line = new QFrame(widget);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	layout->addWidget(line);
}


static
void add_header_entry(const QString& name, QWidget* widget)
{
	QLayout* layout = widget->layout();

	add_line(widget);

	QLabel* label = new QLabel(widget);
	label->setText(name);
	label->setStyleSheet("font-weight: bold;");
	layout->addWidget(label);

	add_line(widget);
}


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

GUI_IconPreferences::~GUI_IconPreferences() {}

void GUI_IconPreferences::language_changed()
{
	QString standard_theme(Gui::Icons::standard_theme());
	IconRadioButton* rb = m->rb_map[standard_theme];
	if(rb){
		rb->setText(
			Lang::get(Lang::Default) + " (" + standard_theme + ")"
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

	for(auto it=m->rb_map.cbegin(); it != m->rb_map.cend(); it++)
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

	bool force_std_icons = ui->cb_force_in_dark_theme->isChecked();
	Gui::Icons::force_standard_icons(force_std_icons);
	SetSetting(Set::Icon_ForceInDarkTheme, force_std_icons);

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

	ui->cb_force_in_dark_theme->setChecked(GetSetting(Set::Icon_ForceInDarkTheme));

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

void GUI_IconPreferences::init_ui()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_IconPreferences();
	ui->setupUi(this);

	QString standard_theme(Gui::Icons::standard_theme());

	QWidget* widget = ui->scroll_area_widget;
	QLayout* layout = widget->layout();
	add_line(widget);

	IconRadioButton* rb_automatic = add_radio_button(
					Lang::get(Lang::Default) + " (" + standard_theme + ")",
					standard_theme,
					 widget,
					true
	);

	layout->addWidget(rb_automatic);
	rb_automatic->setStyleSheet("font-weight: bold;");

	m->rb_map[standard_theme] = rb_automatic;

	QStringList icon_paths = QIcon::themeSearchPaths();
	Util::sort(icon_paths, [](const QString& s1, const QString& s2){
		return (s1.size() < s2.size() || s1 < s2);
	});
	QIcon::setThemeSearchPaths(icon_paths);

	for(const QString& icon_path : Util::AsConst(icon_paths))
	{
		QDir d(icon_path);
		QStringList subdirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		if(subdirs.isEmpty()){
			continue;
		}

		QList<IconRadioButton*> buttons;
		for(const QString& subdir : subdirs)
		{
			if(subdir.isEmpty()){
				continue;
			}

			IconRadioButton* rb = add_radio_button(subdir, subdir, widget, false);
			if(!rb){
				continue;
			}

			connect(rb, &QRadioButton::toggled, this, [=](bool checked)
			{
				if(checked){
					this->theme_changed(rb->data());
				}
			});

			m->rb_map[subdir] = rb;

			if(m->original_theme.compare(QIcon::themeName()) == 0){
				rb->setStyleSheet("font-weight: bold;");
			}

			buttons << rb;
		}

		if(!buttons.isEmpty()){
			add_header_entry(icon_path, widget);
		}

		for(IconRadioButton* rb : Util::AsConst(buttons))
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
