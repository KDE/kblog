/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "kblog/blogcomment.h"

#include <QString>
#include <QUrl>
#include <QDateTime>

Q_DECLARE_METATYPE(KBlog::BlogComment::Status)

using namespace KBlog;

class testBlogComment: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testValidity_data();
};

#include "testblogcomment.moc"

void testBlogComment::testValidity_data()
{
    QTest::addColumn<QString>("commentId");
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("content");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("email");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<BlogComment::Status>("status");
    QTest::addColumn<QString>("error");
    QTest::addColumn<QDateTime>("creationDateTime");
    QTest::addColumn<QDateTime>("modificationDateTime");

    QTest::newRow("SimpleTest")
            << QString::fromLatin1("ABC123")
            << QString::fromLatin1("Title")
            << QString::fromLatin1("Content")
            << QString::fromLatin1("Name")
            << QString::fromLatin1("E-Mail")
            << QUrl(QLatin1String("http://my.link/in/outer/space/fancy/ABC123"))
            << BlogComment::New
            << QString::fromLatin1("Error")
            << QDateTime::currentDateTime()
            << QDateTime::currentDateTime();
}

void testBlogComment::testValidity()
{
    BlogComment p;

    QFETCH(QString, commentId);
    QFETCH(QString, title);
    QFETCH(QString, content);
    QFETCH(QString, name);
    QFETCH(QString, email);
    QFETCH(QUrl, url);
    QFETCH(BlogComment::Status, status);
    QFETCH(QString, error);
    QFETCH(QDateTime, creationDateTime);
    QFETCH(QDateTime, modificationDateTime);

    p.setCommentId(commentId);
    p.setTitle(title);
    p.setContent(content);
    p.setName(name);
    p.setEmail(email);
    p.setUrl(url);
    p.setStatus(status);
    p.setError(error);
    p.setCreationDateTime(creationDateTime);
    p.setModificationDateTime(modificationDateTime);

    QCOMPARE(p.commentId(), commentId);
    QCOMPARE(p.title(), title);
    QCOMPARE(p.content(), content);
    QCOMPARE(p.name(), name);
    QCOMPARE(p.email(), email);
    QCOMPARE(p.url(), url);
    QCOMPARE(p.status(), status);
    QCOMPARE(p.error(), error);
    QCOMPARE(p.creationDateTime(), creationDateTime);
    QCOMPARE(p.modificationDateTime(), modificationDateTime);

}

QTEST_GUILESS_MAIN(testBlogComment)
