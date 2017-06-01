#include "qte_utility.hpp"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

namespace qte{

namespace utils{

QString unique_file_name(QString const &save_at, QString const &file_name)
{
    QRegularExpression const re("[<>:\\\"/\\*\\?\\|\\\\]");
    auto valid_file_name = file_name;
    valid_file_name = valid_file_name.remove(re).trimmed();
    if(QFile::exists(save_at + "/" + file_name)){
        QFileInfo file_info(valid_file_name);
        QString const base_name = file_info.baseName();
        QString complete_suffix = file_info.completeSuffix();
        QString new_file_name = base_name + "(0)." + complete_suffix;
        for(size_t i = 1; QFile::exists(save_at + "/" + new_file_name); ++i){
            new_file_name = base_name + "(" + QString::number(i) + ")." + complete_suffix;
        }

        return new_file_name.trimmed();
    }

    return file_name;
}

}

}
