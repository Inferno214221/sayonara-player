#include "GUI_FileExpressionDialog.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSpacerItem>

struct GUI_FileExpressionDialog::Private
{
	QLineEdit* le_expression=nullptr;
	QPushButton* btn_artist=nullptr;
	QPushButton* btn_album=nullptr;
	QPushButton* btn_tracknum=nullptr;
	QPushButton* btn_title=nullptr;
	QPushButton* btn_year=nullptr;
	QPushButton* btn_bitrate=nullptr;

	QPushButton* btn_cancel=nullptr;
	QPushButton* btn_ok=nullptr;
};

static QPushButton* init_button(const QString& value, Lang::Term term, QWidget* parent)
{
	auto* btn = new QPushButton(parent);

	btn->setText(Lang::get(term));
	btn->setProperty("value", value);
	btn->setProperty("langterm", int(term));

	return btn;
}

static bool is_valid(const QString& expression)
{
	if(expression.trimmed().isEmpty())
	{
		return false;
	}

	QStringList invalid
	{
		"/", "\\", "?", "*", "{", "}", "[", "]", "#", "\"", "|"
	};

	for(const QString& s : invalid)
	{
		if(expression.contains(s))
		{
			return false;
		}
	}

	QString replaced(expression);
	replaced.replace("<title>", "Hallo");
	replaced.replace("<artist>", "Hallo");
	replaced.replace("<album>", "Hallo");
	replaced.replace("<tracknum>", "Hallo");
	replaced.replace("<bitrate>", "Hallo");
	replaced.replace("<year>", "Hallo");

	if(replaced == expression) {
		return false;
	}

	if(replaced.contains("<") || replaced.contains(">")){
		return false;
	}

	return true;
}

GUI_FileExpressionDialog::GUI_FileExpressionDialog(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	if(this->layout()){
		this->setLayout(nullptr);
	}

	auto* vbox_layout = new QVBoxLayout(this);
	this->setLayout(vbox_layout);

	{ // line edit
		m->le_expression = new QLineEdit(this);
		vbox_layout->addWidget(m->le_expression);
		connect(m->le_expression, &QLineEdit::textChanged, this, &GUI_FileExpressionDialog::text_changed);
	}

	{ // init buttons
		auto* hbox_layout_buttons = new QHBoxLayout(this);
		m->btn_artist = init_button("<artist>", Lang::Artist, this);
		m->btn_album = init_button("<album>", Lang::Album, this);
		m->btn_tracknum = init_button("<tracknum>", Lang::TrackNo, this);
		m->btn_title = init_button("<title>", Lang::Title, this);
		m->btn_year = init_button("<year>", Lang::Year, this);
		m->btn_bitrate = init_button("<bitrate>", Lang::Bitrate, this);

		QList<QPushButton*> buttons
		{
			m->btn_tracknum, m->btn_artist, m->btn_album,
			m->btn_title, m->btn_year, m->btn_bitrate
		};

		for(auto* btn : buttons)
		{
			hbox_layout_buttons->addWidget(btn);
			connect(btn, &QPushButton::clicked, this, &GUI_FileExpressionDialog::btn_clicked);
		}

		vbox_layout->addLayout(hbox_layout_buttons);
	}

	{ // ok cancel
		auto* hbox_layout_okcancel = new QHBoxLayout(this);
		m->btn_ok = new QPushButton(Lang::get(Lang::OK), this);
		m->btn_cancel = new QPushButton(Lang::get(Lang::Cancel), this);

		hbox_layout_okcancel->addSpacerItem(
			new QSpacerItem(100, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum)
		);

		hbox_layout_okcancel->addWidget(m->btn_cancel);
		hbox_layout_okcancel->addWidget(m->btn_ok);

		auto* line = new QFrame(this);
		line->setFrameShape(QFrame::Shape::HLine);

		vbox_layout->addWidget(line);
		vbox_layout->addLayout(hbox_layout_okcancel);

		connect(m->btn_ok, &QPushButton::clicked, this, [=]()
		{
			SetSetting(Set::Dir_TagToFilenameExpression, m->le_expression->text());
			this->accept();
		});

		connect(m->btn_cancel, &QPushButton::clicked, this, &Gui::Dialog::reject);
	}
}

GUI_FileExpressionDialog::~GUI_FileExpressionDialog() = default;

QString GUI_FileExpressionDialog::expression() const
{
	return m->le_expression->text();
}

void GUI_FileExpressionDialog::showEvent(QShowEvent* event)
{
	Gui::Dialog::showEvent(event);

	m->le_expression->setText(GetSetting(Set::Dir_TagToFilenameExpression));

	if(m->le_expression->text().isEmpty())
	{
		QString text = QString("%1. %2")
			.arg(m->btn_tracknum->property("value").toString())
			.arg(m->btn_title->property("value").toString())
		;

		m->le_expression->setText(text);
	}
}

void GUI_FileExpressionDialog::language_changed()
{
	QList<QPushButton*> buttons
	{
		m->btn_tracknum, m->btn_artist, m->btn_album,
		m->btn_title, m->btn_year, m->btn_bitrate
	};

	for(auto* btn : buttons)
	{
		Lang::Term term = Lang::Term(btn->property("langterm").toInt());
		btn->setText(Lang::get(term));
	}
}

void GUI_FileExpressionDialog::btn_clicked()
{
	auto* button = static_cast<QPushButton*>(sender());

	QString text = m->le_expression->text();
	QString button_text = button->property("value").toString();
	int cursor = m->le_expression->cursorPosition();
	if(cursor < text.size() - 1 && cursor >= 0)
	{
		text.insert(cursor, button_text);
		cursor += button_text.size();
	}

	else
	{
		text.append(button_text);
		cursor = text.size();
	}

	m->le_expression->setText(text);
	m->le_expression->setCursorPosition(cursor);
	m->le_expression->setFocus();
}

void GUI_FileExpressionDialog::text_changed(const QString& text)
{
	bool valid = is_valid(text);

	if(!valid) {
		m->le_expression->setStyleSheet("color: red;");
	} else {
		m->le_expression->setStyleSheet("");
	}

	m->btn_ok->setEnabled(valid);
}
