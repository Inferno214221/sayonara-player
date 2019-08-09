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
#include <QList>
#include <QSize>

using namespace Gui;

#include <QDir>
#include <QDirIterator>

static QString best_resolution_path(const QStringList& paths)
{
	if(paths.isEmpty()){
		return QString();
	}

	int max_size = 0;
	QString max_path = paths.first();
	for(const QString& path : paths)
	{
		QRegExp re(".*([0-9][0-9]+).*");
		int idx = re.indexIn(path);
		if(idx >= 0)
		{
			int size = re.cap(1).toInt();
			if(size > max_size)
			{
				max_size = size;
				max_path = path;
			}
		}
	}

	return max_path;
}

static QStringList icon_paths(const QString& icon_name, Gui::Util::IconTheme theme)
{
	if(icon_name.trimmed().isEmpty()){
		return QStringList();
	}

	QString theme_name;
	if(theme == Gui::Util::MintY)
	{
		theme_name = "MintYIcons";
	}


	QStringList paths;
	if(theme == Gui::Util::MintY)
	{
		QString re_string = QString(".*%1/%2-[0-9]+.*")
			.arg(theme_name)
			.arg(icon_name);

		QRegExp re(re_string);

		QDirIterator it(":/" + theme_name);
		while(it.hasNext())
		{
			QString path = it.next();

			if(re.indexIn(path) >= 0){
				paths << path;
			}
		}
	}

	if(paths.isEmpty())
	{
		QString re_string = QString(".*/%1(\\.[a-z][a-z][a-z])*$").arg(icon_name);
		QRegExp re(re_string);

		QDirIterator it(":/Icons");
		while(it.hasNext())
		{
			QString path = it.next();

			if(re.indexIn(path) >= 0){
				paths << path;
			}
		}
	}

	paths.sort();
	return paths;
}

#include <QRegExp>
QIcon Util::icon(const QString& icon_name, IconTheme theme_name)
{
	if(icon_name.isEmpty()){
		return QIcon();
	}

	QStringList paths = icon_paths(icon_name, theme_name);
	if(paths.isEmpty()){
		return QIcon();
	}

	QIcon icon;
	QString path = best_resolution_path(paths);
	if(!path.isEmpty())
	{
		QPixmap pm(path);
		if(!pm.isNull())
		{
			icon.addPixmap(pm);
		}

		else {
			icon.addFile(path);
		}
	}

	if(icon.isNull()){
		sp_log(Log::Warning, "GuiUtils") << "Icon " << icon_name << " does not exist";
	}

	return icon;
}

QImage Util::image(const QString& icon_name, IconTheme theme_name)
{
	return image(icon_name, theme_name, QSize(0, 0));
}

QImage Util::image(const QString& icon_name, IconTheme theme_name, QSize sz, bool keep_aspect)
{
	QStringList paths = icon_paths(icon_name, theme_name);
	if(paths.isEmpty()){
		return QImage();
	}

	QString path = best_resolution_path(paths);

	QImage image(path);
	if(image.isNull()){
		sp_log(Log::Warning, "GuiUtils") << "Pixmap " << paths.first() << " does not exist";
	}

	if(sz.width() == 0){
		return image;
	}

	else
	{
		if(keep_aspect)
		{
			return image.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		else{
			return image.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
	}
}

QPixmap Util::pixmap(const QString& icon_name, IconTheme theme_name)
{
	return pixmap(icon_name, theme_name, QSize(0, 0));
}

QPixmap Util::pixmap(const QString& icon_name, IconTheme theme_name, QSize sz, bool keep_aspect)
{
	QStringList paths = icon_paths(icon_name, theme_name);
	if(paths.isEmpty()){
		return QPixmap();
	}

	QString path = best_resolution_path(paths);

	QPixmap pixmap(path);
	if(pixmap.isNull()){
		sp_log(Log::Warning, "GuiUtils") << "Pixmap " << paths.first() << " does not exist";
	}

	if(sz.width() == 0){
		return pixmap;
	}

	else
	{
		if(keep_aspect)
		{
			return pixmap.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		else
		{
			return pixmap.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
	}
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
