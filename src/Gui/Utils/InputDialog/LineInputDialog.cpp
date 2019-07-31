#include "LineInputDialog.h"
#include "Gui/Utils/Widgets/Completer.h"
#include "Gui/Utils/ui_LineInputDialog.h"

#include "Utils/Language/Language.h"

using Gui::LineInputDialog;
using Gui::Completer;

struct LineInputDialog::Private
{
	Gui::Completer* completer=nullptr;
	LineInputDialog::ReturnValue return_value;

	Private() : return_value(LineInputDialog::Ok) {}
};

LineInputDialog::LineInputDialog(const QString& window_title, const QString& info_text, QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::LineInputDialog();
	ui->setupUi(this);

	this->setWindowTitle(window_title);
	this->set_header_text(window_title);
	this->set_info_text(info_text);

	m->completer = new Completer(QStringList(), ui->le_input);
	ui->le_input->setCompleter(m->completer);

	connect(ui->btn_ok, &QPushButton::clicked, this, &LineInputDialog::ok_clicked);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &LineInputDialog::cancel_clicked);
}

LineInputDialog::LineInputDialog(const QString& window_title, const QString& info_text, const QString& preset, QWidget* parent) :
	LineInputDialog(window_title, info_text, parent)
{
	ui->le_input->setText(preset);
}

LineInputDialog::~LineInputDialog()
{
	delete ui; ui=nullptr;
}

void Gui::LineInputDialog::set_header_text(const QString& text)
{
	ui->lab_header->setText(text);
}

void Gui::LineInputDialog::set_info_text(const QString& text)
{
	ui->lab_info->setText(text);
}

void LineInputDialog::set_completer_text(const QStringList& lst)
{
	m->completer->set_stringlist(lst);
}

Gui::LineInputDialog::ReturnValue Gui::LineInputDialog::return_value() const
{
	return m->return_value;
}

QString LineInputDialog::text() const
{
	return ui->le_input->text();
}

void LineInputDialog::set_text(const QString& text)
{
	ui->le_input->setText(text);
}

bool LineInputDialog::was_accepted() const
{
	return (m->return_value == LineInputDialog::Ok);
}

void LineInputDialog::ok_clicked()
{
	m->return_value = LineInputDialog::Ok;
	close();
	emit accepted();
}

void LineInputDialog::cancel_clicked()
{
	close();
	emit rejected();
}

void LineInputDialog::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);

	ui->btn_ok->setText(Lang::get(Lang::OK));
	ui->btn_cancel->setText(Lang::get(Lang::Cancel));

	m->return_value = LineInputDialog::Cancelled;
}

void LineInputDialog::closeEvent(QCloseEvent* e)
{
	Gui::Dialog::closeEvent(e);
}

