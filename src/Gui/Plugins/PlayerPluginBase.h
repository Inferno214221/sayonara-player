/* PlayerPlugin.h */

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

#ifndef PLAYERPLUGIN_H
#define PLAYERPLUGIN_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

class QAction;

/**
 * @brief Interface for PlayerPlugin classes.
 *   get_name() and language_changed() must be overwritten
 * @ingroup Interfaces
 */

namespace PlayerPlugin
{
	class Handler;
	class Base :
		public Gui::Widget
	{
		friend class Handler;

		Q_OBJECT

	private:
		PIMPL(Base)

	public:
		explicit Base(QWidget* parent=nullptr);
		virtual ~Base() override;

	signals:
		/**
		 * @brief signal is emitted when the plugin action is triggered\n
		 * also emitted for when closeEvent is fired
		 * @param plugin this pointer to current plugin
		 * @param checked indicates whether checked or unchecked
		 */
		void sigActionTriggered(bool checked);

		/**
		 * @brief emitted when reloading is requested, after firing this signal
		 * the plugin will be painted new. Useful, if the size has changed
		 */
		void sigReload(PlayerPlugin::Base* plugin);

		void sigOpened();


	private slots:
		/**
		 * @brief Checks/unchecks the action and emits sig_action_triggered signal
		 * also called when closeEvent is fired
		 * @param checked if action is checked or unchecked
		 */
		void actionTriggered(bool checked);

	private:

		/**
		 * @brief mark ui as initialized
		 */
		void setUiInitialized();


		/**
		 * @brief languageChanged. Calls retranslate_ui in subclasses
		 */
		virtual void languageChanged() final override;

		/**
		 * @brief GUI will be initialized on first show up. Please use this to make Sayonara starting fast
		 */
		virtual void initUi()=0;


	protected:
		virtual void finalizeInitialization();

		/**
		 * @brief Check if ui already was initialized
		 * @return
		 */
		virtual bool isUiInitialized() const;
		virtual void assignUiVariables();

		virtual void retranslate()=0;

		template<typename T, typename UiClass>
		void setupParent(T* widget, UiClass** ui)
		{
			if(isUiInitialized()){
				return;
			}

			*ui = new UiClass();
			(*ui)->setupUi(widget);

			assignUiVariables();
			finalizeInitialization();
		}

		void closeEvent(QCloseEvent* e) override;
		void showEvent(QShowEvent* e) override;


	public:

		/**
		 * @brief needed by the player ui, final
		 * @return action of plugin
		 */
		virtual QAction*	pluginAction() const final;


		/**
		 * @brief must be overwritten
		 * @return the NOT translated name of the plugin
		 */
		virtual QString		name() const=0;

		/**
		 * @brief must be overwritten
		 * @return the translated name of the plugin
		 */
		virtual QString		displayName() const=0;


		/**
		 * @brief indicates if title bar is shown or not
		 */
		virtual bool		hasTitle() const;

		/**
		 * @brief indicates if the widget has a loading bar. If yes, there will be reserved
		 * some extra space at the bottom of the widget
		 * @return
		 */
		virtual bool		hasLoadingBar() const;
	};
}

Q_DECLARE_INTERFACE(PlayerPlugin::Base, "com.sayonara-player.playerplugin")

#endif // PLAYERPLUGIN_H
