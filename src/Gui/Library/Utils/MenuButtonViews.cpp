#include "MenuButtonViews.h"

#include "Gui/Utils/Icons.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Gui/Utils/PreferenceAction.h"

#include <QMenu>
#include <QAction>

using Library::MenuButtonViews;

struct MenuButtonViews::Private
{
	QMenu* menu=nullptr;
	QAction* tableViewAction=nullptr;
	QAction* coverViewAction=nullptr;
	QAction* directoryViewAction=nullptr;

	Gui::PreferenceAction* preferenceAction=nullptr;

	Private(QWidget* parent)
	{
		menu = new QMenu(parent);

		tableViewAction = new QAction();
		tableViewAction->setCheckable(true);

		coverViewAction = new QAction();
		coverViewAction->setCheckable(true);

		directoryViewAction = new QAction();
		directoryViewAction->setCheckable(true);

		preferenceAction = new Gui::ShortcutPreferenceAction(menu);

		auto* actionGroup = new QActionGroup(parent);
		actionGroup->addAction(tableViewAction);
		actionGroup->addAction(coverViewAction);
		actionGroup->addAction(directoryViewAction);
	}
};

MenuButtonViews::MenuButtonViews(QWidget* parent) :
	Gui::MenuToolButton(parent)
{
	m = Pimpl::make<Private>(this);

	this->registerAction(m->tableViewAction);
	this->registerAction(m->coverViewAction);
	this->registerAction(m->directoryViewAction);
	this->addAction(m->menu->addSeparator());

	this->registerPreferenceAction(m->preferenceAction);

	viewTypeChanged();

	connect(m->tableViewAction, &QAction::triggered, this, &MenuButtonViews::actionTriggered);
	connect(m->coverViewAction, &QAction::triggered, this, &MenuButtonViews::actionTriggered);
	connect(m->directoryViewAction, &QAction::triggered, this, &MenuButtonViews::actionTriggered);

	ListenSetting(Set::Lib_ViewType, MenuButtonViews::viewTypeChanged);
}

void MenuButtonViews::actionTriggered(bool b)
{
	Q_UNUSED(b)

	if(m->tableViewAction->isChecked()) {
		SetSetting(Set::Lib_ViewType, Library::ViewType::Standard);
	}

	else if(m->coverViewAction->isChecked()) {
		SetSetting(Set::Lib_ViewType, Library::ViewType::CoverView);
	}

	else if(m->directoryViewAction->isChecked()) {
		SetSetting(Set::Lib_ViewType, Library::ViewType::FileView);
	}
}

void MenuButtonViews::viewTypeChanged()
{
	Library::ViewType viewType = GetSetting(Set::Lib_ViewType);

	if(viewType == Library::ViewType::Standard)
	{
		m->tableViewAction->setChecked(true);
	}

	else if(viewType == Library::ViewType::CoverView)
	{
		m->coverViewAction->setChecked(true);
	}

	else if(viewType == Library::ViewType::FileView)
	{
		m->directoryViewAction->setChecked(true);
	}
}

MenuButtonViews::~MenuButtonViews() = default;

static void checkIcon(QPushButton* btn)
{
	QIcon icon = Gui::Icons::icon(Gui::Icons::Grid);
	if(icon.isNull())
	{
		btn->setText(QString::fromUtf8("≡"));
		btn->setIcon(QIcon());
	}

	else
	{
		btn->setText("");
		btn->setIcon(icon);
	}
}

void MenuButtonViews::languageChanged()
{
	Gui::MenuToolButton::languageChanged();

	m->tableViewAction->setText(Lang::get(Lang::Default));
	m->coverViewAction->setText(Lang::get(Lang::Covers));
	m->directoryViewAction->setText(Lang::get(Lang::Directories));

	checkIcon(this);
}

void MenuButtonViews::skinChanged()
{
	Gui::MenuToolButton::skinChanged();

	checkIcon(this);
}

