/* CoverButton.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/Language.h"

#include <QPainter>
#include <QMenu>

using Cover::Location;
using Cover::Lookup;
using Cover::ChangeNotfier;
using CoverButtonBase=Gui::WidgetTemplate<QPushButton>;

struct CoverButton::Private
{
	Lookup*			cover_lookup=nullptr;
	Location		cover_location;
	QString			current_cover_path;
	QString			class_tmp_file;
	bool			cover_forced;

	Private() :
		cover_forced(false)
	{
		class_tmp_file = Cover::Util::cover_directory("cb_" + Util::random_string(16) + ".jpg");
	}

	~Private()
	{
		::Util::File::delete_files({class_tmp_file});
	}
};


CoverButton::CoverButton(QWidget* parent) :
	CoverButtonBase(parent)
{
	m = Pimpl::make<CoverButton::Private>();

	this->setObjectName("CoverButton");

	m->current_cover_path = Location::invalid_location().preferred_path();
	m->cover_location = Location::invalid_location();

	connect(this, &QPushButton::clicked, this, &CoverButton::cover_button_clicked);

	Cover::ChangeNotfier* cn = Cover::ChangeNotfier::instance();
	connect(cn, &Cover::ChangeNotfier::sig_covers_changed, this, &CoverButton::refresh);
}

CoverButton::~CoverButton() {}

void CoverButton::refresh()
{
	this->setIcon(current_icon());
}


void CoverButton::set_cover_image(const QString& cover_path)
{
	m->current_cover_path = cover_path;
	m->cover_forced = false;

	this->refresh();
}


void CoverButton::set_cover_location(const Location& cl)
{
	this->setToolTip("Cover source: " + cl.identifer());

	m->cover_location = cl;
	m->cover_forced = false;

	if(!m->cover_lookup)
	{
		m->cover_lookup = new Lookup(this);
		connect(m->cover_lookup, &Lookup::sig_cover_found, this, &CoverButton::set_cover_image);
		connect(m->cover_lookup, &Lookup::sig_finished, this, &CoverButton::cover_lookup_finished);
	}

	set_cover_image(Cover::Location::invalid_location().cover_path());

	m->cover_lookup->fetch_cover(cl);
}



QIcon CoverButton::current_icon() const
{
	QIcon icon;
	QPixmap pm = QPixmap(m->current_cover_path)
			.scaled(this->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	if(pm.isNull()){
		sp_log(Log::Warning, this) << "Pixmap not valid";
		return QIcon();
	}

	for(QIcon::Mode m : { QIcon::Mode::Normal, QIcon::Mode::Disabled, QIcon::Mode::Active, QIcon::Mode::Selected })
	{
		for(QIcon::State s : {QIcon::State::On, QIcon::State::Off})
		{
			icon.addPixmap(pm, m, s);
		}
	}

	return icon;
}


void CoverButton::cover_button_clicked()
{
	// The cover comes from the ID3 file
	if(m->cover_forced)
	{
		emit sig_rejected();
	}

	else
	{
		GUI_AlternativeCovers* alt_cover = new GUI_AlternativeCovers(this->parentWidget());

		connect(alt_cover, &GUI_AlternativeCovers::sig_cover_changed, this, &CoverButton::alternative_cover_fetched );
		connect(alt_cover, &GUI_AlternativeCovers::sig_closed, alt_cover, &GUI_AlternativeCovers::deleteLater);

		alt_cover->start(m->cover_location);
	}
}


void CoverButton::alternative_cover_fetched(const Location& cl)
{
	if(cl.valid()){
		ChangeNotfier::instance()->shout();
	}

	set_cover_image(cl.cover_path());
}


void CoverButton::cover_found(const Location& cl)
{
	this->setToolTip("Cover source: " + cl.identifer());

	/* If cover was forced while CoverLookup was still running */
	if(m->cover_forced && (sender() == m->cover_lookup)) {
		return;
	}

	set_cover_image(cl.cover_path());
}

void CoverButton::cover_lookup_finished(bool success)
{
	if(!success){
		sp_log(Log::Warning, this) << "Cover lookup finished: false";
		set_cover_image(Location::invalid_location().preferred_path());
	}
}


void CoverButton::force_cover(const QPixmap& pm)
{
	if(!_settings->get<Set::Cover_LoadFromFile>()){
		return;
	}

	this->setToolTip(tr("Cover source: File"));

	m->current_cover_path = m->class_tmp_file;
	m->cover_forced = true;

	pm.save(m->current_cover_path);

	refresh();
}


void CoverButton::force_cover(const QImage& img)
{
	force_cover(QPixmap::fromImage(img));
}


void CoverButton::showEvent(QShowEvent* e)
{
	this->setFlat(true);
	this->setToolTip(tr("Search an alternative cover"));

	CoverButtonBase::showEvent(e);
}


void CoverButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	QPainter painter(this);
	painter.save();

	int sz = std::min(this->height(), this->width()) - 2;
	QSize size(sz, sz);

	QPixmap pm = QPixmap(m->current_cover_path)
			.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	int x = (this->width() - sz) / 2;

	painter.drawPixmap(
		QRect(x, 1, sz, sz),
		pm
	);

	painter.restore();
}

