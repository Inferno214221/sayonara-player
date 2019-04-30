#include "GUI_ConfigureStreams.h"
#include "GUI/Plugins/ui_GUI_ConfigureStreams.h"
#include "Utils/Language.h"

GUI_ConfigureStreams::GUI_ConfigureStreams(const QString& type, GUI_ConfigureStreams::Mode mode, QWidget* parent) :
	Gui::Dialog(parent)
{
	ui = new Ui::GUI_ConfigureStreams();
	ui->setupUi(this);
	ui->lab_error->setVisible(false);

	connect(ui->btn_ok, &QPushButton::clicked, this, &Gui::Dialog::accept);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &Gui::Dialog::reject);

	set_mode(type, mode);
}

GUI_ConfigureStreams::~GUI_ConfigureStreams() {}

QString GUI_ConfigureStreams::url() const
{
	return ui->le_url->text();
}

QString GUI_ConfigureStreams::name() const
{
	return ui->le_name->text();
}

void GUI_ConfigureStreams::set_url(const QString& url)
{
	ui->le_url->setText(url);
}

void GUI_ConfigureStreams::set_name(const QString& name)
{
	ui->le_name->setText(name);
}

void GUI_ConfigureStreams::set_error_message(const QString& message)
{
	ui->lab_error->setText(message);
	ui->lab_error->setVisible(true);
}

void GUI_ConfigureStreams::set_mode(const QString& type, GUI_ConfigureStreams::Mode mode)
{
	QString mode_str;
	if(mode == GUI_ConfigureStreams::Edit){
		mode_str = Lang::get(Lang::Edit);
	}

	else {
		mode_str = Lang::get(Lang::New);
	}

	QString text = QString("%1: %2").arg(type).arg(mode_str);

	ui->lab_header->setText(text);
	this->setWindowTitle(text);
}

bool GUI_ConfigureStreams::was_accepted() const
{
	return (this->result() == QDialog::Accepted);
}
