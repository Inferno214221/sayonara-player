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
	QPixmap					invalidCover;
	QPixmap					currentCover, currentCoverScaled;
	QPixmap					oldCover, oldCoverScaled;
	QByteArray				currentHash;
	QRect					pixmapRect;

	QTimer*					timer=nullptr;
	Lookup*					coverLookup=nullptr;
	double					opacity;
	Cover::Source			coverSource;
	bool					silent;
	bool					alternativeSearchEnabled;

	Private() :
		coverLocation(Location::invalidLocation()),
		currentCover(Location::invalidPath()),
		opacity(1.0),
		silent(false),
		alternativeSearchEnabled(true)
	{
		invalidCover = QPixmap(Cover::Location::invalidPath());
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
	if(m->coverLookup)
	{
		m->coverLookup->stop();
		m->coverLookup->deleteLater();
	}
}

QPixmap CoverButton::pixmap() const
{
	return m->currentCover;
}

int CoverButton::verticalPadding() const
{
	if(m->currentCoverScaled.isNull()) 
	{
		// if this is not handled, the padding is always
		// approx the button height. But that should never
		// happen as the invalid cover should be set when 
		// possible
		return 0;
	}

	int p = (this->height() - m->currentCoverScaled.size().height()) - 2;
	if(p <= 0){
		p = -(this->width() - m->currentCoverScaled.size().width() - 2);
	}

	return p;
}

void CoverButton::setAlternativeSearchEnabled(bool b)
{
	m->alternativeSearchEnabled = b;
}

bool CoverButton::isAlternativeSearchEnabled() const
{
	return m->alternativeSearchEnabled;
}

void CoverButton::trigger()
{
	if(m->coverSource == Cover::Source::AudioFile && !isSilent())
	{
		emit sigRejected();
		return;
	}

	if(m->alternativeSearchEnabled)
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

void CoverButton::setCoverImagePixmap(const QPixmap& _pm)
{
	QPixmap pm(_pm);
	if(pm.isNull()) {
		pm = m->invalidCover;
	}

	{ // check if the image is already the same
		QPixmap pmScaled50 = pm.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation);
		auto newHash = Util::calcHash(Util::convertPixmapToByteArray(pmScaled50));
		if(newHash == m->currentHash) {
			return;
		}

		m->currentHash = newHash;
	}

	// don't change the currently fading out cover
	if(!m->timer->isActive() && GetSetting(Set::Player_FadingCover))
	{
		m->oldCover = m->currentCover;
		m->oldCoverScaled = m->currentCoverScaled;
	}

	m->currentCover = pm;
	m->currentCoverScaled = m->currentCover.scaled
	(
		(this->size() - QSize(2, 2)),
		Qt::KeepAspectRatio,
		Qt::SmoothTransformation
	);

	this->setToolTip
	(
		QString("%1x%2")
			.arg(pm.width())
			.arg(pm.height())
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

	const QPixmap pm = worker->pixmap();
	if(!pm.isNull()) {
		this->setCoverImagePixmap(pm);
	}

	else {
		spLog(Log::Warning, this) << "Cover from track seems invalid or broken";
	}

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
		setCoverImagePixmap(m->invalidCover);
	}

	m->coverLocation = cl;

	if(cl.hash().isEmpty() || !cl.isValid()) {
		return;
	}

	if(!m->coverLookup)
	{
		m->coverLookup = new Lookup(cl, 1, this);

		connect(m->coverLookup, &Lookup::sigCoverFound, this, &CoverButton::setCoverImagePixmap);
		connect(m->coverLookup, &Lookup::sigFinished, this, &CoverButton::coverLookupFinished);
	}

	else {
		m->coverLookup->setCoverLocation(cl);
	}

	m->coverLookup->start();
}

void CoverButton::coverLookupFinished(bool success)
{
	if(!success)
	{
		spLog(Log::Warning, this) << "Cover lookup finished: false";
		setCoverImage(Location::invalidPath());
	}

	auto* lookup = static_cast<Cover::Lookup*>(sender());
	m->coverSource = lookup->source();
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
	m->coverSource = Cover::Source::Unknown;

	if(!isSilent())
	{
		if(cl.isValid())
		{
			ChangeNotfier::instance()->shout();
		}
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
		m->oldCover = QPixmap();
		m->oldCoverScaled = QPixmap();
		m->timer->stop();
	}
}

void CoverButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	if(m->currentCoverScaled.isNull()){
		return;
	}

	QPainter painter(this);

	int h = this->height() - 2;
	int w = this->width() - 2;

	QPixmap pm = m->currentCoverScaled;
	QPixmap pmOld;
	if(!m->oldCoverScaled.isNull() && GetSetting(Set::Player_FadingCover))
	{
		pmOld = m->oldCoverScaled;
	}

	int x = (w - pm.width()) / 2;
	int y = (h - pm.height()) / 2;

	if(!pmOld.isNull())
	{
		int xOld = (w - pmOld.width()) / 2;
		int yOld = (h - pmOld.height()) / 2;

		painter.setOpacity(1.0 - m->opacity);
		painter.drawPixmap
		(
			xOld, yOld, pmOld.width(), pmOld.height(),
			pmOld
		);

		m->pixmapRect = QRect(xOld, yOld, pmOld.width(), pmOld.height());

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

		m->pixmapRect = QRect(x, y, pm.width(), pm.height());
	}
}

void CoverButton::resizeEvent(QResizeEvent* e)
{
	Gui::WidgetTemplate<QPushButton>::resizeEvent(e);

	int h = this->height() - 2;
	int w = this->width() - 2;

	m->currentCoverScaled = m->currentCover.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CoverButton::mouseMoveEvent(QMouseEvent* event)
{
	bool within = m->pixmapRect.contains(event->pos());
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
		bool within = m->pixmapRect.contains(event->pos());
		if(!within)
		{
			QPushButton::mouseReleaseEvent(event);
			return;
		}

		trigger();
	}

	QPushButton::mouseReleaseEvent(event);
}
