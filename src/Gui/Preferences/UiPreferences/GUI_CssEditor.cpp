#include "GUI_CssEditor.h"
#include "Gui/Preferences/ui_GUI_CssEditor.h"
#include "Utils/Settings/Settings.h"

#include "Utils/FileUtils.h"
#include "Utils/Style.h"
#include "Utils/Utils.h"
#include "Utils/StandardPaths.h"

#include <QPushButton>

struct GUI_CssEditor::Private
{
	bool dark;
	QString original;

	Private() :
		dark(true)
	{}

	QString filename() const
	{
		if(this->dark) {
			return Util::xdgConfigPath("dark.css");
		}

		else {
			return Util::xdgConfigPath("standard.css");
		}
	}
};

GUI_CssEditor::GUI_CssEditor(QWidget* parent) :
	Gui::Dialog(parent),
	ui(new Ui::GUI_CssEditor)
{
	m = Pimpl::make<Private>();

	ui->setupUi(this);

	connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QWidget::close);
	connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &GUI_CssEditor::applyClicked);
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &GUI_CssEditor::saveClicked);
	connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &GUI_CssEditor::undoClicked);

	connect(ui->cb_darkMode, &QCheckBox::toggled, this, &GUI_CssEditor::darkModeToggled);
}

GUI_CssEditor::~GUI_CssEditor()
{
	delete ui;
}

void GUI_CssEditor::undoClicked()
{
	ui->te_css->setPlainText(m->original);
}

void GUI_CssEditor::applyClicked()
{
	QString filename = m->filename();
	Util::File::writeFile(ui->te_css->toPlainText().toLocal8Bit(), filename);

	Set::shout<SetNoDB::Player_MetaStyle>();
}

void GUI_CssEditor::saveClicked()
{
	applyClicked();
	close();
}

void GUI_CssEditor::darkModeToggled(bool b)
{
	m->dark = b;

	Util::File::readFileIntoString(m->filename(), m->original);

	ui->te_css->setFont(QFont("monospace"));
	ui->te_css->setPlainText(m->original);
}

void GUI_CssEditor::showEvent(QShowEvent* e)
{
	bool isDark = Style::isDark();

	ui->cb_darkMode->setChecked(isDark);
	darkModeToggled(isDark);

	Gui::Dialog::showEvent(e);
}

void GUI_CssEditor::skinChanged()
{
	Gui::Dialog::skinChanged();
	ui->te_css->setFont(QFont("monospace"));
}
