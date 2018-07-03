#ifndef GUI_UI_PREFERENCES_H_
#define GUI_UI_PREFERENCES_H_

#include "Interfaces/PreferenceDialog/PreferenceWidget.h"

UI_FWD(GUI_UiPreferences)

class GUI_UiPreferences :
		public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_UiPreferences)
	UI_CLASS(GUI_UiPreferences)

public:
	GUI_UiPreferences(const QString& identifier);
	virtual ~GUI_UiPreferences();

	QString action_name() const override;
	bool commit() override;
	void revert() override;
	void init_ui() override;
	void retranslate_ui() override;
};

#endif
