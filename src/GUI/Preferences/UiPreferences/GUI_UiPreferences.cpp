
#include "GUI_UiPreferences.h"
#include "GUI_FontConfig.h"
#include "GUI_IconPreferences.h"
#include "Utils/Settings/Settings.h"

#include "GUI/Preferences/ui_GUI_UiPreferences.h"
#include "Utils/Language.h"


struct GUI_UiPreferences::Private
{
	GUI_FontConfig* font_config=nullptr;
	GUI_IconPreferences* icon_config=nullptr;
};

GUI_UiPreferences::GUI_UiPreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

 GUI_UiPreferences::~GUI_UiPreferences() {}

QString GUI_UiPreferences::action_name() const
{
	return tr("User Interface");
}

bool GUI_UiPreferences::commit()
{
	m->font_config->commit();
	m->icon_config->commit();

	_settings->set<Set::Player_ControlStyle>(ui->cb_big_cover->isChecked() ? 1 : 0);
	_settings->set<Set::Player_Style>(ui->cb_dark_mode->isChecked() ? 1 : 0);

	return true;
}

void GUI_UiPreferences::revert()
{
	m->font_config->revert();
	m->icon_config->revert();

	ui->cb_big_cover->setChecked(_settings->get<Set::Player_ControlStyle>() == 1);
	ui->cb_dark_mode->setChecked(_settings->get<Set::Player_Style>() == 1);
}

void GUI_UiPreferences::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);

	m->font_config = new GUI_FontConfig(ui->tabWidget);
	m->icon_config = new GUI_IconPreferences(ui->tabWidget);

	ui->tabWidget->addTab(m->font_config, m->font_config->action_name());
	ui->tabWidget->addTab(m->icon_config, m->icon_config->action_name());

	retranslate_ui();
	revert();
}

void GUI_UiPreferences::retranslate_ui()
{
	ui->tabWidget->setTabText(0, "General");
	if(m->font_config){
		ui->tabWidget->setTabText(1, m->font_config->action_name());
	}

	if(m->icon_config){
		ui->tabWidget->setTabText(2, m->icon_config->action_name());
	}
}
