/* CoverEditor.h */
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
#ifndef SAYONARA_PLAYER_COVEREDITOR_H
#define SAYONARA_PLAYER_COVEREDITOR_H

#include "Utils/Pimpl.h"
#include <QObject>

class QPixmap;
namespace Cover
{
	class Location;
};

namespace Tagging
{
	class Editor;
	class CoverEditor : public QObject
	{
		Q_OBJECT
		PIMPL(CoverEditor)

		public:
			CoverEditor(Editor* editor, QObject* parent);
			~CoverEditor();

			void setCurrentIndex(int index);
			void reset();
			void updateTrack(int index);
			void replaceCurrentCover(const QPixmap& pixmap);
			void replaceCoverForAll(const QPixmap& pixmap);

			bool isCoverForAllAvailable() const;
			QPixmap currentOriginalCover() const;
			QPixmap currentReplacementCover() const;
			Cover::Location currentCoverLocation() const;

			int count() const;
	};
}
#endif //SAYONARA_PLAYER_COVEREDITOR_H
