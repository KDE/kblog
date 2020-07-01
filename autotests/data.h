/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBLOG_TEST_DATA_H_
#define KBLOG_TEST_DATA_H_

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QTimeZone>
#include <QDateTime>

QUrl mUrl(QLatin1String("http://kblogunittests.wordpress.com/xmlrpc.php"));
QString mUsername(QLatin1String("kblogunittests"));
QString mPassword(QLatin1String("k0nt4ctbl0g"));
QString mBlogId(QLatin1String("1"));

QDateTime mCreationDateTime(QDateTime::currentDateTime());
QDateTime mModificationDateTime(QDateTime::currentDateTime());
QString mTitle(QLatin1String("TestBlog"));
QString mContent(QLatin1String("TestBlog: <strong>posted</strong> content."));
QString mModifiedContent(QLatin1String("TestBlog: <strong>modified</strong>content."));
bool mPrivate = false;
QString mPostId(QLatin1String("113"));

QString mCommentTitle(QLatin1String("TestBlog Comment"));
QString mCommentContent(QLatin1String("TestBlog: posted comment."));
QString mCommentEmail(QLatin1String("fancy_mail@not.valid"));
bool mCommentAllowed = true;
bool mTrackBackAllowed = true;
QStringList mTags(QLatin1String("funny"));
QStringList mCategories(QLatin1String("funny"));
QString mSummary = QLatin1String("A simple summary.");

#endif
