/* SayonaraWidget.h */

/* Copyright (C) 2011-2016  Lucio Carreras
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



#ifndef SAYONARAWIDGET_H
#define SAYONARAWIDGET_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QComboBox>
#include <QPalette>
#include <QColor>
#include <QShortcut>
#include <QCloseEvent>


#include "Helper/SayonaraClass.h"


#define combo_current_index_changed_int	static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged)
#define combo_activated_int	static_cast<void (QComboBox::*) (int)>(&QComboBox::activated)
#define spinbox_value_changed_int	static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged)

class QCloseEvent;
class SayonaraShortcutWidget {

public:
	virtual QString get_shortcut_text(const QString& shortcut_identifier) const
	{
		Q_UNUSED(shortcut_identifier)
		return "";
	}
};

template<typename T>
class SayonaraWidgetTemplate :
		public T,
		protected SayonaraClass
{

public:
	SayonaraWidgetTemplate(QWidget* parent=nullptr) :
		T(parent),
		SayonaraClass()
	{

	}

	virtual ~SayonaraWidgetTemplate(){

	}

	bool is_dark() const
	{
		bool dark = (_settings->get(Set::Player_Style) == 1);

		QPalette palette = this->palette();
		QColor color = palette.color(QPalette::Normal, QPalette::Background);

		if(color.lightness() < 128 || dark){
			return true;
		}

		else{
			return false;
		}
	}



protected:

	QString elide_text(const QString &text, QWidget *widget, int max_lines){

		QFontMetrics metric = widget->fontMetrics();
		int width = widget->width();

		QStringList splitted = text.split(" ");
		QStringList ret;
		QString tmp;
		QString line;

		for( const QString& str : splitted){

			tmp = line + str;

			if(metric.boundingRect(tmp).width() > width){
				ret << line;

				if(ret.size() == max_lines){
					line = "";
					break;
				}

				line = str;
			}

			else{
				line += str + " ";
			}
		}


		QString final_str;
		if(ret.isEmpty()){
			final_str = text;
		}

		else if(line.isEmpty()){
			final_str = ret.join("\n");
			final_str += "...";
		}

		else {
			final_str = ret.join("\n") + line;
		}


		return final_str;
	}
};


class SayonaraDialog;
class SayonaraWidget : public SayonaraWidgetTemplate<QWidget> {

	Q_OBJECT

public:
	SayonaraWidget(QWidget* parent=nullptr);
	virtual ~SayonaraWidget();

	SayonaraDialog* box_into_dialog();


protected:
	SayonaraDialog* _boxed_dialog=nullptr;

protected slots:
	virtual void language_changed();
	virtual void skin_changed();
};


class SayonaraDialog : public SayonaraWidgetTemplate<QDialog> {

	Q_OBJECT

signals:
	void sig_closed();

public:
	SayonaraDialog(QWidget* parent=nullptr);
	virtual ~SayonaraDialog();

protected:
	virtual void closeEvent(QCloseEvent* e) override;

protected slots:
	virtual void language_changed();
	virtual void skin_changed();

};


class SayonaraMainWindow : public SayonaraWidgetTemplate<QMainWindow> {

	Q_OBJECT

public:
	SayonaraMainWindow(QWidget* parent=nullptr);
	virtual ~SayonaraMainWindow();


protected slots:
	virtual void language_changed();
	virtual void skin_changed();
};


#endif // SAYONARAWIDGET_H
