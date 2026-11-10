#pragma once
#include <QtWidgets>
namespace sv {
class Editor : public QPlainTextEdit {
public:
    explicit Editor(QWidget* parent=nullptr);
    int gutterWidth() const;
    void paintGutter(QPaintEvent* event);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    QWidget* gutter;
    QCompleter* completion;
    QStringListModel* candidates;
    void completeMember();
};
}
