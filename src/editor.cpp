#include "editor.h"

Editor::Editor(QWidget *parent)
	: QsciScintilla(parent)
{
	// identify this widget as a code editor
	// WindoweIconText is used only for windows widgets. 
	// So, it's free for us to use it here
	setWindowIconText("editor");
	//setEolMode(QsciScintilla::EolUnix);

	lexer = new QsciLexerCPP;
	this->setLexer(lexer);
	//setAutoCompletionThreshold(0);
	
	api = new QsciAPIs(lexer);
	api->prepare();

	LoadStyleSheet(this, "style_code_editor.css");
	
    setAutoCompletionThreshold(1);
    setAutoCompletionSource(QsciScintilla::AcsAll);

	connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(onCursorPositionChanged(int, int)));

	context= new QMenu(this);		
	LoadStyleSheet(context, "style_menu.css");
	actionHelpWithThis = context->addAction("Help with this code");
	connect(actionHelpWithThis, SIGNAL(triggered()), this, SLOT(HelpWithThis()));
	context->addSeparator();
	
	QAction * action = context->addAction("Undo");
	action->setShortcut(tr("Ctrl+Z"));
	connect(action, SIGNAL(triggered()), this, SLOT(MenuUndo()));
	
	action = context->addAction("Redo");
	action->setShortcut(tr("Ctrl+Y"));
	connect(action, SIGNAL(triggered()), this, SLOT(MenuRedo()));
	
	context->addSeparator();
	
	action = context->addAction("Cut");
	action->setShortcut(tr("Ctrl+X"));
	connect(action, SIGNAL(triggered()), this, SLOT(MenuCut()));

	action = context->addAction("Copy");
	action->setShortcut(tr("Ctrl+C"));
	connect(action, SIGNAL(triggered()), this, SLOT(MenuCopy()));

	action = context->addAction("Paste");
	action->setShortcut(tr("Ctrl+V"));
	connect(action, SIGNAL(triggered()), this, SLOT(MenuPaste()));

	/*action = context->addAction("Delete");
	connect(action, SIGNAL(triggered()), this, SLOT(MenuDelete()));
	*/

	context->addSeparator();
	
	action = context->addAction("Select all");
	action->setShortcut(tr("Ctrl+A"));
	connect(action, SIGNAL(triggered()), this, SLOT(MenuSelectAll()));
		
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
					this,SLOT(ShowEditorMenu(const QPoint )));	
	
	connect(this, SIGNAL(cursorPositionChanged (int , int)), this, SLOT(cursorPositionChanged (int, int)));
	
	lblCursorPosition = new QLabel(this);
	LoadStyleSheet(lblCursorPosition, "style_cursorpos.css");
	lblCursorPosition->setVisible(true);

	QVBoxLayout * vl = new QVBoxLayout(this);	
	//QVBoxLayout * vl2 = new QVBoxLayout(this);	
	
	
	int scrollbarw = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);


	QHBoxLayout * hl = new QHBoxLayout(this);		
	hl->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	hl->addWidget(lblCursorPosition);
	hl->addSpacerItem(new QSpacerItem (scrollbarw,0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	vl->addLayout(hl);
	vl->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Fixed,QSizePolicy::Expanding ));
	
	//hl->addLayout(vl2);		
	//vl2->addWidget(lblCursorPosition);
	//vl2->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding,QSizePolicy::Expanding ));	
	
	//vl->addLayout(hl);		
//	vl->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding,QSizePolicy::Expanding ));

	lastModifiedTime = QDateTime::currentDateTime();

	setEditorStyle();
}


Editor::~Editor()
{
   delete lexer;
   delete actionHelpWithThis;   
}

void Editor::SetFileName(QString filename)
{
	file = filename;
}

QString Editor::GetFileName(void)
{
	return file;
}

void Editor::setEditorStyle (void)
{
	//QFont font ("Consolas", 14, QFont::Normal, false);
	QFont font (config.editorFontName, config.editorFontSize, QFont::Normal, false);

	lblCursorPosition->setFont(font);
	QFontMetrics fontMetrics(font);

    //QColor backColor = QColor(22, 30, 32);
    QColor backColor = QColor(config.editorColorName);

    //this->setColor(backColor);
	
	// margin
	setMarginLineNumbers(100, true);
	setMarginsFont(font);
	setMarginsBackgroundColor(backColor);//QColor(24,32,34));//QColor(20,28,30));
	setMarginsForegroundColor(QColor(42,50,52));
	setMarginWidth(0, fontMetrics.width("00000") + 6);	
	
	// General editor things
	setTabWidth(4);	
	ensureCursorVisible();
	setIndicatorForegroundColor (Qt::red, 1);	

	// compile error/warning mark
	markerDefine(QsciScintilla::RightArrow, 1); 
	setMarkerBackgroundColor(Qt::red, 1);	
	markerDefine(QsciScintilla::Circle, 0);
	setMarkerBackgroundColor(Qt::red, 0);
	setMarkerForegroundColor(Qt::white, 0);

	// caret
	setCaretLineVisible(true);
	setCaretWidth(2);
	setCaretForegroundColor(QColor(220, 200, 200));
	setCaretLineBackgroundColor(QColor(24, 32, 34));

	//SendScintilla (SC_TECHNOLOGY_DIRECTWRITE, 1);
	//SendScintilla (SCI_SETBUFFEREDDRAW, 1);
	//SendScintilla (SCI_SETTWOPHASEDRAW, 1);
	//SendScintilla (SCI_SETFONTQUALITY,  SC_EFF_QUALITY_NON_ANTIALIASED);//SC_EFF_QUALITY_DEFAULT);	

	// matched and unmatched braces coloring
	setBraceMatching(QsciScintilla::SloppyBraceMatch);
	setMatchedBraceBackgroundColor(backColor);
	setMatchedBraceForegroundColor(QColor(220, 220, 50));
	setUnmatchedBraceBackgroundColor(QColor(0,0,0));
	setUnmatchedBraceForegroundColor(QColor(255, 0, 0));

	// selection
	setSelectionForegroundColor(QColor(150, 150, 150));
	setSelectionBackgroundColor(QColor(52, 60, 62));
	
	// maim cpp styles
	setLexerStyle(-1, QColor (120, 120, 120),backColor);	
	//setLexerStyle(-1, Qt::red, Qt::blue);	
	//setLexerStyle(QsciLexerCPP::Default, Qt::blue, Qt::yellow);
	setLexerStyle(QsciLexerCPP::Default, QColor (0, 0, 120),backColor);
	//setLexerStyle(QsciLexerCPP::callSTYLE_CALLTIP, QColor (255,0,0), QColor(255,255,255));
	
	setLexerStyle(QsciLexerCPP::Comment, QColor (80, 80, 80),backColor);
	setLexerStyle(QsciLexerCPP::CommentDoc, QColor (80, 80, 80),backColor);		
	setLexerStyle(QsciLexerCPP::CommentLine, QColor (80, 80, 80),backColor);
	setLexerStyle(QsciLexerCPP::Number, QColor (0, 120, 0),backColor);
	setLexerStyle(QsciLexerCPP::Keyword, QColor (140, 100, 0),backColor);
	setLexerStyle(QsciLexerCPP::DoubleQuotedString, QColor (0, 120, 120),backColor);
	setLexerStyle(QsciLexerCPP::SingleQuotedString, QColor (0, 120, 120),backColor);
	setLexerStyle(QsciLexerCPP::PreProcessor, QColor (110, 80, 140),backColor);
	setLexerStyle(QsciLexerCPP::Operator, QColor (140, 140, 50),backColor);
	setLexerStyle(QsciLexerCPP::Identifier, QColor (140, 140, 140),backColor);
	setLexerStyle(QsciLexerCPP::UnclosedString, QColor (0, 255, 255),backColor);
	
	// secondary styles
	setLexerStyle(QsciLexerCPP::UUID, QColor (150, 150, 150),backColor);	
	setLexerStyle(QsciLexerCPP::VerbatimString, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::Regex, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::CommentLineDoc, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::KeywordSet2, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::CommentDocKeyword, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::CommentDocKeywordError, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::GlobalClass, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::RawString, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::TripleQuotedVerbatimString, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::HashQuotedString, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::PreProcessorComment, QColor (150, 150, 150),backColor);
	setLexerStyle(QsciLexerCPP::PreProcessorCommentLineDoc, QColor (150, 150, 150),backColor); 	
	
}

void Editor::setLexerStyle(int style, QColor foreground, QColor background, bool bold, bool italic, bool underline)
{
	//QFont font("Consolas", -1, QFont::Normal, false);
	QFont font (config.editorFontName, config.editorFontSize, QFont::Normal, false);
	lexer->setFont(font, style);
	lexer->setColor(foreground, style);
	lexer->setPaper(background, style);		
}


/*void Editor::onCursorPositionChanged(int line, int index)
{

}*/

void Editor::mousePressEvent ( QMouseEvent * event )
{
	QsciScintilla::mousePressEvent(event);
	long res = this->SendScintilla(SCI_GETCARETLINEBACK, 0);
	if (res != 0x00222018) {
		setCaretLineBackgroundColor(QColor(24, 32, 34));
	}
	markerDeleteAll();
}


void Editor::focusInEvent ( QFocusEvent * event )
{
	QsciScintilla::focusInEvent(event);

	// Check if file was modified
	QFileInfo fInfo(file);
	if (fInfo.exists()) {
		QDateTime fileTime = fInfo.lastModified();
        QString s1 = lastModifiedTime.toString();
		QString s2 = fileTime.toString();
        if (lastModifiedTime < fileTime) {
			lastModifiedTime = QDateTime::currentDateTime();
			QString name = QFileInfo(file).fileName().toLower();
			if (name != "mariamole_auto_generated.h") {
				if (GetUserConfirmation("File was modified outside editor. Do you want to reload it?\n" +file) == false)  {
					return;
				}
			}
		
			QFile inFile(file);
			if (!inFile.open(QFile::ReadOnly)) {
				ErrorMessage(tr("Cannot read file %1:\n%2.").arg(file).arg(inFile.errorString()));			
				return;
			}
			QTextStream in(&inFile);
			QApplication::setOverrideCursor(Qt::WaitCursor);
			QString txt(in.readAll());
			setText(txt);				
			setModified(false);
			QApplication::restoreOverrideCursor();
        }
	} // file exists
}


void Editor::CodeBeautifier(void)
{
	QString code = this->text();
	QString output = "";
	QStringList lines = code.split("\n");
	QString tab = "";
	int i = 0;
	bool inc;

	while ( i < lines.count() ) {
		inc = true;
		QString trim = lines[i].trimmed();

		int pos = trim.indexOf(" ");
		bool IfAdd_DoItOnSametLine = false;
		if (pos > 0) {
			QString firstWord = trim.left(pos).trimmed().toUpper();
			IfAdd_DoItOnSametLine |= (firstWord == "INT");
			IfAdd_DoItOnSametLine |= (firstWord == "VOID");
			IfAdd_DoItOnSametLine |= (firstWord == "FLOAT");
			IfAdd_DoItOnSametLine |= (firstWord == "DOUBLE");
			IfAdd_DoItOnSametLine |= (firstWord == "UNSIGNED");
			IfAdd_DoItOnSametLine |= (firstWord == "LONG");
			IfAdd_DoItOnSametLine |= (firstWord == "SHORT");
			IfAdd_DoItOnSametLine |= (firstWord == "CHAR");
			IfAdd_DoItOnSametLine |= (firstWord == "CHAR*");
			IfAdd_DoItOnSametLine |= (firstWord == "BYTE");
			IfAdd_DoItOnSametLine |= (firstWord == "BOOL");
		}

		// fix the } at the end of a block
		if (trim.indexOf("}") >= 0) {
			if (tab.length() > 0) {
				tab.remove(0, 1);
			}
		}	
		
		// fix line tabs/blank spaces
		lines[i] = tab + trim;
		//output += tab + trim + "\n";
		
		// fix the { at the beginning of a block
		if (trim.indexOf("{") >= 0) {
			if (trim == "{") {
				if (i > 0) {					
					if (IfAdd_DoItOnSametLine) {
						// move "{" of the end of the previous
						lines[i-1] += " " + trim;
						lines.erase(lines.begin() + i);
					} else {
						// leave "{" of the current line
						lines[i] = trim; 
					}
					inc = false;
				}
			}
			tab += "\t";
		}
		
		if (inc) {
			++i;
		}
	}
	code = lines.join("\n");
	setText(code);
}


bool Editor::Open(QString filename)
{
	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) {
		ErrorMessage(tr("Cannot read file %1:\n%2.").arg(filename).arg(file.errorString()));			
		return false;
	}
	QTextStream in(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);

	QString txt(in.readAll());
	setText(txt);
	SetFileName(filename);
	setModified(false);
    lastModifiedTime = QFileInfo(filename).lastModified();
	
	return true;
}

bool Editor::Save(void)
{
	// text open in editor is always associated to a filename
	// so wer don't need to ask for filenames before
	// saving those files

	if (isModified() == false) {
		return true;
	}

	QFile qfile(file);
	qfile.open(QIODevice::WriteOnly);// | QIODevice::Text);
	if (qfile.error() != 0) {		
		ErrorMessage(tr("Error while saving file %1:\n%2.")
								.arg(file)
								.arg(qfile.errorString()));
		return false;
	} else {    		
		QTextStream out(&qfile); // we will serialize the data into the file
		QString text = this->text();		
		out << text; 		
		qfile.close();

		setModified(false);
        lastModifiedTime = QFileInfo(file).lastModified();
		return true;
	}
}


void Editor::Configure(void)
{
	setEditorStyle();
}

void Editor::ShowEditorMenu(const QPoint point)
{
	if (GetRefenceForWordUnderCursor() != "") {
		int line, index;
		getCursorPosition(&line, &index);
		QString word = wordAtLineIndex(line, index);// wordAtPoint(mapToGlobal(QCursor::pos()));
		actionHelpWithThis->setText("Help with this code: " + word);
		actionHelpWithThis->setVisible(true);
	} else {
		actionHelpWithThis->setText("Help with this code");
		actionHelpWithThis->setVisible(false);
	}

	context->popup(mapToGlobal(point));
}

QString Editor::GetRefenceForWordUnderCursor(void)
{
	int line, index;
	getCursorPosition(&line, &index);
	QString word = wordAtLineIndex(line, index);// wordAtPoint(mapToGlobal(QCursor::pos()));
	if (word != "") {
		QString fileName = (word + ".html").toUpper();
		QDir dir(config.arduinoInstall + "/reference");
		if (dir.entryList().contains(fileName, Qt::CaseInsensitive)) {
			for (int i=0; i < dir.entryList().count(); i++) {
				if (dir.entryList().at(i).toUpper() == fileName) {
					return config.arduinoInstall + "/reference/" + dir.entryList().at(i);
				}
			}
		}
	}

	return "";
}
void Editor::HelpWithThis(void)
{
	QString file = GetRefenceForWordUnderCursor();
	if (file != "") {
		QDesktopServices::openUrl(QUrl(file));
	}
//	QString word = wordAtPosition (();
}

void Editor::MenuUndo(void)
{
	undo();
}

void Editor::MenuRedo(void)
{
	redo();
}

void Editor::MenuCut(void)
{
	cut();
}

void Editor::MenuCopy(void)
{
	copy();
}

void Editor::MenuPaste(void)
{
	paste();
}

/*void Editor::MenuDelete(void)
{
	this->era
}*/

void Editor::MenuSelectAll(void)
{
	selectAll();
}

void Editor::cursorPositionChanged (int line, int index) 
{
	lblCursorPosition->setText("lin:" + QString::number(line) + ", col:" + QString::number(index));
}
