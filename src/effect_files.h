#pragma once
#include <QtWidgets>
#include <functional>
namespace sv {
class EffectFiles : public QFileSystemModel {
public:
    using QFileSystemModel::QFileSystemModel;
    QMap<QString,bool> checks;
    QMap<QString,QString> errors;
    std::function<void(QString,bool)> toggle;
    QVariant data(const QModelIndex& index,int role=Qt::DisplayRole) const override {
        auto path=filePath(index).toLower();
        if(index.column()==0 && role==Qt::CheckStateRole && checks.contains(path))return checks[path]?Qt::Checked:Qt::Unchecked;
        if(errors.contains(path)){if(role==Qt::ToolTipRole)return errors[path];if(role==Qt::ForegroundRole)return QColor("#d05050");}
        return QFileSystemModel::data(index,role);
    }
    Qt::ItemFlags flags(const QModelIndex& index) const override {
        auto f=QFileSystemModel::flags(index);if(index.column()==0 && checks.contains(filePath(index).toLower()))f|=Qt::ItemIsUserCheckable;return f;
    }
    bool setData(const QModelIndex& index,const QVariant& value,int role) override {
        if(role==Qt::CheckStateRole && index.column()==0 && checks.contains(filePath(index).toLower())){if(toggle)toggle(filePath(index),value.toInt()==Qt::Checked);return true;}
        return QFileSystemModel::setData(index,value,role);
    }
    void refresh(const QString& path){auto i=index(path);if(i.isValid())emit dataChanged(i,i);}
};
}
