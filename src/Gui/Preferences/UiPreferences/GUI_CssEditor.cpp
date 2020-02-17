#include "GUI_CssEditor.h"
#include "Gui/Preferences/ui_GUI_CssEditor.h"
#include "Utils/Settings/Settings.h"

#include "Utils/FileUtils.h"
#include "Utils/Style.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"

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
			return Util::sayonaraPath("dark.css");
		}

		else {
			return Util::sayonaraPath("standard.css");
		}
	}
};

GUI_CssEditor::GUI_CssEditor(QWidget* parent) :
	Gui::Dialog(parent),
	ui(new Ui::GUI_CssEditor)
{
	m = Pimpl::make<Private>();

	ui->setupUi(this);

	connect(ui->btn_cancel, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_CssEditor::applyClicked);
	connect(ui->btn_save, &QPushButton::clicked, this, &GUI_CssEditor::saveClicked);
	connect(ui->btn_undo, &QPushButton::clicked, this, &GUI_CssEditor::undoClicked);
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

	ui->te_css->setFont( QFont("monospace") );
	ui->te_css->setPlainText(m->original);
}

void GUI_CssEditor::showEvent(QShowEvent* e)
{
	bool isDark = Style::isDark();

	ui->cb_darkMode->setChecked(isDark);
	darkModeToggled(isDark);

	Gui::Dialog::showEvent(e);
}

void GUI_CssEditor::languageChanged()
{
	ui->btn_save->setText(Lang::get(Lang::Save));
	ui->btn_apply->setText(Lang::get(Lang::Apply));
	ui->btn_undo->setText(Lang::get(Lang::Undo));
	ui->btn_cancel->setText(Lang::get(Lang::Cancel));
}
