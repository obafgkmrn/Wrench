/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2013-2014  Patrick von Reth <vonreth@kde.org>


    SnoreNotify is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SnoreNotify is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SnoreNotify.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../snore.h"
#include "snorebackend.h"
#include "snorefrontend.h"

#include <QTimer>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>

namespace Snore{

SnorePlugin::SnorePlugin ( const QString &name ) :
    m_name ( name ),
    m_initialized(false)
{}

SnorePlugin::~SnorePlugin()
{
    qDebug() << m_name << this << "deleted";
    deinitialize();
}

bool SnorePlugin::initialize( SnoreCore *snore )
{
    if(m_initialized)
    {
        qFatal("Something went wrong, plugin %s is already initialized",this->name().toLatin1().constData());
        return false;
    }
    qDebug() << "Initialize" << m_name << this << snore;
    this->m_snore = snore;
    m_initialized = true;
    return true;
}

bool SnorePlugin::isInitialized()
{
    return m_initialized;
}

SnoreCore* SnorePlugin::snore()
{
    return m_snore.data();
}

const QString &SnorePlugin::name() const
{
    return m_name;
}

void SnorePlugin::startTimeout(Notification &notification)
{
    if(notification.sticky())
    {
        return;
    }
    uint id = notification.id();
    QTimer *timer = qvariant_cast<QTimer*>(notification.hints().privateValue(this, "timeout"));
    if(notification.updateID() != (uint)-1)
    {
        id = notification.updateID();
    }
    if(timer)
    {
        timer->stop();
    }
    else
    {
        timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setProperty("notificationID", id);
    }

    timer->setInterval(notification.timeout() * 1000);
    connect(timer,SIGNAL(timeout()),this,SLOT(notificationTimedOut()));
    timer->start();
}

void SnorePlugin::notificationTimedOut()
{

    QTimer *timer = qobject_cast<QTimer*>(sender());
    Notification n = snore()->getActiveNotificationByID(timer->property("notificationID").toUInt());
    if(n.isValid())
    {
        qDebug() << Q_FUNC_INFO << n;
        timer->deleteLater();
        snore()->requestCloseNotification(n,NotificationEnums::CloseReasons::TIMED_OUT);
    }
    timer->deleteLater();
}

bool SnorePlugin::deinitialize()
{
    if(m_initialized)
    {
        qDebug() << "Deinitialize" << m_name << this;
        m_initialized = false;
        return true;
    }
    return false;
}

}
