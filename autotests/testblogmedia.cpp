/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QtCore>

#include <QTest>
#include "kblog/blogmedia.h"
#include "qurl.h"

Q_DECLARE_METATYPE(KBlog::BlogMedia::Status)

using namespace KBlog;

class testBlogMedia: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testValidity_data();
};

#include "testblogmedia.moc"

void testBlogMedia::testValidity_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("mimetype");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<BlogMedia::Status>("status");
    QTest::addColumn<QString>("error");

    QTest::newRow("SimpleTest")
            << QString::fromLatin1("FancyMedia")
            << QUrl(QLatin1String("http://my.link/in/outer/space/fancyMedia.jpg"))
            << QString::fromLatin1("text/xml")
            << QByteArray("Tags 1 2")
            << BlogMedia::New
            << QString::fromLatin1("Error");
}

void testBlogMedia::testValidity()
{
    BlogMedia p;

    QFETCH(QString, name);
    QFETCH(QUrl, url);
    QFETCH(QString, mimetype);
    QFETCH(QByteArray, data);
    QFETCH(BlogMedia::Status, status);
    QFETCH(QString, error);

    p.setName(name);
    p.setUrl(url);
    p.setMimetype(mimetype);
    p.setData(data);
    p.setStatus(status);
    p.setError(error);

    QCOMPARE(p.name(), name);
    QCOMPARE(p.url(), url);
    QCOMPARE(p.mimetype(), mimetype);
    QCOMPARE(p.data(), data);
    QCOMPARE(p.status(), status);
    QCOMPARE(p.error(), error);

}

QTEST_GUILESS_MAIN(testBlogMedia)
