/* CoverButton.cpp */

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

#include "CoverButton.h"
#include "GUI_AlternativeCovers.h"

#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverUtils.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
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
	emit sig_finished();
}


struct CoverButton::Private
{
	QString					hash;
	Location				cover_location;
	QPixmap					invalid_cover;
	QPixmap					current_cover, current_cover_scaled;
	QPixmap					old_cover, old_cover_scaled;
	QByteArray				current_hash;

	QTimer*					timer=nullptr;
	Lookup*					cover_lookup=nullptr;
	double					opacity;
	Cover::Source			cover_source;
	bool					silent;

	Private() :
		cover_location(Location::invalid_location()),
		current_cover(Location::invalid_path()),
		opacity(1.0),
		silent(false)
	{
		invalid_cover = QPixmap(Cover::Location::invalid_path());
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

	Cover::ChangeNotfier* cn = Cover::ChangeNotfier::instance();
	connect(cn, &Cover::ChangeNotfier::sig_covers_changed, this, &CoverButton::covers_changed);

	connect(m->timer, &QTimer::timeout, this, &CoverButton::timer_timed_out);
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

void CoverButton::trigger()
{
	if(m->cover_source == Cover::Source::AudioFile && !is_silent())
	{
		emit sig_rejected();
		return;
	}

	GUI_AlternativeCovers* alt_cover = new GUI_AlternativeCovers(m->cover_location, m->silent, this->parentWidget());

	connect(alt_cover, &GUI_AlternativeCovers::sig_cover_changed, this, &CoverButton::alternative_cover_fetched);
	connect(alt_cover, &GUI_AlternativeCovers::sig_closed, alt_cover, &GUI_AlternativeCovers::deleteLater);

	alt_cover->show();
}


void CoverButton::set_cover_image(const QString& path)
{
	set_cover_image_pixmap(QPixmap(path));
}

void CoverButton::set_cover_image_pixmap(const QPixmap& pm)
{
	QPixmap pm_scaled = pm.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation);
	{ // check if current cover is the same
		auto h1 = Util::calc_hash(Util::cvt_pixmap_to_bytearray(pm_scaled));
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

	m->current_hash = Util::calc_hash (
		Util::cvt_pixmap_to_bytearray(m->current_cover.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation))
	);

	emit sig_cover_changed();

	// if timer is not active, start new timer loop
	if(!m->timer->isActive() && GetSetting(Set::Player_FadingCover))
	{
		m->opacity = 0;
		m->timer->start();
	}
}


void CoverButton::set_cover_data(const QByteArray& data, const QString& mimetype)
{
	auto* thread = new QThread();
	auto* worker = new ByteArrayConverter(data, mimetype);
	worker->moveToThread(thread);

	connect(worker, &ByteArrayConverter::sig_finished, this, &CoverButton::byteconverter_finished);
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	connect(thread, &QThread::started, worker, &ByteArrayConverter::start);

	thread->start();
}


void CoverButton::byteconverter_finished()
{
	auto* worker = static_cast<ByteArrayConverter*>(sender());
	this->set_cover_image_pixmap(worker->pixmap());

	worker->deleteLater();
}



void CoverButton::set_cover_location(const Location& cl)
{
	if(m->hash.size() > 0 && cl.hash() == m->hash){
		return;
	}

	m->hash = cl.hash();

	if(!cl.is_valid())
	{
		set_cover_image_pixmap(m->invalid_cover);
	}

	m->cover_location = cl;

	if(cl.hash().isEmpty() || !cl.is_valid()) {
		return;
	}

	if(!m->cover_lookup)
	{
		m->cover_lookup = new Lookup(cl, 1, this);

		connect(m->cover_lookup, &Lookup::sig_cover_found, this, &CoverButton::set_cover_image_pixmap);
		connect(m->cover_lookup, &Lookup::sig_finished, this, &CoverButton::cover_lookup_finished);
	}

	else {
		m->cover_lookup->set_cover_location(cl);
	}

	m->cover_lookup->start();
}



void CoverButton::cover_lookup_finished(bool success)
{
	if(!success)
	{
		sp_log(Log::Warning, this) << "Cover lookup finished: false";
		set_cover_image(Location::invalid_path());
	}

	auto* lookup = static_cast<Cover::Lookup*>(sender());
	m->cover_source = lookup->source();
}


void CoverButton::covers_changed()
{
	if(!is_silent())
	{
		m->hash = QString();
		set_cover_location(m->cover_location);
	}
}


void CoverButton::alternative_cover_fetched(const Location& cl)
{
	m->hash = QString();
	m->cover_source = Cover::Source::Unknown;

	if(!is_silent())
	{
		if(cl.is_valid())
		{
			ChangeNotfier::instance()->shout();
		}

		set_cover_image(cl.cover_path());
	}

	else
	{
		set_cover_image(cl.alternative_path());
	}
}



void CoverButton::set_silent(bool silent)
{
	m->silent = silent;
}

bool CoverButton::is_silent() const
{
	return m->silent;
}



void CoverButton::timer_timed_out()
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
	}
}

void CoverButton::resizeEvent(QResizeEvent* e)
{
	Gui::WidgetTemplate<QPushButton>::resizeEvent(e);

	int h = this->height() - 2;
	int w = this->width() - 2;

	m->current_cover_scaled = m->current_cover.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}



static bool check_if_within_cover(QPoint pos, QRect geometry)
{
	int difference = geometry.width() - geometry.height();
	if(difference > 0)
	{
		int smaller_site = geometry.height();

		if ((pos.x() < (difference / 2)) ||
			(pos.x() >= (difference / 2 + smaller_site)))
		{
			return false;
		}
	}

	return true;
}

void CoverButton::mouseMoveEvent(QMouseEvent* event)
{
	bool within = check_if_within_cover(event->pos() - this->geometry().topLeft(), this->geometry());
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
		bool within = check_if_within_cover(event->pos() - this->geometry().topLeft(), this->geometry());
		if(!within)
		{
			QPushButton::mouseReleaseEvent(event);
			return;
		}

		trigger();
	}

	QPushButton::mouseReleaseEvent(event);
}

