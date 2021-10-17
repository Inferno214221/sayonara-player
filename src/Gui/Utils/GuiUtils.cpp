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

#include <QApplication>
#include <QDesktopWidget>
#include <QDirIterator>
#include <QFontMetrics>
#include <QIcon>
#include <QList>
#include <QPalette>
#include <QPixmap>
#include <QRegExp>
#include <QScreen>
#include <QSize>
#include <QString>
#include <QCache>

#include <algorithm>

namespace
{
	using IconCache = QCache<QString, QIcon>;
	Q_GLOBAL_STATIC_WITH_ARGS(IconCache, iconCache, (50))

	QString getCacheKey(const QString& iconName, Gui::Util::IconTheme theme)
	{
		return QString("%1-%2")
			.arg(iconName)
			.arg(static_cast<int>(theme));
	}

	QString getCacheKey(const QString& iconName, const QString& systemThemeName)
	{
		return QString("%1-%2")
			.arg(iconName)
			.arg(systemThemeName);
	}

	bool insertIconIntoCache(const QString& key, QIcon& icon)
	{
		if(!key.isNull() && !icon.isNull() && !iconCache.isDestroyed())
		{
			auto* iconPtr = new QIcon(std::move(icon));
			iconCache->insert(key, iconPtr);
			return true;
		}

		return false;
	}

	QIcon fetchIconFromCache(const QString& cacheKey)
	{
		return (!iconCache.isDestroyed() && iconCache->object(cacheKey))
		       ? *iconCache->object(cacheKey)
		       : QIcon();
	}

	QStringList searchInResource(const QString& resourceName, const QString& regex)
	{
		const auto re = QRegExp(regex);
		const auto themePath = QString(":/%1").arg(resourceName);

		QStringList paths;
		auto dirIterator = QDirIterator(themePath);
		while(dirIterator.hasNext())
		{
			const auto path = dirIterator.next();
			if(re.indexIn(path) >= 0)
			{
				paths << path;
			}
		}

		return paths;
	}

	QStringList searchInMintYIcons(const QString& iconName)
	{
		const auto themeName = QStringLiteral("MintYIcons");
		const auto reString = QString(".*%1/%2-[0-9]+.*")
			.arg(themeName)
			.arg(iconName);

		return searchInResource(themeName, reString);
	}

	QStringList searchInStandardIcons(const QString& iconName)
	{
		QStringList paths;

		const auto reString = QString(".*/%1(\\.[a-z][a-z][a-z])*$").arg(iconName);
		return searchInResource("Icons", reString);
	}

	QStringList iconPaths(const QString& iconName, Gui::Util::IconTheme iconTheme)
	{
		if(iconName.trimmed().isEmpty())
		{
			return QStringList();
		}

		QStringList paths;
		if(iconTheme == Gui::Util::MintY)
		{
			paths = searchInMintYIcons(iconName);
		}

		if(paths.isEmpty())
		{
			paths = searchInStandardIcons(iconName);
		}

		return paths;
	}
}

namespace Gui
{
	QIcon Util::icon(const QString& iconName, IconTheme iconTheme)
	{
		if(iconName.isEmpty())
		{
			return QIcon();
		}

		const auto cacheKey = getCacheKey(iconName, iconTheme);
		auto icon = fetchIconFromCache(cacheKey);
		if(!icon.isNull())
		{
			return icon;
		}

		const auto paths = iconPaths(iconName, iconTheme);
		for(const auto& path : paths)
		{
			const auto pixmap = QPixmap(path);
			if(!pixmap.isNull())
			{
				icon.addPixmap(pixmap);
			}
		}

		const auto success = insertIconIntoCache(cacheKey, icon);
		return (success)
		       ? fetchIconFromCache(cacheKey)
		       : icon;
	}

	QIcon Util::systemThemeIcon(const QString& iconName)
	{
		const auto cacheKey = getCacheKey(iconName, QIcon::themeName());
		auto icon = fetchIconFromCache(cacheKey);
		if(!icon.isNull())
		{
			return icon;
		}

		icon = QIcon::fromTheme(iconName);
		const auto success = insertIconIntoCache(cacheKey, icon);
		return success
		       ? fetchIconFromCache(cacheKey)
		       : icon;
	}

	QImage Util::image(const QString& iconName, IconTheme themeName)
	{
		return image(iconName, themeName, QSize(32, 32));
	}

	QImage Util::image(const QString& iconName, IconTheme themeName, const QSize& size, bool keepAspectRatio)
	{
		const auto pixmap = Util::pixmap(iconName, themeName, size, keepAspectRatio);
		return (!pixmap.isNull())
		       ? pixmap.toImage()
		       : QImage();
	}

	QPixmap Util::pixmap(const QString& iconName, IconTheme themeName)
	{
		return pixmap(iconName, themeName, QSize(32, 32));
	}

	QPixmap Util::pixmap(const QString& iconName, IconTheme themeName, const QSize& size, bool keepAspectRatio)
	{
		const auto icon = Util::icon(iconName, themeName);
		if(!icon.isNull())
		{
			const auto pixmap = icon.pixmap(size);
			if(!pixmap.isNull())
			{
				const auto aspect = keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
				return pixmap.scaled(size, aspect, Qt::SmoothTransformation);
			}
		}

		return QPixmap();
	}

	QColor Util::color(QPalette::ColorGroup colorGroup, QPalette::ColorRole colorRole)
	{
		return QApplication::palette().color(colorGroup, colorRole);
	}

	QScreen* Util::getBiggestScreen()
	{
		const auto screens = QApplication::screens();
		const auto it = std::max_element(screens.cbegin(),
		                                 screens.cend(),
		                                 [](const auto* screen1, const auto* screen2) {
			                                 return (screen1->size().height() < screen2->size().height());
		                                 });

		return (it != screens.end())
		       ? *it
		       : nullptr;
	}

	void Util::placeInScreenCenter(QWidget* widget, float relativeSizeX, float relativeSizeY)
	{
		const auto* screen = getBiggestScreen();
		if(!screen)
		{
			return;
		}

		const auto width = screen->size().width();
		const auto height = screen->size().height();

		if(relativeSizeX < 0.1f || relativeSizeY < 0.1f)
		{
			relativeSizeX = 0.7f;
			relativeSizeY = 0.7f;
		}

		const auto xRemainder = (1.0f - relativeSizeX) / 2.0f;
		const auto yRemainder = (1.0f - relativeSizeY) / 2.0f;

		const auto xAbs = std::max(0, static_cast<int>(width * xRemainder)) + screen->geometry().x();
		const auto yAbs = std::max(0, static_cast<int>(height * yRemainder)) + screen->geometry().y();
		auto wAbs = std::max(0, static_cast<int>(width * relativeSizeX));
		auto hAbs = std::max(0, static_cast<int>(height * relativeSizeY));

		if(wAbs == 0)
		{
			wAbs = 1200;
		}

		if(hAbs == 0)
		{
			hAbs = 800;
		}

		widget->setGeometry(xAbs, yAbs, wAbs, hAbs);
	}

	int Util::textWidth(const QFontMetrics& fm, const QString& text)
	{
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 11
		return fm.horizontalAdvance(text);
#else
		return fm.width(text);
#endif
	}

	int Util::textWidth(QWidget* widget, const QString& text)
	{
		return textWidth(widget->fontMetrics(), text);
	}

	int Util::viewRowHeight()
	{
		return -1;
	}
}