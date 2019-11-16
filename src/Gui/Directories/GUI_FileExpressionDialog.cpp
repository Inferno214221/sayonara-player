#include "GUI_FileExpressionDialog.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include "Components/Directories/FileOperations.h"

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSpacerItem>

struct GUI_FileExpressionDialog::Private
{
	QMap<QString, Lang::Term> tag_lang_mapping;
	QLineEdit* le_expression=nullptr;

	QList<QPushButton*> buttons;
	QPushButton* btn_cancel=nullptr;
	QPushButton* btn_ok=nullptr;

	Private()
	{
		tag_lang_mapping = QMap<QString, Lang::Term>
		{
			{"<title>", Lang::Title},
			{"<album>", Lang::Album},
			{"<artist>", Lang::Artist},
			{"<year>", Lang::Year},
			{"<bitrate>", Lang::Bitrate},
			{"<tracknum>", Lang::TrackNo},
			{"<disc>", Lang::Disc}
		};
	}

	QPushButton* init_button(const QString& value, QWidget* parent)
	{
		auto* btn = new QPushButton(parent);

		if(!tag_lang_mapping.contains(value))
		{
			sp_log(Log::Warning, this) << value << " is not allowed";
			return nullptr;
		}

		Lang::Term term = tag_lang_mapping[value];
		btn->setText(Lang::get(term));
		btn->setProperty("value", value);
		btn->setProperty("langterm", int(term));

		buttons << btn;

		return btn;
	}

};

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
	const QStringList allowed_tags = FileOperations::supported_tag_replacements();
	for(const QString& tag : allowed_tags)
	{
		replaced.replace(tag, "Hallo");
	}

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
		auto* hbox_layout_buttons = new QHBoxLayout();

		const QStringList allowed_tags = FileOperations::supported_tag_replacements();
		for(const QString& tag : allowed_tags)
		{
			m->init_button(tag, this);
		}

		for(auto* btn : m->buttons)
		{
			hbox_layout_buttons->addWidget(btn);
			connect(btn, &QPushButton::clicked, this, &GUI_FileExpressionDialog::btn_clicked);
		}

		vbox_layout->addLayout(hbox_layout_buttons);
	}

	{ // ok cancel
		auto* hbox_layout_okcancel = new QHBoxLayout();
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

	{ // taborder
		this->setTabOrder(m->le_expression, m->buttons.first());
		for(int i=0; i<m->buttons.size() - 1; i++)
		{
			this->setTabOrder(m->buttons[i], m->buttons[i+1]);
		}
		this->setTabOrder(m->buttons.last(), m->btn_cancel);
		this->setTabOrder(m->btn_cancel, m->btn_ok);
		this->setTabOrder(m->btn_ok, m->le_expression);
	}

	m->btn_ok->setDefault(true);
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
		QString text = QString("<tracknum>. <title>");
		m->le_expression->setText(text);
	}
}

void GUI_FileExpressionDialog::language_changed()
{
	for(auto* btn : m->buttons)
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
