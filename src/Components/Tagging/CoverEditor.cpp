/* CoverEditor.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "CoverEditor.h"
#include "Editor.h"

#include "Utils/globals.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Components/Covers/CoverLocation.h"

#include <QMap>
#include <QPixmap>

namespace
{
	class CoverMap
	{
		private:
			QMap<int, QPixmap> indexCoverMap;
			QPixmap coverForAll;

		public:
			void setCover(int index, const QPixmap& pixmap)
			{
				indexCoverMap[index] = pixmap;
			}

			void setCoverForAll(const QPixmap& pixmap)
			{
				coverForAll = pixmap;
			}

			QPixmap cover(int index) const
			{
				return (coverForAll.isNull())
				       ? indexCoverMap[index]
				       : coverForAll;
			}

			bool isCoverForAllAvailable() const
			{
				return !coverForAll.isNull();
			}

			void reset()
			{
				indexCoverMap.clear();
				coverForAll = QPixmap();
			}
	};
}

namespace Tagging
{
	struct CoverEditor::Private
	{
		Editor* tagEditor;
		int currentIndex {-1};
		CoverMap coverMap;

		Private(Editor* tagEditor) :
			tagEditor {tagEditor} {}
	};

	CoverEditor::CoverEditor(Editor* editor, QObject* parent) :
		QObject(parent)
	{
		m = Pimpl::make<Private>(editor);
	}

	CoverEditor::~CoverEditor() = default;

	void CoverEditor::setCurrentIndex(int index)
	{
		m->currentIndex = index;
	}

	void CoverEditor::reset()
	{
		m->currentIndex = -1;
		m->coverMap.reset();
	}

	void CoverEditor::updateTrack(int index)
	{
		m->tagEditor->updateCover(index, m->coverMap.cover(index));
	}

	void CoverEditor::replaceCurrentCover(const QPixmap& pixmap)
	{
		m->coverMap.setCover(m->currentIndex, pixmap);
	}

	void CoverEditor::replaceCoverForAll(const QPixmap& pixmap)
	{
		m->coverMap.setCoverForAll(pixmap);
	}

	QPixmap CoverEditor::currentOriginalCover() const
	{
		if(!Util::between(m->currentIndex, m->tagEditor->count()))
		{
			return QPixmap();
		}

		const auto track = m->tagEditor->metadata(m->currentIndex);
		return Tagging::extractCover(track.filepath());
	}

	QPixmap CoverEditor::currentReplacementCover() const
	{
		return Util::between(m->currentIndex, m->tagEditor->count())
		       ? m->coverMap.cover(m->currentIndex)
		       : QPixmap();
	}

	Cover::Location CoverEditor::currentCoverLocation() const
	{
		if(!Util::between(m->currentIndex, m->tagEditor->count()))
		{
			return Cover::Location::invalidLocation();
		}

		const auto track = m->tagEditor->metadata(m->currentIndex);
		return Cover::Location::coverLocation(track);
	}

	int CoverEditor::count() const
	{
		return m->tagEditor->count();
	}

	bool CoverEditor::isCoverForAllAvailable() const
	{
		return m->coverMap.isCoverForAllAvailable();
	}
}