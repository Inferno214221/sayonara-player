/* GuiUtils.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include <QApplication>
#include <QDesktopWidget>
#include <QFontMetrics>
#include <QIcon>
#include <QString>
#include <QPixmap>
#include <QMainWindow>
#include <QList>
#include <QSize>
#include <QScreen>
#include <QRegExp>
#include <QDir>
#include <QDirIterator>

using namespace Gui;

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

QScreen* Util::get_biggest_screen()
{
	QList<QScreen*> screens = QApplication::screens();
	auto it = std::max_element(screens.begin(), screens.end(), [](QScreen* s1, QScreen* s2)
	{
		return (s1->size().height() < s2->size().height());
	});

	if(it != screens.end()) {
		return *it;
	}

	if(screens.size() > 0) {
		return screens[0];
	}

	return nullptr;
}

void Util::place_in_screen_center(QWidget* widget, float rel_size_x, float rel_size_y)
{
	QScreen* screen = get_biggest_screen();
	if(!screen){
		return;
	}

	int w = screen->size().width();
	int h = screen->size().height();

	if(rel_size_x < 0.1f || rel_size_y < 0.1f)
	{
		rel_size_x = 0.7f;
		rel_size_y = 0.7f;
	}

	float x_remainder = (1.0f - rel_size_x) / 2.0f;
	float y_remainder = (1.0f - rel_size_y) / 2.0f;

	int x_abs = std::max(0, int(w * x_remainder)) + screen->geometry().x();
	int y_abs = std::max(0, int(h * y_remainder)) + screen->geometry().y();
	int w_abs = std::max(0, int(w * rel_size_x));
	int h_abs = std::max(0, int(h * rel_size_y));

	if(w_abs == 0){
		w_abs = 1200;
	}

	if(h_abs == 0){
		h_abs = 800;
	}

	widget->setGeometry(x_abs, y_abs, w_abs, h_abs);
}

int Util::text_width(const QFontMetrics& fm, const QString& text)
{
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 11
	return fm.horizontalAdvance(text);
#else
	return fm.width(text);
#endif
}
