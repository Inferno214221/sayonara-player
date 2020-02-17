/* Discnumber.cpp */

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

#include "Discnumber.h"
#include <QStringList>

ID3v2::DiscnumberFrame::DiscnumberFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<Models::Discnumber, TagLib::ID3v2::TextIdentificationFrame>(tag, "TPOS") {}

ID3v2::DiscnumberFrame::~DiscnumberFrame() {}

void ID3v2::DiscnumberFrame::map_model_to_frame(const Models::Discnumber& model, TagLib::ID3v2::TextIdentificationFrame* frame)
{
	TagLib::String str(model.to_string().toLatin1().constData(), TagLib::String::Latin1);
	frame->setText(str);
}

void ID3v2::DiscnumberFrame::map_frame_to_model(const TagLib::ID3v2::TextIdentificationFrame* frame, Models::Discnumber& model)
{
	TagLib::String text = frame->toString();
	QString str = QString::fromLatin1(text.toCString());

	QStringList lst = str.split('/');
	if(lst.size() > 0)
	{
		model.disc = lst[0].toInt();
	}

	if(lst.size() > 1)
	{
		model.disccount = lst[1].toInt();
	}
}

TagLib::ID3v2::Frame* ID3v2::DiscnumberFrame::create_id3v2_frame()
{
	return new TagLib::ID3v2::TextIdentificationFrame("TPOS", TagLib::String::Latin1);
}

