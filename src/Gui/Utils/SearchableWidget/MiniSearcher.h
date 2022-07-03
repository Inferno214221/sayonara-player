/* MiniSearcher.h */

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

#ifndef MINISEARCHER_H
#define MINISEARCHER_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include <QObject>
#include <QFrame>
#include <QMap>

class SearchableViewInterface;
class QEvent;
class QKeyEvent;
class QFocusEvent;
class QHideEvent;

namespace Gui
{
	class MiniSearchEventFilter :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigTabPressed();
			void sigFocusLost();

		public:
			using QObject::QObject;

		protected:
			bool eventFilter(QObject* o, QEvent* e) override;
	};

	class MiniSearcher :
		public Gui::WidgetTemplate<QFrame>
	{
		Q_OBJECT
		PIMPL(MiniSearcher)

		signals:
			void sigTextChanged(const QString&);
			void sigFindNextRow();
			void sigFindPrevRow();

		public:
			MiniSearcher(SearchableViewInterface* searchableView);
			~MiniSearcher() override;

			bool handleKeyPress(QKeyEvent* e);
			void setExtraTriggers(const QMap<QChar, QString>& triggers);
			void setNumberResults(int results);

		public slots:
			void reset();

		protected:
			void languageChanged() override;
			void keyPressEvent(QKeyEvent* e) override;
			void showEvent(QShowEvent* e) override;
			void focusOutEvent(QFocusEvent* e) override;

		private:
			void notifyViewSearchDone();
			void previousResult();
			void nextResult();
	};
}

#endif // MINISEARCHER_H
