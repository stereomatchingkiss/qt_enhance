#include "download_supervisor.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QNetworkAccessManager>

namespace qte{

namespace net{

download_supervisor::download_supervisor(QObject *parent)
    : QObject(parent),
      max_download_file_(1),
      network_access_(new QNetworkAccessManager(this)),
      total_download_file_(0),
      unique_id_(0)
{

}

size_t download_supervisor::append(const QUrl &url, const QString &save_at, bool save_as_file)
{
    auto task = std::make_shared<download_task>();
    task->unique_id_ = unique_id_++;
    task->url_ = url;
    task->save_at_ = save_at;
    task->save_as_file_ = save_as_file;
    id_table_.insert({task->unique_id_, task});
    download_start(task);

    return task->unique_id_;
}

size_t download_supervisor::get_max_download_file() const
{
    return max_download_file_;
}

void download_supervisor::set_max_download_file(size_t val)
{
    max_download_file_ = val;
}

void download_supervisor::download_finished()
{
    if(total_download_file_ > 0){
        --total_download_file_;
    }
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply && reply->error() == QNetworkReply::NoError){
        auto rit = reply_table_.find(reply);
        if(rit != std::end(reply_table_)){
            auto const unique_id = rit->second->unique_id_;
            emit download_finished(unique_id, rit->second->data_);
            reply_table_.erase(rit);
            auto id_it = id_table_.find(unique_id);
            if(id_it != std::end(id_table_)){
                id_table_.erase(id_it);
            }
        }
    }
}

void download_supervisor::error_handle(QNetworkReply::NetworkError)
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto it = reply_table_.find(reply);
        if(it != std::end(reply_table_)){
            emit error(it->second->unique_id_, reply->errorString());
        }
    }
}

void download_supervisor::handle_download_progress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug()<<__func__<< " receive "<<bytesReceived;
    qDebug()<<__func__<< " total "<<bytesTotal;
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto rit = reply_table_.find(reply);
        if(rit != std::end(reply_table_)){
            emit download_progress(rit->second->unique_id_, bytesReceived,
                                   bytesTotal);
        }
    }
}

void download_supervisor::ready_read()
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        //qDebug()<<__func__<<" ready read";
        auto it = reply_table_.find(reply);
        if(it != std::end(reply_table_)){
            QByteArray data(reply->bytesAvailable(), Qt::Uninitialized);
            if(it->second->save_as_file_ && it->second->file_.isOpen()){
                reply->read(data.data(), data.size());
                it->second->file_.write(data);
            }else{
                it->second->data_ += data;
            }
        }
    }else{
        qDebug()<<__func__<< ":reply is nullptr or fail to cast from sender()";
    }
}

void download_supervisor::download_start(std::shared_ptr<download_task> &task)
{
    if(total_download_file_ < max_download_file_){
        task->file_.setFileName(save_file_name(*task));
        if(task->file_.open(QIODevice::WriteOnly)){
            ++total_download_file_;
            task->network_reply_ = network_access_->get(QNetworkRequest(task->url_));
            reply_table_.insert({task->network_reply_, task});
            connect(task->network_reply_, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(error_handle(QNetworkReply::NetworkError)));
            connect(task->network_reply_, &QNetworkReply::readyRead, this, &download_supervisor::ready_read);
            connect(task->network_reply_, SIGNAL(finished()), this, SLOT(download_finished()));
            connect(task->network_reply_, &QNetworkReply::downloadProgress, this, &download_supervisor::handle_download_progress);
        }else{
            emit error(task->unique_id_, tr("Cannot open file"));
        }
    }
}

QString download_supervisor::save_file_name(const download_supervisor::download_task &task) const
{
    QFileInfo file_info(task.url_.path());
    QString file_name = file_info.fileName();
    if(QFile::exists(task.save_at_ + "/" + file_name)){
        QString const base_name = file_info.baseName();
        QString complete_suffix = file_info.completeSuffix();
        if(complete_suffix.isEmpty()){
            complete_suffix = "txt";
        }
        QString new_file_name = base_name + "(0)." + complete_suffix;
        for(size_t i = 1; QFile::exists(task.save_at_ + "/" + new_file_name); ++i){
            new_file_name = base_name + "(" + QString::number(i) + ")." + complete_suffix;
        }

        return new_file_name;
    }

    return file_name;
}

size_t download_supervisor::download_task::get_unique_id() const
{
    return unique_id_;
}

QUrl download_supervisor::download_task::get_url() const
{
    return url_;
}

} //namespace net

} //namespace qte
