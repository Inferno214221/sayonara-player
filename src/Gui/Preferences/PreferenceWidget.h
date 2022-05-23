/* PreferenceWidgetInterface.h */

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

#ifndef SAYONARA_PLAYER_PREFERENCEWIDGET_H
#define SAYONARA_PLAYER_PREFERENCEWIDGET_H

#include "Gui/Utils/GuiClass.h"
#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

namespace Preferences
{
	class Base :
		public Gui::Widget
	{
		Q_OBJECT
		PIMPL(Base)

		public:
			explicit Base(const QString& identifier);
			~Base() override;

			[[nodiscard]] virtual bool isUiInitialized() const final;
			[[nodiscard]] virtual QAction* action() final;

			[[nodiscard]] virtual QString actionName() const = 0;
			[[nodiscard]] QString identifier() const;

			virtual bool commit() = 0;
			virtual void revert() = 0;
			virtual void initUi() = 0;
			virtual void retranslate() = 0;

			[[nodiscard]] virtual bool hasError() const;
			[[nodiscard]] virtual QString errorString() const;

		protected:
			template<typename W, typename UiClass>
			void setupParent(W* widget, UiClass** ui)
			{
				*ui = new UiClass();
				(*ui)->setupUi(widget);

				setInitialized();

				widget->languageChanged();

				skinChanged();
			}

			void languageChanged() final;
			void translationAction();

			void showEvent(QShowEvent* e) override;
			void closeEvent(QCloseEvent* e) override;

		private:
			void setInitialized();
	};
}

#endif // SAYONARA_PLAYER_PREFERENCEWIDGET_H
