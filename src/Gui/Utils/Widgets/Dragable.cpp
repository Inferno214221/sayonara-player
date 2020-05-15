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
#include "Gui/Utils/MimeData/MimeDataUtils.h"

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
		int playlists, dirs, tracks, otherFiles;
		playlists = dirs = tracks = otherFiles = 0;

		QList<QUrl> urls = data->urls();

		for(const QUrl& url : urls)
		{
			QString filename = url.toLocalFile();
			if(filename.isEmpty()){
				filename = url.toString(QUrl::PreferLocalFile);
			}

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
				otherFiles++;
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

		if(otherFiles > 0){
			ret << Lang::getWithNumber(Lang::NrFiles, otherFiles);
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

#include <QBuffer>
QDrag* Dragable::createDrag() const
{
	auto* drag = new QDrag(m->widget);

	QMimeData* mimeData = dragableMimedata();
	if(mimeData == nullptr) {
		return drag;
	}

	QStringList strings;
	if(!hasDragLabel()) {
		strings = m->getStrings(mimeData);
	}

	else {
		strings.clear();
		strings << dragLabel();
	}

	const QFontMetrics fm = m->widget->fontMetrics();
	const int	fontHeight = fm.height() + 8;
	const int	logoHeight = fontHeight + 8;
	const int	logoWidth = logoHeight;
	const QSize logoSize(logoWidth, logoHeight);
	const int	leftOffset = 4;
	const int	textHeight = strings.size() * (fontHeight + 2);
	const int	pmHeight = fontHeight * 2;
	const int	fontPadding = (pmHeight - textHeight) / 2 + 1;

	int pmWidth = logoHeight + 4;
	for(const QString& str : Algorithm::AsConst(strings))
	{
		pmWidth = std::max( pmWidth, Gui::Util::textWidth(fm, str) );
	}

	pmWidth += logoWidth + 50;

	const QString coverUrl = Gui::MimeData::coverUrl(mimeData);
	QPixmap cover(coverUrl);
	if(cover.isNull()){
		cover = Gui::Util::pixmap("logo.png", Gui::Util::NoTheme, logoSize, true);
	}

	QPixmap pm(pmWidth, pmHeight);
	QPainter painter(&pm);

	painter.fillRect(pm.rect(), m->widget->palette().highlight().color());
	painter.setPen(m->widget->palette().highlightedText().color());
	painter.drawRect(0, 0, pmWidth - 1, pmHeight - 1);
	painter.drawPixmap(leftOffset, (pmHeight - logoHeight) / 2, logoHeight, logoHeight, cover);
	painter.translate(logoWidth + 15, fontPadding + fontHeight - 2);

	for(const QString& str : Algorithm::AsConst(strings))
	{
		painter.drawText(0, 0, str);
		painter.translate(0, fontHeight + 2);
	}

	mimeData->setImageData(pm);
	drag->setMimeData(mimeData);
	drag->setPixmap(pm);
	drag->start();

	return drag;
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
		return nullptr;
	}

	if(m->dragging) {
		return nullptr;
	}

	if(m->drag) {
		delete m->drag;
	}

	m->drag = createDrag();
	m->dragging = true;
	m->startDraggingPosition = QPoint();

	return m->drag;
}


void Dragable::releaseDrag()
{
	if(!m){
		return;
	}

	m->drag = nullptr;
	m->dragging = false;
	m->startDraggingPosition = QPoint();
}

bool Dragable::isValidDragPosition(const QPoint &p) const
{
	Q_UNUSED(p)
	return true;
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

	auto* mpef = new Gui::MousePressedFilter(widget);
	auto* mmef = new Gui::MouseMoveFilter(widget);

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
	if(drag) {
		connect(drag, &QDrag::destroyed, this, &DragableConnector::dragDestroyed);
	}
}

void DragableConnector::dragDestroyed()
{
	m->dragable->releaseDrag();
}
