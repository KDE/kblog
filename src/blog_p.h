/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BLOG_P_H
#define BLOG_P_H

#include "blog.h"

#include <QTimeZone>
#include <QUrl>

#include <kxmlrpcclient/client.h>

namespace KBlog
{

class BlogPrivate
{
public:
    BlogPrivate();
    virtual ~BlogPrivate();
    Blog *q_ptr;
    QString mBlogId;
    QString mUsername;
    QString mPassword;
    QString mUserAgent;
    QUrl mUrl;
    QTimeZone mTimeZone;
    Q_DECLARE_PUBLIC(Blog)
};

} //namespace KBlog

#endif
