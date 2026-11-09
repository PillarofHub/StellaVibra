#include "editor.h"
namespace sv {
class Gutter : public QWidget { Editor* editor; public: explicit Gutter(Editor* e):QWidget(e),editor(e){} void paintEvent(QPaintEvent* e) override {editor->paintGutter(e);} };
class Highlight : public QSyntaxHighlighter {
public: explicit Highlight(QTextDocument* doc):QSyntaxHighlighter(doc){}
    void highlightBlock(const QString& text) override {
        for(auto rule:{std::pair{QString("\\b(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)\\b"),QColor("#569cd6")}, {QString("\\b[0-9]+(?:\\.[0-9]+)?\\b"),QColor("#b5cea8")},{QString("\"[^\"]*\"|'[^']*'"),QColor("#ce9178")},{QString("--.*$"),QColor("#6a9955")}}) {
            auto matches=QRegularExpression(rule.first).globalMatch(text); while(matches.hasNext()) {auto m=matches.next();setFormat(int(m.capturedStart()),int(m.capturedLength()),rule.second);}
        }
    }
};
Editor::Editor(QWidget* parent):QPlainTextEdit(parent),gutter(new Gutter(this)) {
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); setTabStopDistance(fontMetrics().horizontalAdvance(' ')*4); new Highlight(document());
    connect(this,&QPlainTextEdit::blockCountChanged,this,[this]{setViewportMargins(gutterWidth(),0,0,0);});
    connect(this,&QPlainTextEdit::updateRequest,this,[this](const QRect& rect,int dy){if(dy)gutter->scroll(0,dy);else gutter->update(0,rect.y(),gutter->width(),rect.height());}); setViewportMargins(gutterWidth(),0,0,0);
}
int Editor::gutterWidth() const { return 14+fontMetrics().horizontalAdvance('9')*QString::number(blockCount()).size(); }
void Editor::resizeEvent(QResizeEvent* event){QPlainTextEdit::resizeEvent(event);auto r=contentsRect();gutter->setGeometry(r.left(),r.top(),gutterWidth(),r.height());}
void Editor::paintGutter(QPaintEvent* event) {
    QPainter painter(gutter); painter.fillRect(event->rect(),palette().alternateBase()); auto block=firstVisibleBlock(); int top=qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    while(block.isValid() && top<=event->rect().bottom()){int h=qRound(blockBoundingRect(block).height());if(block.isVisible() && top+h>=event->rect().top()) painter.drawText(0,top,gutter->width()-5,fontMetrics().height(),Qt::AlignRight,QString::number(block.blockNumber()+1)); top+=h;block=block.next();}
}
}
