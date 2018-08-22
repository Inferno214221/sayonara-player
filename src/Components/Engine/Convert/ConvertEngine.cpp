/* ConvertEngine.cpp */

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

#include "ConvertEngine.h"
#include "ConvertPipeline.h"

#include "Components/Engine/Callbacks/PipelineCallbacks.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QUrl>

using Engine::Convert;

struct Convert::Private
{
	Pipeline::Convert*  pipeline=nullptr;
	MetaData            md_target;
	gchar*              target_uri=nullptr;

	Private(Engine::Base* parent)
	{
		pipeline = new Pipeline::Convert(parent);
	}
};

Convert::Convert(QObject *parent) :
	Base(Name::ConvertEngine, parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->pipeline, &Pipeline::Convert::sig_pos_changed_ms, this, &Convert::cur_pos_ms_changed);
}

Convert::~Convert() {}

bool Convert::init()
{
	return m->pipeline->init();
}


bool Convert::change_track(const MetaData& md)
{
	configure_target(md);

	return Base::change_track(md);
}

bool Convert::change_track_by_filename(const QString& filename)
{
	Q_UNUSED(filename);
	return false;
}


void Convert::play()
{
	m->pipeline->play();
}

void Convert::pause()
{
	return;
}

void Convert::stop()
{
	m->pipeline->stop();

	Tagging::Util::setMetaDataOfFile(m->md_target);

	Base::stop();
}

// public from Gstreamer Callbacks
void Convert::set_track_finished(GstElement* src)
{
	Q_UNUSED(src)
	emit sig_track_finished();
}

void Convert::cur_pos_ms_changed(MilliSeconds pos_ms)
{
	if(sender() != m->pipeline){
		return;
	}

	Base::set_current_position_ms(pos_ms);
}

bool Convert::change_uri(char* uri)
{
	return m->pipeline->set_uri(uri);
}

void Convert::configure_target(const MetaData& md)
{
	if(m->target_uri){
		g_free(m->target_uri);
		m->target_uri = nullptr;
	}

	QString filename = Util::File::get_filename_of_path(md.filepath());
	int idx = filename.lastIndexOf(".");
	if(idx > 0) {
		filename = filename.left(idx);
	}

	QString cvt_target_path = _settings->get<Set::Engine_CovertTargetPath>();
	filename = cvt_target_path + "/" + filename + ".mp3";

	gchar* target_uri = filename.toUtf8().data();

	m->target_uri = g_strdup(target_uri);
	m->pipeline->set_target_uri(m->target_uri);

	m->md_target = md;
	m->md_target.set_filepath(filename);
}

void Convert::jump_abs_ms(MilliSeconds pos_ms) { Q_UNUSED(pos_ms); }

void Convert::jump_rel_ms(MilliSeconds ms) { Q_UNUSED(ms); }

void Convert::jump_rel(double percent) { Q_UNUSED(percent); }

void Convert::update_metadata(const MetaData& md, GstElement* src) { Q_UNUSED(src); Q_UNUSED(md) }
