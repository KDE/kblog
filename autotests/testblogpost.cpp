/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "kblog/blogpost.h"

Q_DECLARE_METATYPE(KBlog::BlogPost::Status)

using namespace KBlog;

class testBlogPost: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testValidity_data();
};

#include "testblogpost.moc"

void testBlogPost::testValidity_data()
{
    QTest::addColumn<QString>("postId");
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("content");
    QTest::addColumn<bool>("isPrivate");
//     QTest::addColumn<QString>( "abbreviatedContent" );
    QTest::addColumn<QUrl>("link");
    QTest::addColumn<QUrl>("permalink");
    QTest::addColumn<bool>("isCommentAllowed");
    QTest::addColumn<bool>("isTrackBackAllowed");
    QTest::addColumn<QString>("summary");
    QTest::addColumn<QStringList>("tags");
//     QTest::addColumn<QList<KUrl> >( "trackBackUrls" );
    QTest::addColumn<QString>("mood");
    QTest::addColumn<QString>("music");
    QTest::addColumn<QStringList>("categories");
    QTest::addColumn<QDateTime>("creationDateTime");
    QTest::addColumn<QDateTime>("modificationDateTime");
    QTest::addColumn<BlogPost::Status>("status");
    QTest::addColumn<QString>("error");

//     QList<KUrl> url;
//     url.append( QUrl("http://track.back.url/some/path") );
    QTest::newRow("SimpleTest")
            << QString::fromLatin1("123ABC")
            << QString::fromLatin1("Title")
            << QString::fromLatin1("Content")
            << true //<< QString("Abbreviated Content")
            << QUrl(QLatin1String("http://my.link/in/outer/space"))
            << QUrl(QLatin1String("http://my.perma/link/space"))
            << true
            << true
            << QString::fromLatin1("Summary")
            << QStringList(QLatin1String("Tags"))   //<< url
            << QString::fromLatin1("Mood") << QString::fromLatin1("Music")
            << QStringList(QLatin1String("Category"))
            << QDateTime::currentDateTime()
            << QDateTime::currentDateTime()
            << BlogPost::New
            << QString::fromLatin1("Error");
}

void testBlogPost::testValidity()
{
    BlogPost p;

    QFETCH(QString, postId);
    QFETCH(QString, title);
    QFETCH(QString, content);
    QFETCH(bool, isPrivate);
//     QFETCH( QString, abbreviatedContent );
    QFETCH(QUrl, link);
    QFETCH(QUrl, permalink);
    QFETCH(bool, isCommentAllowed);
    QFETCH(bool, isTrackBackAllowed);
    QFETCH(QString, summary);
    QFETCH(QStringList, tags);
//     QFETCH( QList<KUrl>, trackBackUrls );
    QFETCH(QString, mood);
    QFETCH(QString, music);
    QFETCH(QStringList, categories);
    QFETCH(QDateTime, creationDateTime);
    QFETCH(QDateTime, modificationDateTime);
    QFETCH(BlogPost::Status, status);
    QFETCH(QString, error);
    p.setPostId(postId);
    p.setTitle(title);
    p.setContent(content);
    p.setPrivate(isPrivate);
//     p.setAbbreviatedContent( abbreviatedContent );
    p.setLink(link);
    p.setPermaLink(permalink);
    p.setCommentAllowed(isCommentAllowed);
    p.setTrackBackAllowed(isTrackBackAllowed);
    p.setSummary(summary);
    p.setTags(tags);
//     p.setTrackBackUrls( trackBackUrls );
    p.setMood(mood);
    p.setMusic(music);
    p.setCategories(categories);
    p.setCreationDateTime(creationDateTime);
    p.setModificationDateTime(modificationDateTime);
    p.setStatus(status);
    p.setError(error);

    QCOMPARE(p.postId(), postId);
    QCOMPARE(p.title(), title);
    QCOMPARE(p.content(), content);
    QCOMPARE(p.isPrivate(), isPrivate);
//     QCOMPARE( p.abbreviatedContent(), abbreviatedContent );
    QCOMPARE(p.link(), link);
    QCOMPARE(p.permaLink(), permalink);
    QCOMPARE(p.isCommentAllowed(), isCommentAllowed);
    QCOMPARE(p.isTrackBackAllowed(), isTrackBackAllowed);
    QCOMPARE(p.summary(), summary);
    QCOMPARE(p.tags(), tags);
//     QCOMPARE( p.trackBackUrls(), trackBackUrls );
    QCOMPARE(p.mood(), mood);
    QCOMPARE(p.music(), music);
    QCOMPARE(p.categories(), categories);
    QCOMPARE(p.creationDateTime(), creationDateTime);
    QCOMPARE(p.modificationDateTime(), modificationDateTime);
    QCOMPARE(p.status(), status);
    QCOMPARE(p.error(), error);
}

QTEST_GUILESS_MAIN(testBlogPost)
