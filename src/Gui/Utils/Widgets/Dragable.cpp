/* Dragable.cpp */

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

#include "Dragable.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Algorithm.h"

#include <QPoint>
#include <QMimeData>
#include <QPixmap>
#include <QDrag>
#include <QPainter>
#include <QApplication>
#include <QWidget>
#include <QUrl>
#include <QFontMetrics>

namespace Algorithm=Util::Algorithm;
namespace UtilFile=Util::File;

using Gui::Dragable;

struct Dragable::Private
{
	QPoint		start_drag_pos;
	QWidget*	parent=nullptr;
	QDrag*		drag=nullptr;
	bool		valid;

	bool		dragging=false;

	Private(QWidget* parent) :
		parent(parent),
		valid(false),
		dragging(false)
	{}

	QStringList get_strings(const QMimeData* data)
	{
		QStringList ret;
		int playlists, dirs, tracks;
		playlists = dirs = tracks = 0;

		QList<QUrl> urls = data->urls();

		for(const QUrl& url : urls)
		{
			QString filename = url.toLocalFile();
			if(UtilFile::is_playlistfile(filename)){
				playlists++;
			}

			else if(UtilFile::is_soundfile(filename)){
				tracks++;
			}

			else if(UtilFile::is_dir(filename)){
				dirs++;
			}
		}

		if(tracks > 0){
			ret << QString::number(tracks) + " " + Lang::get(Lang::Tracks).toLower();
		}

		if(playlists > 0){
			ret << QString::number(playlists) + " " + Lang::get(Lang::Playlists).toLower();
		}

		if(dirs == 1){
			ret << QString::number(dirs) + " " + Lang::get(Lang::Directory).toLower();
		}

		else if(dirs > 0){
			ret << QString::number(dirs) + " " + Lang::get(Lang::Directories).toLower();
		}

		return ret;
	}
};


Dragable::Dragable(QWidget* parent)
{
	m = Pimpl::make<Dragable::Private>(parent);
}

Dragable::~Dragable() {}

void Dragable::drag_pressed(const QPoint& p)
{
	m->valid = is_valid_drag_position(p);
	m->dragging = false;
	m->start_drag_pos = p;
}


QDrag* Dragable::drag_moving(const QPoint& p)
{
	if(!m->valid){
		return nullptr;
	}

	int distance = (p - m->start_drag_pos).manhattanLength();

	if( distance < QApplication::startDragDistance())
	{
		return m->drag;
	}

	if(m->dragging)
	{
		return m->drag;
	}

	if(m->drag){
		delete m->drag;
	}

	m->dragging = true;
	m->start_drag_pos = QPoint();
	m->drag = new QDrag(m->parent);

	QMimeData* data = dragable_mimedata();
	if(data == nullptr)
	{
		return m->drag;
	}

	QStringList strings;
	if(!has_drag_label()){
		strings = m->get_strings(data);
	}

	else{
		strings.clear();
		strings << drag_label();
	}

	QFontMetrics fm(QApplication::font());
	const int logo_height = 24;
	const int logo_width = logo_height;
	const QSize logo_size(logo_width, logo_height);
	const int left_offset = 4;
	const int font_height = fm.ascent();
	const int text_height = strings.size() * (font_height + 2);
	const int pm_height = std::max(30, 30 + (strings.size() - 1) * font_height + 4);
	const int font_padding = (pm_height - text_height) / 2 + 1;

	int pm_width = logo_height + 4;

	for(const QString& str : Algorithm::AsConst(strings))
	{
		pm_width = std::max( pm_width, fm.width(str) );
	}

	pm_width += logo_width + 22;


	QPixmap cover = drag_pixmap();
	if(cover.isNull()){
		cover = Gui::Util::pixmap("logo.png", logo_size, true);
	}

	QPixmap pm(pm_width, pm_height);
	QPainter painter(&pm);

	painter.fillRect(pm.rect(), QColor(64, 64, 64));
	painter.setPen(QColor(243,132,26));
	painter.drawRect(0, 0, pm_width - 1, pm_height - 1);
	painter.drawPixmap(left_offset, (pm_height - logo_height) / 2, logo_height, logo_height, cover);
	painter.setPen(QColor(255, 255, 255));
	painter.translate(logo_width + 15, font_padding + font_height - 2);

	for(const QString& str : Algorithm::AsConst(strings))
	{
		painter.drawText(0, 0, str);
		painter.translate(0, font_height + 2);
	}

	m->drag->setMimeData(data);
	m->drag->setPixmap(pm);
	m->drag->exec(Qt::CopyAction);

	return m->drag;
}


void Dragable::drag_released(Dragable::ReleaseReason reason)
{
	if(!m){
		return;
	}

	if(reason == Dragable::ReleaseReason::Destroyed)
	{
		m->drag = nullptr;
	}

	else if(m->drag){
		delete m->drag; m->drag = nullptr;
	}

	m->dragging = false;
	m->start_drag_pos = QPoint();
}

bool Dragable::is_valid_drag_position(const QPoint &p) const
{
	Q_UNUSED(p)
	return true;
}


QPixmap Dragable::drag_pixmap() const
{
	return QPixmap();
}

bool Dragable::has_drag_label() const
{
	return false;
}

QString Dragable::drag_label() const
{
	return QString();
}
