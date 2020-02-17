/* CoverButton.cpp */

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

#include "CoverButton.h"
#include "GUI_AlternativeCovers.h"

#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverUtils.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QPainter>
#include <QMenu>
#include <QTimer>
#include <QThread>

using Gui::CoverButton;
using Gui::ByteArrayConverter;
using Cover::Location;
using Cover::Lookup;
using Cover::ChangeNotfier;
using CoverButtonBase=Gui::WidgetTemplate<QPushButton>;

struct ByteArrayConverter::Private
{
	QByteArray	data;
	QString		mime;

	QPixmap		pixmap;

	Private(const QByteArray& data, const QString& mime) :
		data(data),
		mime(mime)
	{}
};

ByteArrayConverter::ByteArrayConverter(const QByteArray& data, const QString& mime) :
	QObject()
{
	m = Pimpl::make<Private>(data, mime);
}

ByteArrayConverter::~ByteArrayConverter() = default;

QPixmap ByteArrayConverter::pixmap() const
{
	return m->pixmap;
}

void ByteArrayConverter::start()
{
	m->pixmap.loadFromData(m->data, m->mime.toLocal8Bit().data());
	emit sigFinished();
}


struct CoverButton::Private
{
	QString					hash;
	Location				coverLocation;
	QPixmap					invalid_cover;
	QPixmap					current_cover, current_cover_scaled;
	QPixmap					old_cover, old_cover_scaled;
	QByteArray				current_hash;
	QRect					pixmap_rect;

	QTimer*					timer=nullptr;
	Lookup*					cover_lookup=nullptr;
	double					opacity;
	Cover::Source			cover_source;
	bool					silent;
	bool					alternative_search_enabled;

	Private() :
		coverLocation(Location::invalidLocation()),
		current_cover(Location::invalidPath()),
		opacity(1.0),
		silent(false),
		alternative_search_enabled(true)
	{
		invalid_cover = QPixmap(Cover::Location::invalidPath());
	}
};


CoverButton::CoverButton(QWidget* parent) :
	CoverButtonBase(parent)
{
	m = Pimpl::make<CoverButton::Private>();
	m->timer = new QTimer(this);
	m->timer->setTimerType(Qt::TimerType::PreciseTimer);
	m->timer->setInterval(10);

	this->setObjectName("CoverButton");
	this->setMouseTracking(true);
	this->setFlat(true);
	this->setToolTip(tr("Search an alternative cover"));

	auto* cn = Cover::ChangeNotfier::instance();
	connect(cn, &Cover::ChangeNotfier::sigCoversChanged, this, &CoverButton::coversChanged);

	connect(m->timer, &QTimer::timeout, this, &CoverButton::timerTimedOut);
}

CoverButton::~CoverButton()
{
	if(m->cover_lookup)
	{
		m->cover_lookup->stop();
		m->cover_lookup->deleteLater();
	}
}

QPixmap CoverButton::pixmap() const
{
	return m->current_cover;
}

int CoverButton::verticalPadding() const
{
	int p = (this->height() - m->current_cover_scaled.size().height()) - 2;
	if(p <= 0){
		p = -(this->width() - m->current_cover_scaled.size().width() - 2);
	}

	return p;
}

void CoverButton::setAlternativeSearchEnabled(bool b)
{
	m->alternative_search_enabled = b;
}

bool CoverButton::isAlternativeSearchEnabled() const
{
	return m->alternative_search_enabled;
}

void CoverButton::trigger()
{
	if(m->cover_source == Cover::Source::AudioFile && !isSilent())
	{
		emit sigRejected();
		return;
	}

	if(m->alternative_search_enabled)
	{
		auto* alt_cover = new GUI_AlternativeCovers(m->coverLocation, m->silent, this->parentWidget());

		connect(alt_cover, &GUI_AlternativeCovers::sigCoverChanged, this, &CoverButton::alternativeCoverFetched);
		connect(alt_cover, &GUI_AlternativeCovers::sigClosed, alt_cover, &GUI_AlternativeCovers::deleteLater);

		alt_cover->show();
	}

	else
	{
		emit sigRejected();
	}
}


void CoverButton::setCoverImage(const QString& path)
{
	setCoverImagePixmap(QPixmap(path));
}

void CoverButton::setCoverImagePixmap(const QPixmap& pm)
{
	QPixmap pm_scaled = pm.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation);
	{ // check if current cover is the same
		auto h1 = Util::calcHash(Util::convertPixmapToByteArray(pm_scaled));
		if(h1 == m->current_hash && !pm.isNull()){
			return;
		}
	}

	// don't change the currently fading out cover
	if(!m->timer->isActive() && GetSetting(Set::Player_FadingCover))
	{
		m->old_cover = m->current_cover;
		m->old_cover_scaled = m->current_cover_scaled;
	}

	int h = this->height() - 2;
	int w = this->width() - 2;

	m->current_cover = pm;
	m->current_cover_scaled = m->current_cover.scaled(QSize(w,h), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	this->setToolTip(QString("%1x%2").arg(pm.width()).arg(pm.height()));

	m->current_hash = Util::calcHash (
		Util::convertPixmapToByteArray(m->current_cover.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation))
	);

	emit sigCoverChanged();

	// if timer is not active, start new timer loop
	if(!m->timer->isActive() && GetSetting(Set::Player_FadingCover))
	{
		m->opacity = 0;
		m->timer->start();
	}
}


void CoverButton::setCoverData(const QByteArray& data, const QString& mimetype)
{
	auto* thread = new QThread();
	auto* worker = new ByteArrayConverter(data, mimetype);
	worker->moveToThread(thread);

	connect(worker, &ByteArrayConverter::sigFinished, this, &CoverButton::byteconverterFinished);
	connect(worker, &ByteArrayConverter::sigFinished, thread, &QThread::quit);
	connect(thread, &QThread::started, worker, &ByteArrayConverter::start);
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);

	thread->start();
}


void CoverButton::byteconverterFinished()
{
	auto* worker = static_cast<ByteArrayConverter*>(sender());
	this->setCoverImagePixmap(worker->pixmap());

	worker->deleteLater();
}

void CoverButton::setCoverLocation(const Location& cl)
{
	if(m->hash.size() > 0 && cl.hash() == m->hash){
		return;
	}

	m->hash = cl.hash();

	if(!cl.isValid())
	{
		setCoverImagePixmap(m->invalid_cover);
	}

	m->coverLocation = cl;

	if(cl.hash().isEmpty() || !cl.isValid()) {
		return;
	}

	if(!m->cover_lookup)
	{
		m->cover_lookup = new Lookup(cl, 1, this);

		connect(m->cover_lookup, &Lookup::sigCoverFound, this, &CoverButton::setCoverImagePixmap);
		connect(m->cover_lookup, &Lookup::sigFinished, this, &CoverButton::coverLookupFinished);
	}

	else {
		m->cover_lookup->setCoverLocation(cl);
	}

	m->cover_lookup->start();
}



void CoverButton::coverLookupFinished(bool success)
{
	if(!success)
	{
		spLog(Log::Warning, this) << "Cover lookup finished: false";
		setCoverImage(Location::invalidPath());
	}

	auto* lookup = static_cast<Cover::Lookup*>(sender());
	m->cover_source = lookup->source();
}


void CoverButton::coversChanged()
{
	if(!isSilent())
	{
		m->hash = QString();
		setCoverLocation(m->coverLocation);
	}
}


void CoverButton::alternativeCoverFetched(const Location& cl)
{
	m->hash = QString();
	m->cover_source = Cover::Source::Unknown;

	if(!isSilent())
	{
		if(cl.isValid())
		{
			ChangeNotfier::instance()->shout();
		}

		setCoverImage(cl.coverPath());
	}

	else
	{
		setCoverImage(cl.alternativePath());
	}
}



void CoverButton::setSilent(bool silent)
{
	m->silent = silent;
}

bool CoverButton::isSilent() const
{
	return m->silent;
}

void CoverButton::timerTimedOut()
{
	m->opacity = std::min(1.0, m->opacity + 0.025);

	if(m->opacity < 1.0)
	{
		repaint();
	}

	else
	{
		m->old_cover = QPixmap();
		m->old_cover_scaled = QPixmap();
		m->timer->stop();
	}
}


void CoverButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	if(m->current_cover_scaled.isNull()){
		return;
	}

	QPainter painter(this);

	int h = this->height() - 2;
	int w = this->width() - 2;

	QPixmap pm = m->current_cover_scaled;
	QPixmap pm_old;
	if(!m->old_cover_scaled.isNull() && GetSetting(Set::Player_FadingCover))
	{
		pm_old = m->old_cover_scaled;
	}

	int x = (w - pm.width()) / 2;
	int y = (h - pm.height()) / 2;

	if(!pm_old.isNull())
	{
		int x_old = (w - pm_old.width()) / 2;
		int y_old = (h - pm_old.height()) / 2;

		painter.setOpacity(1.0 - m->opacity);
		painter.drawPixmap
		(
			x_old, y_old, pm_old.width(), pm_old.height(),
			pm_old
		);

		m->pixmap_rect = QRect(x_old, y_old, pm_old.width(), pm_old.height());

		painter.setOpacity(m->opacity);
	}

	else {
		painter.setOpacity(1.0);
	}

	if(!pm.isNull())
	{
		painter.drawPixmap
		(
			x, y, pm.width(), pm.height(),
			pm
		);

		m->pixmap_rect = QRect(x, y, pm.width(), pm.height());
	}
}

void CoverButton::resizeEvent(QResizeEvent* e)
{
	Gui::WidgetTemplate<QPushButton>::resizeEvent(e);

	int h = this->height() - 2;
	int w = this->width() - 2;

	m->current_cover_scaled = m->current_cover.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CoverButton::mouseMoveEvent(QMouseEvent* event)
{
	bool within = m->pixmap_rect.contains(event->pos());
	QCursor c;

	if(within)
	{
		c.setShape(Qt::PointingHandCursor);
	}

	else
	{
		c.setShape(Qt::ArrowCursor);
	}

	this->setCursor(c);

	QPushButton::mouseMoveEvent(event);
}


void CoverButton::mouseReleaseEvent(QMouseEvent* event)
{
	if(event->button() | Qt::LeftButton)
	{
		bool within = m->pixmap_rect.contains(event->pos());
		if(!within)
		{
			QPushButton::mouseReleaseEvent(event);
			return;
		}

		trigger();
	}

	QPushButton::mouseReleaseEvent(event);
}

