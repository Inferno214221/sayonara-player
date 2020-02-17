/* GuiUtils.cpp */

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

static QStringList iconPaths(const QString& iconName, Gui::Util::IconTheme theme)
{
	if(iconName.trimmed().isEmpty()){
		return QStringList();
	}

	QString themeName;
	if(theme == Gui::Util::MintY)
	{
		themeName = "MintYIcons";
	}


	QStringList paths;
	if(theme == Gui::Util::MintY)
	{
		QString reString = QString(".*%1/%2-[0-9]+.*")
			.arg(themeName)
			.arg(iconName);

		QRegExp re(reString);

		QDirIterator it(":/" + themeName);
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
		QString reString = QString(".*/%1(\\.[a-z][a-z][a-z])*$").arg(iconName);
		QRegExp re(reString);

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


QIcon Util::icon(const QString& iconName, IconTheme themeName)
{
	if(iconName.isEmpty()){
		return QIcon();
	}

	QStringList paths = iconPaths(iconName, themeName);
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
		spLog(Log::Warning, "GuiUtils") << "Icon " << iconName << " does not exist";
	}

	return icon;
}

QImage Util::image(const QString& iconName, IconTheme themeName)
{
	return image(iconName, themeName, QSize(0, 0));
}

QImage Util::image(const QString& iconName, IconTheme themeName, QSize sz, bool keep_aspect)
{
	QStringList paths = iconPaths(iconName, themeName);
	if(paths.isEmpty()){
		return QImage();
	}

	QString path = best_resolution_path(paths);

	QImage image(path);
	if(image.isNull()){
		spLog(Log::Warning, "GuiUtils") << "Pixmap " << paths.first() << " does not exist";
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

QPixmap Util::pixmap(const QString& iconName, IconTheme themeName)
{
	return pixmap(iconName, themeName, QSize(0, 0));
}

QPixmap Util::pixmap(const QString& iconName, IconTheme themeName, QSize sz, bool keep_aspect)
{
	QStringList paths = iconPaths(iconName, themeName);
	if(paths.isEmpty()){
		return QPixmap();
	}

	QString path = best_resolution_path(paths);

	QPixmap pixmap(path);
	if(pixmap.isNull()){
		spLog(Log::Warning, "GuiUtils") << "Pixmap " << paths.first() << " does not exist";
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

QMainWindow* Util::mainWindow()
{
	return ::main_window;
}

void Util::setMainWindow(QMainWindow* window)
{
	::main_window = window;
}

QScreen* Util::getBiggestScreen()
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

void Util::placeInScreenCenter(QWidget* widget, float relativeSizeX, float relativeSizeY)
{
	QScreen* screen = getBiggestScreen();
	if(!screen){
		return;
	}

	int w = screen->size().width();
	int h = screen->size().height();

	if(relativeSizeX < 0.1f || relativeSizeY < 0.1f)
	{
		relativeSizeX = 0.7f;
		relativeSizeY = 0.7f;
	}

	float xRemainder = (1.0f - relativeSizeX) / 2.0f;
	float yRemainder = (1.0f - relativeSizeY) / 2.0f;

	int xAbs = std::max(0, int(w * xRemainder)) + screen->geometry().x();
	int yAbs = std::max(0, int(h * yRemainder)) + screen->geometry().y();
	int wAbs = std::max(0, int(w * relativeSizeX));
	int hAbs = std::max(0, int(h * relativeSizeY));

	if(wAbs == 0){
		wAbs = 1200;
	}

	if(hAbs == 0){
		hAbs = 800;
	}

	widget->setGeometry(xAbs, yAbs, wAbs, hAbs);
}

int Util::textWidget(const QFontMetrics& fm, const QString& text)
{
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 11
	return fm.horizontalAdvance(text);
#else
	return fm.width(text);
#endif
}
