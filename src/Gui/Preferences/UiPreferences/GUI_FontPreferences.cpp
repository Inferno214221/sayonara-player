
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

#include "GUI_FontPreferences.h"
#include "Gui/Preferences/ui_GUI_FontPreferences.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>

struct GUI_FontPreferences::Private
{
	QFontDatabase*	fontDatabase=nullptr;
	int				currentFontSize;
	int				currentFontWeight;

	Private() :
		currentFontSize(0),
		currentFontWeight(0)
	{}
};

GUI_FontPreferences::GUI_FontPreferences(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_FontPreferences::~GUI_FontPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_FontPreferences::initUi()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_FontPreferences();
	ui->setupUi(this);

	m->fontDatabase = new QFontDatabase();

	connect(ui->combo_fonts, &QFontComboBox::currentFontChanged, this, &GUI_FontPreferences::comboFontsChanged);
	connect(ui->btn_default, &QPushButton::clicked, this, &GUI_FontPreferences::defaultClicked);

	ui->combo_fonts->setEditable(false);
	ui->combo_fonts->setFontFilters(QFontComboBox::ScalableFonts);

	revert();
}

QString GUI_FontPreferences::actionName() const
{
	return Lang::get(Lang::Fonts);
}

void GUI_FontPreferences::comboFontsChanged(const QFont& font)
{
	m->currentFontSize = ui->combo_sizes->currentText().toInt();

	QStringList sizes = availableFontSizes(font);
	fillSizes(sizes);

	int fontSize = m->currentFontSize;
	if(fontSize <= 0){
		fontSize = QApplication::font().pointSize();
	}

	int currentFontSizeIndex = ui->combo_sizes->findText(QString::number(fontSize));

	if(currentFontSizeIndex >= 0){
		ui->combo_sizes->setCurrentIndex(currentFontSizeIndex);
	}

	ui->combo_libSize->setCurrentIndex(0);
	ui->combo_plSize->setCurrentIndex(0);
}


QStringList GUI_FontPreferences::availableFontSizes(const QString& font_name, const QString& style)
{
	QStringList ret;
	QList<int> fontSizes =  m->fontDatabase->pointSizes(font_name, style);

	for(int fontSize : fontSizes){
		ret << QString::number(fontSize);
	}

	return ret;
}

QStringList GUI_FontPreferences::availableFontSizes(const QFont& font)
{
	return availableFontSizes(font.family(), font.styleName());
}

void GUI_FontPreferences::fillSizes(const QStringList& sizes)
{
	ui->combo_sizes->clear();
	ui->combo_libSize->clear();
	ui->combo_plSize->clear();

	ui->combo_libSize->addItem(tr("Inherit"));
	ui->combo_plSize->addItem(tr("Inherit"));

	for(const QString& sz : sizes)
	{
		int isz = sz.toInt();
		if(isz < 7 || isz > 20){
			continue;
		}

		ui->combo_sizes->addItem(sz);
		ui->combo_libSize->addItem(sz);
		ui->combo_plSize->addItem(sz);
	}
}

bool GUI_FontPreferences::commit()
{
	if(!ui)
	{
		return true;
	}

	bool ok;
	int font_size;

	font_size = ui->combo_sizes->currentText().toInt(&ok);
	SetSetting(Set::Player_FontName, ui->combo_fonts->currentText());

	if(ok){
		SetSetting(Set::Player_FontSize, font_size);
	}

	font_size = ui->combo_libSize->currentText().toInt(&ok);
	if(!ok){
		font_size = -1;
	}

	SetSetting(Set::Lib_FontSize, font_size);
	font_size = ui->combo_plSize->currentText().toInt(&ok);
	if(!ok){
		font_size = -1;
	}

	SetSetting(Set::PL_FontSize, font_size);
	SetSetting(Set::Lib_FontBold, ui->cb_libBold->isChecked());

	Set::shout<SetNoDB::Player_MetaStyle>();

	m->currentFontSize = font_size;

	return true;
}

void GUI_FontPreferences::revert()
{
	if(!ui)
	{
		return;
	}

	QString cur_family = GetSetting(Set::Player_FontName);
	int cur_font_size = GetSetting(Set::Player_FontSize);
	int cur_pl_font_size = GetSetting(Set::PL_FontSize);
	int cur_lib_font_size = GetSetting(Set::Lib_FontSize);
	bool bold = GetSetting(Set::Lib_FontBold);

	int idx = ui->combo_fonts->findText(cur_family);
	if(idx >= 0){
		ui->combo_fonts->setCurrentIndex(idx);
	}

	fillSizes( availableFontSizes(ui->combo_fonts->currentFont()) );

	idx = ui->combo_sizes->findText(QString::number(cur_font_size));
	if(idx >= 0){
		ui->combo_sizes->setCurrentIndex(idx);
	}

	idx = ui->combo_libSize->findText(QString::number(cur_lib_font_size));
	if(idx >= 0){
		ui->combo_libSize->setCurrentIndex(idx);
	}
	else{
		ui->combo_libSize->setCurrentIndex(0);
	}

	idx = ui->combo_plSize->findText(QString::number(cur_pl_font_size));
	if(idx >= 0){
		ui->combo_plSize->setCurrentIndex(idx);
	}
	else{
		ui->combo_plSize->setCurrentIndex(0);
	}

	ui->cb_libBold->setChecked(bold);
}


void GUI_FontPreferences::defaultClicked()
{
	QFont font = QApplication::font();

	int cur_font_idx = ui->combo_fonts->findText(font.family());
	if(cur_font_idx >= 0){
		ui->combo_fonts->setCurrentIndex(cur_font_idx);
	}

	ui->combo_libSize->setCurrentIndex(0);
	ui->combo_plSize->setCurrentIndex(0);
	ui->cb_libBold->setChecked(true);

	int cur_font_size_idx = ui->combo_sizes->findText(QString::number(font.pointSize()));
	if(cur_font_size_idx >= 0){
		ui->combo_sizes->setCurrentIndex(cur_font_size_idx);
	}
}


void GUI_FontPreferences::languageChanged()
{
	if(!ui){
		return;
	}

	ui->retranslateUi(this);
	ui->lab_library->setText(Lang::get(Lang::Library));
	ui->lab_playlist->setText(Lang::get(Lang::Playlist));
	ui->btn_default->setText(Lang::get(Lang::Default));
}

void GUI_FontPreferences::skinChanged()
{
	if(!ui){
		return;
	}

	ui->btn_default->setIcon(Gui::Icons::icon(Gui::Icons::Undo));
}

void GUI_FontPreferences::showEvent(QShowEvent* e)
{
	if(!ui){
		initUi();
	}

	Gui::Widget::showEvent(e);
}
