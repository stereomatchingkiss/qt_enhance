#include "download_manager.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>

namespace qte{

namespace net{

namespace{

class recycle
{
public:
    explicit recycle(QNetworkReply *value) :
        value_(value) {}
    ~recycle(){ value_->deleteLater(); }

private:
    QNetworkReply *value_;
};

template<typename Index, typename Pair>
bool create_dir(QString const &save_at, Index &index,
                Pair const &pair)
{
    QDir dir(save_at);
    if(!dir.exists()){
        if(!QDir().mkpath(save_at)){
            index.erase(pair.first);
            return false;
        }
    }else{
        qDebug()<<"dir "<<save_at<<" exist";
    }

    return true;
}

template<typename Index, typename Pair>
bool create_file(QString const &save_at, QString const &save_as,
                 Index &index, Pair const &pair)
{
    bool success = true;
    index.modify(pair.first, [&](download_info &v)
    {
        v.file_ = std::make_shared<QFile>(save_at + "/" + save_as);
        if(!v.file_->open(QIODevice::WriteOnly)){
            qDebug()<<__func__<<" cannot open file "<<save_as;
            success = false;
        }
    });

    return success;
}

}

download_manager::download_manager(QObject *obj) :
    QObject(obj),
    manager_{new QNetworkAccessManager(obj)},
    max_download_size_{4},
    total_download_files_{0},
    uuid_{0}
{

}

int_fast64_t download_manager::
append(QUrl const &url,
       QString const &save_at,
       QString const &save_as)
{
    return append_impl(url, save_at, save_as);
}

int_fast64_t download_manager::append(const QUrl &url)
{
    return append_impl(url, "", "");
}

void download_manager::clear_download_list()
{
    download_info_.clear();
}

bool download_manager::erase(int_fast64_t uuid)
{
    auto &id_set = download_info_.get<uid>();
    auto id_it = id_set.find(uuid);
    if(id_it != std::end(id_set)){
        id_set.erase(id_it);
        return true;
    }

    return false;
}

bool download_manager::start_download(int_fast64_t uuid)
{
    qDebug()<<__func__<<"start download id "<<uuid;
    auto &id_set = download_info_.get<uid>();
    auto id_it = id_set.find(uuid);
    if(id_it != std::end(id_set) && !id_it->reply_){
        qDebug()<<__func__<<" can find uuid "<<uuid;

        auto pair = std::make_pair(id_it, true);
        if(!id_it->save_at_.isEmpty()){
            bool const can_create_dir =
                    create_dir(id_it->save_at_, id_set, pair);
            bool const can_create_file =
                    create_file(id_it->save_at_, id_it->save_as_,
                                id_set, pair);
            if(!can_create_dir || !can_create_file){
                emit download_finished(id_it->uuid_, QByteArray(),
                                       tr("Cannot create file %1").arg(id_it->save_as_));
                erase(id_it->uuid_);
                return false;
            }
        }

        auto copy_it = *id_it;
        copy_it.error_.clear();
        qDebug()<<__func__<<" : "<<copy_it.url_;
        QNetworkRequest request(copy_it.url_);
        copy_it.reply_ = manager_->get(request);
        if(copy_it.reply_){
            bool const success = id_set.replace(id_it, copy_it);
            if(success){
                qDebug()<<__func__<<" can start download";
                connect_network_reply(id_it->reply_);
                return true;
            }
        }else{
            qDebug()<<__func__<<" can not start download";
        }
    }

    return false;
}

size_t download_manager::get_max_download_size() const
{
    return max_download_size_;
}

size_t download_manager::get_total_download_file() const
{
    return total_download_files_;
}

bool download_manager::restart_download(int_fast64_t uuid)
{
    auto &id_set = download_info_.get<uid>();
    auto id_it = id_set.find(uuid);

    if(id_it != std::end(id_set)){
        auto &net_set = download_info_.get<net_reply>();
        auto net_it = net_set.find(id_it->reply_);
        auto func = [=](download_info &v)->bool
        {
            connect_network_reply(v.reply_, false);
            v.reply_->abort();
            recycle rcy(v.reply_);

            QNetworkRequest request(v.url_);
            v.reply_ = manager_->get(request);
            if(v.reply_){
                v.data_.clear();
                if(v.file_.get() && v.file_->isOpen()){
                    v.file_->close();
                    v.file_->open(QIODevice::WriteOnly);
                }
                qDebug()<<"restart download id : "<<v.uuid_;
                --total_download_files_;
                connect_network_reply(v.reply_);
                return start_download(v.uuid_);
            }else{
                return false;
            }
        };
        return net_set.modify(net_it, func);
    }

    return false;
}

bool download_manager::restart_network_manager()
{
    manager_->deleteLater();
    manager_ = new QNetworkAccessManager(this);

    return true;
}

void download_manager::set_max_download_size(size_t value)
{
    max_download_size_ = value;
}

void download_manager::
connect_network_reply(QNetworkReply *reply,
                      bool is_connect)
{
    if(is_connect){
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(error(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(readyRead()),
                this, SLOT(download_ready_read()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(download_progress(qint64,qint64)));
        connect(reply, SIGNAL(finished()),
                this, SLOT(download_finished()));
        ++total_download_files_;
    }else{
        disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                   this, SLOT(error(QNetworkReply::NetworkError)));
        disconnect(reply, SIGNAL(readyRead()),
                   this, SLOT(download_ready_read()));
        disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                   this, SLOT(download_progress(qint64,qint64)));
        disconnect(reply, SIGNAL(finished()),
                   this, SLOT(download_finished()));
    }
}

int_fast64_t download_manager::append_impl(QUrl const &url,
                                           QString const &save_at,
                                           QString const &save_as)
{             
    QNetworkReply *reply = nullptr;
    auto &uid_index = download_info_.get<uid>();
    download_info info{uuid_, reply, save_at, save_as};
    qDebug()<<__func__<<" save at == "<<save_at<<
              ", save as == "<<save_as;
    qDebug()<<__func__<<"uuid == "<<uuid_;
    info.url_ = url;
    qDebug()<<__func__<<"url == "<<info.url_;
    auto pair = uid_index.insert(info);
    if(!pair.second){
        return -1;
    }

    return uuid_++;
}

void download_manager::download_finished()
{    
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        recycle rc(reply);

        auto &net_index = download_info_.get<net_reply>();
        auto it = net_index.find(reply);
        if(it != std::end(net_index)){
            //keep the item because the users may want to download it again
            net_index.modify(it, [](download_info &v)
            {
                v.reply_ = nullptr;
                if(v.file_.get() && v.file_->isOpen()){
                    v.file_->close();
                }
            });
            if(reply->isFinished() && it->error_.isEmpty()){
                emit download_finished(it->uuid_, it->data_, tr("Finished"));
            }else{
                QDir dir(it->save_at_);
                dir.remove(it->save_as_);
                emit download_finished(it->uuid_, it->data_, it->error_);
            }
            emit downloading_size_decrease(--total_download_files_);
            //net_index.erase(it);
        }
    }else{
        qDebug()<<__func__<<" : do not exist";
    }
}

void download_manager::
download_progress(qint64 bytes_received,
                  qint64 bytes_total)
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto &net_index = download_info_.get<net_reply>();
        auto it = net_index.find(reply);
        qDebug()<<__func__<< " receive "<<bytes_received;
        qDebug()<<__func__<< " total "<<bytes_total;
        if(it != std::end(net_index)){
            emit download_progress(it->uuid_, bytes_received,
                                   bytes_total);
        }
    }
}

void download_manager::download_ready_read()
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        qDebug()<<__func__<<" ready read";
        auto &net_index = download_info_.get<net_reply>();
        auto it = net_index.find(reply);
        if(it != std::end(net_index)){
            QByteArray data(reply->bytesAvailable(), Qt::Uninitialized);
            reply->read(data.data(), data.size());
            if(it->file_.get() && it->file_->isOpen()){
                it->file_->write(data);
            }else{
                qDebug()<<it->save_as_<<" do not open";
                net_index.modify(it, [&](download_info &v)
                {
                    v.data_ += data;
                });
            }

            emit download_ready_read(it->uuid_);
        }
    }else{
        qDebug()<<__func__<< " cannot cast sender to reply";
    }
}

void download_manager::error(QNetworkReply::NetworkError)
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        qDebug()<<__func__<<" : "<<reply->errorString();
        auto &reply_set = download_info_.get<net_reply>();
        auto r_it = reply_set.find(reply);
        if(r_it != std::end(reply_set)){
            reply_set.modify(r_it, [&](download_info &v)
            {
                v.error_ = reply->errorString();
            });
        }
    }
}

}

}
