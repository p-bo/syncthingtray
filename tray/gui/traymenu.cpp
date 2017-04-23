#include "./traymenu.h"
#include "./traywidget.h"
#include "./trayicon.h"

#include "../../widgets/settings/settings.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QCursor>
#include <QDesktopWidget>

namespace QtGui {

TrayMenu::TrayMenu(TrayIcon *trayIcon, QWidget *parent) :
    TrayMenu(parent)
{
    m_trayIcon = trayIcon;
}

TrayMenu::TrayMenu(QWidget *parent) :
    QMenu(parent),
    m_trayIcon(nullptr)
{
    auto *menuLayout = new QHBoxLayout;
    menuLayout->setMargin(0), menuLayout->setSpacing(0);
    menuLayout->addWidget(m_trayWidget = new TrayWidget(this));
    setLayout(menuLayout);
    setPlatformMenu(nullptr);
}

QSize TrayMenu::sizeHint() const
{
    return Settings::values().appearance.trayMenuSize;
}

/*!
 * \brief Moves the specified \a innerRect at the specified \a point into the specified \a outerRect
 *        by altering \a point.
 */
void moveInside(QPoint &point, const QSize &innerRect, const QRect &outerRect)
{
    if(point.y() < outerRect.top()) {
        point.setY(outerRect.top());
    } else if(point.y() + innerRect.height() > outerRect.bottom()) {
        point.setY(outerRect.bottom() - innerRect.height());
    }
    if(point.x() < outerRect.left()) {
        point.setX(outerRect.left());
    } else if(point.x() + innerRect.width() > outerRect.right()) {
        point.setX(outerRect.right() - innerRect.width());
    }
}

void TrayMenu::showAtCursor()
{
    resize(sizeHint());
    QPoint pos(QCursor::pos());
    moveInside(pos, size(), QApplication::desktop()->availableGeometry(pos));
    popup(pos);
}

}
