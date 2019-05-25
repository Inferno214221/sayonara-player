/* GuiUtils.cpp */

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


/* GuiUtils.cpp */

#include "GuiUtils.h"
#include "Utils/Logger/Logger.h"

#include <QFontMetrics>
#include <QIcon>
#include <QString>
#include <QPixmap>
#include <QMainWindow>

using namespace Gui;

static QString icon_path(const QString& icon_name)
{
	QString path = QString(":/Icons/") + icon_name;
	if(path.endsWith(".png")){
		// alles paletti
	}

	else if(path.endsWith(".svg")){
		// svg
	}

	else if(!path.endsWith(".svg.png")){
		path += ".svg.png";
	}

	return path;
}

QIcon Util::icon(const QString& icon_name)
{
	QString path = icon_path(icon_name);
	QIcon icon = QIcon(path);

	if(icon.isNull()){
		sp_log(Log::Warning, "GuiUtils") << "Icon " << path << " does not exist";
	}

	return icon;
}

QImage Util::image(const QString& icon_name, QSize sz, bool keep_aspect)
{
	QString path = icon_path(icon_name);
	QImage image(path);

	if(image.isNull()){
		sp_log(Log::Warning, "GuiUtils") << "Pixmap " << path << " does not exist";
	}

	if(sz.width() == 0){
		return image;
	}

	else{
		if(keep_aspect){
			return image.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		else{
			return image.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
	}

	return image;
}

QPixmap Util::pixmap(const QString& icon_name, QSize sz, bool keep_aspect)
{
	QString path = icon_path(icon_name);
	QPixmap pixmap(path);

	if(pixmap.isNull()){
		sp_log(Log::Warning, "GuiUtils") << "Pixmap " << path << " does not exist";
	}

	if(sz.width() == 0){
		return pixmap;
	}

	else{
		if(keep_aspect){
			return pixmap.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		else{
			return pixmap.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
	}

	return pixmap;
}


static QMainWindow* main_window=nullptr;

QMainWindow* Util::main_window()
{
	return ::main_window;
}

void Util::set_main_window(QMainWindow* window)
{
	::main_window = window;
}

QString Util::elide_text(const QString &text, QWidget *widget, int max_lines)
{
	QFontMetrics metric = widget->fontMetrics();
	int width = widget->width();

	QStringList splitted = text.split(" ");
	QStringList ret;
	QString line;

	for( const QString& str : splitted)
	{
		QString tmp = line + str;

		if(metric.boundingRect(tmp).width() > width){
			ret << line;

			if(ret.size() == max_lines){
				line = "";
				break;
			}

			line = str + " ";
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
