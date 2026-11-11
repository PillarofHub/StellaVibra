#include "editor.h"
#include "telemetry.h"
namespace sv {
class Gutter : public QWidget { Editor* editor; public: explicit Gutter(Editor* e):QWidget(e),editor(e){} void paintEvent(QPaintEvent* e) override {editor->paintGutter(e);} };
class Highlight : public QSyntaxHighlighter {
public: explicit Highlight(QTextDocument* doc):QSyntaxHighlighter(doc){}
    void highlightBlock(const QString& text) override {
        if(document()->property("json").toBool()) {
            for(auto rule:{std::pair{QString(R"(\b(true|false|null)\b)"),QColor("#569cd6")},{QString(R"(-?\b[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)"),QColor("#b5cea8")},{QString("\"(?:\\\\.|[^\"\\\\])*\""),QColor("#ce9178")}}){auto matches=QRegularExpression(rule.first).globalMatch(text);while(matches.hasNext()){auto m=matches.next();setFormat(int(m.capturedStart()),int(m.capturedLength()),rule.second);}}
            return;
        }
        for(auto rule:{std::pair{QString("\\b(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)\\b"),QColor("#569cd6")}, {QString("\\b[0-9]+(?:\\.[0-9]+)?\\b"),QColor("#b5cea8")},{QString("\"[^\"]*\"|'[^']*'"),QColor("#ce9178")},{QString("--.*$"),QColor("#6a9955")}}) {
            auto matches=QRegularExpression(rule.first).globalMatch(text); while(matches.hasNext()) {auto m=matches.next();setFormat(int(m.capturedStart()),int(m.capturedLength()),rule.second);}
        }
    }
};
Editor::Editor(QWidget* parent):QPlainTextEdit(parent),gutter(new Gutter(this)) {
    candidates=new QStringListModel(this);completion=new QCompleter(candidates,this);
    completion->setWidget(this);completion->setCompletionMode(QCompleter::PopupCompletion);
    completion->setCaseSensitivity(Qt::CaseSensitive);
    connect(completion,qOverload<const QString&>(&QCompleter::activated),this,[this](const QString& value){
        if(isReadOnly())return;
        auto cursor=textCursor();cursor.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor,completion->completionPrefix().size());
        cursor.insertText(value);setTextCursor(cursor);
    });
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); setTabStopDistance(fontMetrics().horizontalAdvance(' ')*4); new Highlight(document());
    connect(this,&QPlainTextEdit::blockCountChanged,this,[this]{setViewportMargins(gutterWidth(),0,0,0);});
    connect(this,&QPlainTextEdit::updateRequest,this,[this](const QRect& rect,int dy){if(dy)gutter->scroll(0,dy);else gutter->update(0,rect.y(),gutter->width(),rect.height());}); setViewportMargins(gutterWidth(),0,0,0);
}
void Editor::keyPressEvent(QKeyEvent* event) {
    if(completion->popup()->isVisible()) {
        if(event->key()==Qt::Key_Escape){completion->popup()->hide();event->accept();return;}
        if(event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter || event->key()==Qt::Key_Tab || event->key()==Qt::Key_Backtab){event->ignore();return;}
    }
    const bool explicitRequest=event->key()==Qt::Key_Space && event->modifiers()==Qt::ControlModifier;
    if(!explicitRequest)QPlainTextEdit::keyPressEvent(event);
    if(explicitRequest || !event->text().isEmpty() || event->key()==Qt::Key_Backspace)completeMember();
    else completion->popup()->hide();
}
void Editor::completeMember() {
    completion->popup()->hide();
    if(isReadOnly() || document()->property("json").toBool())return;
    const auto before=toPlainText().left(textCursor().position());
    // Skip strings and comments, including Lua long brackets. Never execute draft code.
    for(int i=0;i<before.size();) {
        const bool comment=before.mid(i,2)=="--";
        if(comment)i+=2;
        static const QRegularExpression longBracket("\\[(=*)\\[");
        auto bracket=longBracket.match(before,i,QRegularExpression::NormalMatch,QRegularExpression::AnchorAtOffsetMatchOption);
        if(bracket.hasMatch()) {
            auto end=before.indexOf("]"+bracket.captured(1)+"]",i+bracket.capturedLength());
            if(end<0)return;i=int(end+bracket.captured(1).size()+2);continue;
        }
        if(comment){auto end=before.indexOf('\n',i);if(end<0)return;i=int(end+1);continue;}
        if(before[i]=='\'' || before[i]=='"') {
            auto quote=before[i++];bool closed=false;
            while(i<before.size()){if(before[i]=='\\'){i+=2;continue;}if(before[i++]==quote){closed=true;break;}}
            if(!closed)return;continue;
        }
        ++i;
    }
    auto match=QRegularExpression(R"((\btelemetry(?:\.msfs|\.gears\[[^\]\n]+\])?|parameters(?:\.[A-Za-z_][A-Za-z_0-9]*)?|context|gear)([:.])([A-Za-z_][A-Za-z_0-9]*|)$)").match(before);
    if(!match.hasMatch())return;
    const auto object=match.captured(1),separator=match.captured(2);QStringList names;
    if(object=="context") {
        const bool inInit=before.lastIndexOf(":init(")>before.lastIndexOf(":update(");
        names=separator==":"?(inInit?QStringList{"parameter","output","log"}:QStringList{"log","vibrate","now","elapsed","sample_is_new","input_valid"}):QStringList{};
    }

    else if(separator==".") {
        if(object.endsWith(".msfs"))names={"paused"};
        else if(object=="gear" || object.endsWith(']'))names={"contact","animation_percent"};
        else if(object=="telemetry") {names={"sample_id","received_ms","sim_time_advanced","aircraft_key","gears","msfs"};for(const auto& field:simVariables())names<<field.key;}
        else if(object=="parameters" || object.startsWith("parameters.")) {
            // Literal registrations in init are sufficient for bundled scripts; no draft execution.
            auto source=toPlainText();auto init=source.indexOf(QRegularExpression(":init\\s*\\("));
            if(init>=0){auto end=source.indexOf(QRegularExpression("\\bfunction\\b"),init+1);auto body=source.mid(init,end<0?-1:end-init);
                auto registrations=QRegularExpression(R"rx(:parameter\s*\{[^}]*?\bname\s*=\s*(["'])([^"']+)\1)rx").globalMatch(body);
                while(registrations.hasNext()){auto registration=registrations.next();auto key=parameterAlias(registration.captured(2));if(object=="parameters" && luaIdentifier(key))names<<key;
                    if(object=="parameters."+key){auto tail=body.mid(registration.capturedStart(),body.indexOf('}',registration.capturedStart())-registration.capturedStart());if(QRegularExpression(R"(type\s*=\s*["']haptic_power["'])").match(tail).hasMatch())names={"strength","pwm","on_ms","period_ms"};}
                }
            }
        }
    }
    if(names.isEmpty())return;
    names.sort();candidates->setStringList(names);completion->setCompletionPrefix(match.captured(3));
    if(!completion->completionCount())return;
    completion->popup()->setCurrentIndex(completion->completionModel()->index(0,0));
    auto rect=cursorRect();rect.setWidth(260);completion->complete(rect);
}
int Editor::gutterWidth() const { return 14+fontMetrics().horizontalAdvance('9')*QString::number(blockCount()).size(); }
void Editor::resizeEvent(QResizeEvent* event){QPlainTextEdit::resizeEvent(event);auto r=contentsRect();gutter->setGeometry(r.left(),r.top(),gutterWidth(),r.height());}
void Editor::paintGutter(QPaintEvent* event) {
    QPainter painter(gutter); painter.fillRect(event->rect(),palette().alternateBase()); auto block=firstVisibleBlock(); int top=qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    while(block.isValid() && top<=event->rect().bottom()){int h=qRound(blockBoundingRect(block).height());if(block.isVisible() && top+h>=event->rect().top()) painter.drawText(0,top,gutter->width()-5,fontMetrics().height(),Qt::AlignRight,QString::number(block.blockNumber()+1)); top+=h;block=block.next();}
}
}
