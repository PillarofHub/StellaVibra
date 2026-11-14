#pragma once
#include <QtWidgets>
#include <functional>
namespace sv {
class EffectSortModel : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
protected:
    bool lessThan(const QModelIndex& left,const QModelIndex& right) const override {
        const auto* files=qobject_cast<const QFileSystemModel*>(sourceModel());
        if(files && files->isDir(left)!=files->isDir(right))return files->isDir(left);
        return QSortFilterProxyModel::lessThan(left,right);
    }
};
// Use the native item layout: indentation, checkbox, icon, then name.
class EffectCheckDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QRect checkRect(const QStyleOptionViewItem& option,const QModelIndex& index) const {
        if(!index.data(Qt::CheckStateRole).isValid())return {};
        auto opt=option;initStyleOption(&opt,index);
        auto* style=opt.widget?opt.widget->style():QApplication::style();
        return style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator,&opt,opt.widget);
    }
};
class EffectTree : public QTreeView {
public:
    using QTreeView::QTreeView;
    std::function<bool(QString,QString)> canMove;
    std::function<bool(QString,QString)> move;
protected:
    void startDrag(Qt::DropActions) override {
        if(!currentIndex().isValid())return;
        const auto path=currentIndex().data(QFileSystemModel::FilePathRole).toString();
        if(!canMove || !canMove(path,QString()))return;
        QDrag drag(this);auto* mime=new QMimeData;
        mime->setData("application/x-stella-effect",path.toUtf8());drag.setMimeData(mime);
        drag.exec(Qt::MoveAction,Qt::MoveAction);
    }
    void dragEnterEvent(QDragEnterEvent* event) override {
        if(event->source()==this && event->mimeData()->hasFormat("application/x-stella-effect")){event->setDropAction(Qt::MoveAction);event->accept();}
        else event->ignore();
    }
    void dragMoveEvent(QDragMoveEvent* event) override {
        const auto index=indexAt(event->position().toPoint());
        const auto source=QString::fromUtf8(event->mimeData()->data("application/x-stella-effect"));
        if(event->source()==this && index.isValid() && canMove && canMove(source,index.data(QFileSystemModel::FilePathRole).toString())){
            QTreeView::dragMoveEvent(event);
            event->setDropAction(Qt::MoveAction);event->accept();
        }else event->ignore();
    }
    void dropEvent(QDropEvent* event) override {
        const auto index=indexAt(event->position().toPoint());
        const auto source=QString::fromUtf8(event->mimeData()->data("application/x-stella-effect"));
        if(event->source()==this && index.isValid() && canMove && canMove(source,index.data(QFileSystemModel::FilePathRole).toString()) && move && move(source,index.data(QFileSystemModel::FilePathRole).toString())){event->setDropAction(Qt::MoveAction);event->accept();}
        else event->ignore();
    }
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        auto index=indexAt(event->position().toPoint());QStyleOptionViewItem opt;initViewItemOption(&opt);opt.rect=visualRect(index);
        auto* delegate=static_cast<EffectCheckDelegate*>(itemDelegate());
        if(index.isValid() && delegate->checkRect(opt,index).contains(event->position().toPoint())){event->accept();return;}
        QTreeView::mouseDoubleClickEvent(event);
    }
};
class EffectFiles : public QFileSystemModel {
public:
    using QFileSystemModel::QFileSystemModel;
    QMap<QString,bool> checks;
    QMap<QString,QString> errors;
    QString effectsRoot;
    bool effectGroup(const QString& path) const {return !effectsRoot.isEmpty() && path.startsWith(effectsRoot+"/") && QFileInfo(path).isDir();}
    std::function<void(QString,bool)> toggle;
    QVariant data(const QModelIndex& index,int role=Qt::DisplayRole) const override {
        auto path=filePath(index).toLower();
        if(index.column()==0) {
            if(role==Qt::CheckStateRole){if(checks.contains(path))return checks[path]?Qt::Checked:Qt::Unchecked;
                if(effectGroup(path)){int on=0,off=0;for(auto it=checks.begin();it!=checks.end();++it)if(it.key().startsWith(path+"/")){if(it.value())++on;else ++off;}return on?(off?Qt::PartiallyChecked:Qt::Checked):Qt::Unchecked;}}
        }
        if(errors.contains(path)){if(role==Qt::ToolTipRole)return errors[path];if(role==Qt::ForegroundRole)return QColor("#d05050");}
        return QFileSystemModel::data(index,role);
    }
    Qt::ItemFlags flags(const QModelIndex& index) const override {
        auto path=filePath(index).toLower();auto f=QFileSystemModel::flags(index);if(index.column()==0 && (checks.contains(path)||effectGroup(path)))f|=Qt::ItemIsUserCheckable;
        if(!effectsRoot.isEmpty() && (path==effectsRoot || path.startsWith(effectsRoot+"/"))){
            if(QFileInfo(path).isDir())f|=Qt::ItemIsDropEnabled;
            if(path!=effectsRoot && (QFileInfo(path).isDir() || path.endsWith(".lua")))f|=Qt::ItemIsDragEnabled;
        }
        return f;
    }
    bool setData(const QModelIndex& index,const QVariant& value,int role) override {
        auto path=filePath(index).toLower();if(role==Qt::CheckStateRole && index.column()==0 && (checks.contains(path)||effectGroup(path))){if(toggle)toggle(filePath(index),value.toInt()==Qt::Checked);return true;}
        return QFileSystemModel::setData(index,value,role);
    }
    void refresh(const QString& path){auto i=index(path);while(i.isValid()){emit dataChanged(i,i,{Qt::CheckStateRole,Qt::ToolTipRole,Qt::ForegroundRole});i=i.parent();}}
};
}
