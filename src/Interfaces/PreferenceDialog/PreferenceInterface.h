/* PreferenceInterface.h */

/* Copyright (C) 2011-2016  Lucio Carreras
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



#ifndef PREFERENCEINTERFACE_H
#define PREFERENCEINTERFACE_H

#include <QAction>
#include <QByteArray>
#include <QShowEvent>
#include <QCloseEvent>
#include <QString>

#include "GUI/Helper/SayonaraWidget/SayonaraDialog.h"
#include "GUI/Helper/SayonaraWidget/SayonaraWidget.h"


/**
 * @brief The action, which is used to access the Preference.
 *
 * This action is generated by and handled by the PreferenceInterface.
 * Usually you don't get in touch with that class.
 * @ingroup Preferences
 */
class PreferenceAction : public QAction
{
	Q_OBJECT

public:
	/**
	 * @brief PreferenceAction Create QAction object, which is automatically
	 * connected to the show event of the underlying widget.
	 * @param text text of the action
	 * @param preference_interface Widget, that should appear when action is triggered
	 */
	PreferenceAction(const QString& text, QWidget* preference_interface);
};

template <typename T>
/**
 * @brief Template class for implementing preference dialogs and preference widgets
 * @ingroup Interfaces
 * @ingroup Preferences
 */
class PreferenceInterface : public T
{

private:
	PreferenceAction*	_action=nullptr;
	bool				_is_initialized;
	QByteArray			_geometry;

protected:
	/**
	 * @brief call setup_parent(this) here.\n
	 * initialize compoenents and connections here.\n
	 * After calling setup_parent(this), the preference Dialog is ready to use, language_changed() is called automatically
	 */
	virtual void init_ui()=0;

	template<typename W, typename UiClass>
	/**
	 * @brief Sets up the Preference dialog. After this method, the dialog is "ready to use"\n
	 * This method should be the first to be called when calling init_ui()
	 * @param widget should always be "this"
	 */
	void setup_parent(W* widget, UiClass** ui) {

		*ui = new UiClass();
		(*ui)->setupUi(widget);

		_is_initialized = true;

		widget->language_changed();
	}

	/**
	 * @brief automatically called when language has changed. When overriding this method.
	 * Overriding this method should look like this:
	 * void GUI_FontConfig::language_changed()\n
	 *  {\n
	 *		translate_action();\n\n
	 *
	 *		if(!is_ui_initialized()){\n
	 * 			return; \n
	 *		}\n\n
	 *
	 *		retranslateUi(this);\n
	 *		PreferenceWidgetInterface::language_changed();\n
	 *  }\n
	 */
	void language_changed() override
	{
		translate_action();

		if(!is_ui_initialized()){
			return;
		}

		QString new_name = get_action_name();
		this->setWindowTitle(new_name);
	}


	/**
	 * @brief Sets the new translated action name
	 */
	void translate_action()
	{
		QString new_name = this->get_action_name();
		this->get_action()->setText(new_name + "...");
	}



protected:

	/**
	 * @brief shows the widget and automatically calls init_ui()
	 * @param e
	 */
	void showEvent(QShowEvent* e) override
	{
		{
			if(!is_ui_initialized()){
				init_ui();
			}

			T::showEvent(e);

			if(!_geometry.isEmpty()){
				this->restoreGeometry(_geometry);
			}
		}
	}


	/**
	 * @brief closes the widget
	 * @param e
	 */
	void closeEvent(QCloseEvent* e) override
	{
		T::closeEvent(e);
	}


public:

	/**
	 * @brief Standard constructor
	 * @param parent
	 */
	PreferenceInterface(QWidget* parent=nullptr) :
		T(parent)
	{
		_is_initialized = false;
	}


	/**
	 * @brief checks if ui has already been initialized.
	 * @return false, if the widget has never been activated before, true else
	 */
	virtual bool is_ui_initialized() const final
	{
		return _is_initialized;
	}


	/**
	 * @brief get action with translated text
	 * @return
	 */
	virtual QAction* get_action() final
	{
		// action has to be initialized here, because pure
		// virtual get_action_name should not be called from ctor
		QString name = get_action_name();
		if(!_action){
			_action = new PreferenceAction(name, this);
		}

		_action->setText(name + "...");
		return _action;
	}

	/**
	 * @brief has to be implemented and should return the translated action text
	 * @return translated action name
	 */
	virtual QString get_action_name() const=0;

	/**
	 * @brief This method is called, when OK or apply is pressed. So all settings
	 * should be written there
	 */
	virtual void commit()=0;

	/**
	 * @brief This method is called, when cancel is clicked. So the gui should be
	 * re-initialized when this method is called. This method should also be called
	 * in the init_ui() method
	 */
	virtual void revert()=0;

};

#endif // PREFERENCEINTERFACE_H
