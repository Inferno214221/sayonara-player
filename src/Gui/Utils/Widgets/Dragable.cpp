/* Dragable.cpp */

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

#include "Dragable.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/EventFilter.h"

#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

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
	QPoint		startDraggingPosition;
	QWidget*	widget=nullptr;
	QDrag*		drag=nullptr;
	bool		valid;

	bool		dragging=false;

	Private(QWidget* widget) :
		widget(widget),
		valid(false),
		dragging(false)
	{}

	QStringList getStrings(const QMimeData* data)
	{
		QStringList ret;
		int playlists, dirs, tracks, other_files;
		playlists = dirs = tracks = other_files = 0;

		QList<QUrl> urls = data->urls();

		for(const QUrl& url : urls)
		{
			QString filename = url.toLocalFile();
			if(FileUtils::isPlaylistFile(filename)){
				playlists++;
			}

			else if(FileUtils::isSoundFile(filename)){
				tracks++;
			}

			else if(FileUtils::isDir(filename)){
				dirs++;
			}

			else if(FileUtils::isFile(filename)){
				other_files++;
			}
		}

		if(tracks > 0){
			ret << Lang::getWithNumber(Lang::NrTracks, tracks);
		}

		if(playlists > 0){
			ret << Lang::getWithNumber(Lang::NrPlaylists, playlists);
		}

		if(dirs > 0){
			ret << Lang::getWithNumber(Lang::NrDirectories, dirs);
		}

		if(other_files > 0){
			ret << Lang::getWithNumber(Lang::NrFiles, other_files);
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

void Dragable::startDrag(const QPoint& p)
{
	m->valid = isValidDragPosition(p);
	m->dragging = false;
	m->startDraggingPosition = p;
}

QDrag* Dragable::moveDrag(const QPoint& p)
{
	if(!m->valid){
		return nullptr;
	}

	int distance = (p - m->startDraggingPosition).manhattanLength();

	if(distance < QApplication::startDragDistance())
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
	m->startDraggingPosition = QPoint();
	m->drag = new QDrag(m->widget);

	QMimeData* data = dragableMimedata();
	if(data == nullptr) {
		return m->drag;
	}

	QStringList strings;
	if(!hasDragLabel()) {
		strings = m->getStrings(data);
	}

	else {
		strings.clear();
		strings << dragLabel();
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
		pm_width = std::max( pm_width, Gui::Util::textWidth(fm, str) );
	}

	pm_width += logo_width + 22;


	QPixmap cover = dragPixmap();
	if(cover.isNull()){
		cover = Gui::Util::pixmap("logo.png", Gui::Util::NoTheme, logo_size, true);
	}

	QPixmap pm(pm_width, pm_height);
	QPainter painter(&pm);
	painter.save();

	painter.fillRect(pm.rect(), m->widget->palette().highlight().color());
	painter.setPen(m->widget->palette().highlightedText().color());
	//painter.drawRect(0, 0, pm_width - 1, pm_height - 1);
	painter.drawPixmap(left_offset, (pm_height - logo_height) / 2, logo_height, logo_height, cover);
	painter.translate(logo_width + 15, font_padding + font_height - 2);

	for(const QString& str : Algorithm::AsConst(strings))
	{
		painter.drawText(0, 0, str);
		painter.translate(0, font_height + 2);
	}

	painter.restore();

	m->drag->setMimeData(data);
	m->drag->setPixmap(pm);
	m->drag->exec(Qt::IgnoreAction);

	return m->drag;
}


void Dragable::releaseDrag(Dragable::ReleaseReason reason)
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
	m->startDraggingPosition = QPoint();
}

bool Dragable::isValidDragPosition(const QPoint &p) const
{
	Q_UNUSED(p)
	return true;
}


QPixmap Dragable::dragPixmap() const
{
	return QPixmap();
}

bool Dragable::hasDragLabel() const
{
	return false;
}

QString Dragable::dragLabel() const
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

	connect(mpef, &Gui::MousePressedFilter::sigMousePressed, this, &DragableConnector::mousePressed);
	connect(mmef, &Gui::MouseMoveFilter::sigMouseMoved, this, &DragableConnector::mouseMoved);

	widget->viewport()->installEventFilter(mpef);
	widget->viewport()->installEventFilter(mmef);
}

Gui::DragableConnector::~DragableConnector() = default;

void Gui::DragableConnector::mousePressed(QMouseEvent* e)
{
	if(e->buttons() & Qt::LeftButton)
	{
		m->dragable->startDrag(e->pos());
	}
}

void DragableConnector::mouseMoved(QMouseEvent* e)
{
	QDrag* drag = m->dragable->moveDrag(e->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, this, &DragableConnector::dragDestroyed);
	}
}

void DragableConnector::dragDestroyed()
{
	m->dragable->releaseDrag(Dragable::ReleaseReason::Destroyed);
}
