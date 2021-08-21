/* Cover.cpp */
/* Sayonara Player

  Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Cover.h"

#include <optional>

ID3v2::CoverFrame::CoverFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<Models::Cover, TagLib::ID3v2::AttachedPictureFrame>(tag, "APIC") {}

ID3v2::CoverFrame::~CoverFrame() = default;

void ID3v2::CoverFrame::mapDataToFrame(const Models::Cover& cover, TagLib::ID3v2::AttachedPictureFrame* frame)
{
	const auto description = TagLib::String("Cover by Sayonara Player");
	const auto encoding = TagLib::String::Latin1;
	const auto mimeType = TagLib::String(cover.mimeType.toLatin1().constData());
	const auto type = TagLib::ID3v2::AttachedPictureFrame::FrontCover;

	const auto& imageData = cover.imageData;

	TagLib::ByteVector taglibImageData;
	taglibImageData.setData(imageData.data(), quint32(imageData.size()));

	frame->setDescription(description);
	frame->setTextEncoding(encoding);
	frame->setMimeType(mimeType);
	frame->setType(type);
	frame->setPicture(taglibImageData);

	const auto headerData = TagLib::ByteVector("APIC", 4);

	auto renderedFrame = frame->render();
	if(!renderedFrame.startsWith(headerData))
	{
		renderedFrame = headerData + renderedFrame;
	}

	frame->setData(renderedFrame);
}

std::optional<Models::Cover> ID3v2::CoverFrame::mapFrameToData(const TagLib::ID3v2::AttachedPictureFrame* frame) const
{
	const auto taglibImageData = frame->picture();
	const auto taglibMimeType = frame->mimeType();

	auto cover = Models::Cover {};
	cover.imageData = QByteArray(taglibImageData.data(), qint32(taglibImageData.size()));
	cover.mimeType = QString::fromLatin1(taglibMimeType.toCString(), qint32(taglibMimeType.length()));

	return std::optional(cover);
}

TagLib::ID3v2::Frame* ID3v2::CoverFrame::createId3v2Frame()
{
	return new TagLib::ID3v2::AttachedPictureFrame(TagLib::ByteVector());
}
