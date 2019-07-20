#ifndef DOWNLOADVIEW_H
#define DOWNLOADVIEW_H

#include <QTreeView>

namespace Data {
struct SyncthingItemDownloadProgress;
struct SyncthingDir;
} // namespace Data

namespace QtGui {

class DownloadView : public QTreeView {
    Q_OBJECT
public:
    DownloadView(QWidget *parent = nullptr);

Q_SIGNALS:
    void openDir(const Data::SyncthingDir &dir);
    void openItemDir(const Data::SyncthingItemDownloadProgress &dir);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private Q_SLOTS:
    void showContextMenu(const QPoint &position);
    void copySelectedItem();
};
} // namespace QtGui

#endif // DOWNLOADVIEW_H
