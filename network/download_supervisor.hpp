#ifndef DOWNLOAD_SUPERVISOR_HPP
#define DOWNLOAD_SUPERVISOR_HPP

#include <QFile>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include <deque>
#include <map>
#include <memory>

class QNetworkAccessManager;

namespace qte{

namespace net{

/**
 * Manage multiple download files, similar to download_manager of
 * qt_enhance, but this class only depend on Qt5 and standard c++
 */
class download_supervisor : public QObject
{
    Q_OBJECT
public:
    struct download_task
    {
        friend class download_supervisor;

        size_t get_unique_id() const;
        QUrl get_url() const;

    private:
        QByteArray data_;
        QFile file_;
        QNetworkReply *network_reply_ = nullptr;
        QString save_at_;
        bool save_as_file_ = true;
        size_t unique_id_ = 0;
        QUrl url_;
    };

    explicit download_supervisor(QObject *parent = nullptr);

    size_t append(QUrl const &url, QString const &save_at, bool save_as_file = true);

    size_t get_max_download_file() const;

    void set_max_download_file(size_t val);

signals:
    void download_finished(size_t unique_id, QByteArray data);
    void download_progress(size_t unique_id, qint64 bytesReceived, qint64 bytesTotal);

    void error(size_t unique_id, QString const &error_msg);

private slots:
    void process_download_finished();
    void error_handle(QNetworkReply::NetworkError code);
    void handle_download_progress(qint64 bytesReceived, qint64 bytesTotal);
    void ready_read();

private:
    void launch_download_task(std::shared_ptr<download_task> &task);
    void download_start(std::shared_ptr<download_task> &task);
    QString save_file_name(download_task const &task) const;

    std::map<size_t, std::shared_ptr<download_task>> id_table_;
    size_t max_download_file_;
    QNetworkAccessManager *network_access_;
    std::map<QNetworkReply*, std::shared_ptr<download_task>> reply_table_;
    size_t total_download_file_;
    size_t unique_id_;
};

} //namespace net

} //namespace qte

#endif // DOWNLOAD_MANAGER_HPP
