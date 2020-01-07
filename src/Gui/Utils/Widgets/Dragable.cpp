/* Dragable.cpp */

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

#include "Dragable.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/EventFilter.h"

#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Algorithm.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDrag>
#include <QFontMetrics>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QUrl>

namespace Algorithm=Util::Algorithm;
namespace FileUtils=Util::File;

using Gui::Dragable;

struct Dragable::Private
{
	QPoint		start_drag_pos;
	QWidget*	widget=nullptr;
	QDrag*		drag=nullptr;
	bool		valid;

	bool		dragging=false;

	Private(QWidget* widget) :
		widget(widget),
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
			if(FileUtils::is_playlistfile(filename)){
				playlists++;
			}

			else if(FileUtils::is_soundfile(filename)){
				tracks++;
			}

			else if(FileUtils::is_dir(filename)){
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


Dragable::Dragable(QAbstractItemView* widget)
{
	m = Pimpl::make<Dragable::Private>(widget);

	widget->setDragEnabled(true);
	new DragableConnector(widget, this);
}

Dragable::~Dragable() = default;

void Dragable::start_drag(const QPoint& p)
{
	m->valid = is_valid_drag_position(p);
	m->dragging = false;
	m->start_drag_pos = p;
}


QDrag* Dragable::move_drag(const QPoint& p)
{
	if(!m->valid){
		return nullptr;
	}

	int distance = (p - m->start_drag_pos).manhattanLength();

	if( distance < QApplication::startDragDistance())
	{
		return m->drag;
	}

	if(m->dragging) {
		return m->drag;
	}

	if(m->drag) {
		delete m->drag;
	}

	m->dragging = true;
	m->start_drag_pos = QPoint();
	m->drag = new QDrag(m->widget);

	QMimeData* data = dragable_mimedata();
	if(data == nullptr) {
		return m->drag;
	}

	QStringList strings;
	if(!has_drag_label()) {
		strings = m->get_strings(data);
	}

	else {
		strings.clear();
		strings << drag_label();
	}

	QFontMetrics fm = m->widget->fontMetrics();
	const int	logo_height = 24;
	const int	logo_width = logo_height;
	const QSize logo_size(logo_width, logo_height);
	const int	left_offset = 4;
	const int	font_height = fm.ascent();
	const int	text_height = strings.size() * (font_height + 2);
	const int	pm_height = std::max(30, 30 + (strings.size() - 1) * font_height + 4);
	const int	font_padding = (pm_height - text_height) / 2 + 1;

	int pm_width = logo_height + 4;

	for(const QString& str : Algorithm::AsConst(strings))
	{
		pm_width = std::max( pm_width, Gui::Util::text_width(fm, str) );
	}

	pm_width += logo_width + 22;


	QPixmap cover = drag_pixmap();
	if(cover.isNull()){
		cover = Gui::Util::pixmap("logo.png", Gui::Util::NoTheme, logo_size, true);
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


void Dragable::release_drag(Dragable::ReleaseReason reason)
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

using Gui::DragableConnector;

struct DragableConnector::Private
{
	QAbstractItemView* widget=nullptr;
	Dragable* dragable=nullptr;

	Private(QAbstractItemView* w, Dragable* d) :
		widget(w),
		dragable(d)
	{}
};


Gui::DragableConnector::DragableConnector(QAbstractItemView* widget, Gui::Dragable* dragable) :
	QObject(widget)
{
	m = Pimpl::make<Private>(widget, dragable);

	auto* mpef = new Gui::MousePressedFilter(widget->viewport());
	auto* mmef = new Gui::MouseMoveFilter(widget->viewport());

	connect(mpef, &Gui::MousePressedFilter::sig_mouse_pressed, this, &DragableConnector::mouse_pressed);
	connect(mmef, &Gui::MouseMoveFilter::sig_mouse_moved, this, &DragableConnector::mouse_moved);

	widget->viewport()->installEventFilter(mpef);
	widget->viewport()->installEventFilter(mmef);
}

Gui::DragableConnector::~DragableConnector() = default;

void Gui::DragableConnector::mouse_pressed(QMouseEvent* e)
{
	if(e->buttons() & Qt::LeftButton)
	{
		m->dragable->start_drag(e->pos());
	}
}

void DragableConnector::mouse_moved(QMouseEvent* e)
{
	QDrag* drag = m->dragable->move_drag(e->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, this, &DragableConnector::drag_destroyed);
	}
}

void DragableConnector::drag_destroyed()
{
	m->dragable->release_drag(Dragable::ReleaseReason::Destroyed);
}
