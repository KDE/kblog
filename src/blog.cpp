/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "blog.h"
#include "blog_p.h"
#include "blogpost_p.h"
#include "blog_config.h"

#include "kblog_debug.h"

using namespace KBlog;

Blog::Blog(const QUrl &server, QObject *parent, const QString &applicationName,
           const QString &applicationVersion) :
    QObject(parent), d_ptr(new BlogPrivate)
{
    Q_UNUSED(server);
    d_ptr->q_ptr = this;
    setUserAgent(applicationName, applicationVersion);
}

Blog::Blog(const QUrl &server, BlogPrivate &dd, QObject *parent,
           const QString &applicationName, const QString &applicationVersion)
    : QObject(parent), d_ptr(&dd)
{
    Q_UNUSED(server);
    d_ptr->q_ptr = this;
    setUserAgent(applicationName, applicationVersion);
}

Blog::~Blog()
{
    qCDebug(KBLOG_LOG) << "~Blog()";
    delete d_ptr;
}

QString Blog::userAgent() const
{
    Q_D(const Blog);
    return d->mUserAgent;
}

void Blog::setUserAgent(const QString &applicationName,
                        const QString &applicationVersion)
{
    Q_D(Blog);
    QString userAgent;
    if (!applicationName.isEmpty() &&
            !applicationVersion.isEmpty()) {
        userAgent = QLatin1Char('(') + applicationName + QLatin1Char('/') + applicationVersion + QStringLiteral(") KDE-KBlog/");
    } else {
        userAgent = QStringLiteral("KDE-KBlog/");
    }
    userAgent += QStringLiteral(KDEPIMLIBS_VERSION);
    d->mUserAgent = userAgent;
}

void Blog::setPassword(const QString &pass)
{
    Q_D(Blog);
    d->mPassword = pass;
}

QString Blog::password() const
{
    Q_D(const Blog);
    return d->mPassword;
}

QString Blog::username() const
{
    Q_D(const Blog);
    return d->mUsername;
}

void Blog::setUsername(const QString &username)
{
    Q_D(Blog);
    d->mUsername = username;
}

void Blog::setBlogId(const QString &blogId)
{
    Q_D(Blog);
    d->mBlogId = blogId;
}

QString Blog::blogId() const
{
    Q_D(const Blog);
    return d->mBlogId;
}

void Blog::setUrl(const QUrl &url)
{
    Q_D(Blog);
    d->mUrl = url;
}

QUrl Blog::url() const
{
    Q_D(const Blog);
    return d->mUrl;
}

void Blog::setTimeZone(const QTimeZone &tz)
{
    Q_D(Blog);
    d->mTimeZone = tz;
}

QTimeZone Blog::timeZone()
{
    Q_D(const Blog);
    return d->mTimeZone;
}

BlogPrivate::BlogPrivate() : q_ptr(nullptr)
{
}

BlogPrivate::~BlogPrivate()
{
    qCDebug(KBLOG_LOG) << "~BlogPrivate()";
}
