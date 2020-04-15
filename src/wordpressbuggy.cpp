/*
  This file is part of the kblog library.

  Copyright (c) 2006-2009 Christian Weilbach <christian_weilbach@web.de>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "wordpressbuggy.h"
#include "wordpressbuggy_p.h"

#include "blogpost.h"

#include "kblog_debug.h"
#include <KLocalizedString>

#include <kio/job.h>

#include <QStringList>

using namespace KBlog;

WordpressBuggy::WordpressBuggy(const QUrl &server, QObject *parent)
    : MovableType(server, *new WordpressBuggyPrivate, parent)
{
    qCDebug(KBLOG_LOG);
}

WordpressBuggy::WordpressBuggy(const QUrl &server, WordpressBuggyPrivate &dd,
                               QObject *parent)
    : MovableType(server, dd, parent)
{
    qCDebug(KBLOG_LOG);
}

WordpressBuggy::~WordpressBuggy()
{
    qCDebug(KBLOG_LOG);
}

void WordpressBuggy::createPost(KBlog::BlogPost *post)
{
    // reimplemented because we do this:
    // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
    qCDebug(KBLOG_LOG);
    Q_D(WordpressBuggy);

    // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
    // names later, but we need to map them to categoryId of the blog
    d->loadCategories();
    if (d->mCategoriesList.isEmpty()) {
        qCDebug(KBLOG_LOG) << "No categories in the cache yet. Have to fetch them first.";
        d->mCreatePostCache << post;
        connect(this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                this, SLOT(slotTriggerCreatePost()));
        listCategories();
    } else {
        qCDebug(KBLOG_LOG) << "createPost()";
        if (!post) {
            qCritical() << "WordpressBuggy::createPost: post is a null pointer";
            Q_EMIT error(Other, i18n("Post is a null pointer."));
            return;
        }
        qCDebug(KBLOG_LOG) << "Creating new Post with blogId" << blogId();

        bool publish = post->isPrivate();
        // If we do setPostCategories() later than we disable publishing first.
        if (!post->categories().isEmpty()) {
            post->setPrivate(true);
            if (d->mSilentCreationList.contains(post)) {
                qCDebug(KBLOG_LOG) << "Post already in mSilentCreationList, this *should* never happen!";
            } else {
                d->mSilentCreationList << post;
            }
        }

        QString xmlMarkup = QStringLiteral("<?xml version=\"1.0\"?>");
        xmlMarkup += QStringLiteral("<methodCall>");
        xmlMarkup += QStringLiteral("<methodName>metaWeblog.newPost</methodName>");
        xmlMarkup += QStringLiteral("<params><param>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + blogId() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</param>");
        xmlMarkup += QStringLiteral("<param>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + username() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</param><param>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + password() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</param>");
        xmlMarkup += QStringLiteral("<param><struct>");
        xmlMarkup += QStringLiteral("<member><name>description</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->content() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>title</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->title() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");

        xmlMarkup += QStringLiteral("<name>dateCreated</name>");
        xmlMarkup += QStringLiteral("<value><dateTime.iso8601>") +
                     post->creationDateTime().toUTC().toString(QStringLiteral("yyyyMMddThh:mm:ss")) +
                     QStringLiteral("</dateTime.iso8601></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_allow_comments</name>");
        xmlMarkup += QStringLiteral("<value><int>%1</int></value>").arg((int)post->isCommentAllowed());
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_allow_pings</name>");
        xmlMarkup += QStringLiteral("<value><int>%1</int></value>").arg((int)post->isTrackBackAllowed());
        xmlMarkup += QStringLiteral("</member><member>");
        if (!post->additionalContent().isEmpty()) {
            xmlMarkup += QStringLiteral("<name>mt_text_more</name>");
            xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->additionalContent() + QStringLiteral("]]></string></value>");
            xmlMarkup += QStringLiteral("</member><member>");
        }
        xmlMarkup += QStringLiteral("<name>wp_slug</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->slug() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_excerpt</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->summary() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_keywords</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->tags().join(QLatin1Char(',')) + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member></struct></param>");
        xmlMarkup += QStringLiteral("<param><value><boolean>") +
                     QStringLiteral("%1").arg((int)(!post->isPrivate())) +
                     QStringLiteral("</boolean></value></param>");
        xmlMarkup += QStringLiteral("</params></methodCall>");

        QByteArray postData;
        QDataStream stream(&postData, QIODevice::WriteOnly);
        stream.writeRawData(xmlMarkup.toUtf8().constData(), xmlMarkup.toUtf8().length());

        KIO::StoredTransferJob *job = KIO::storedHttpPost(postData, url(), KIO::HideProgressInfo);
        if (!job) {
            qCWarning(KBLOG_LOG) << "Failed to create job for: " << url().url();
            return;
        }

        d->mCreatePostMap[ job ] = post;


        job->addMetaData(
            QStringLiteral("customHTTPHeader"), QStringLiteral("X-hacker: Shame on you Wordpress, ") + QString() +
            QStringLiteral("you took another 4 hours of my life to work around the stupid dateTime bug."));
        job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: text/xml; charset=utf-8"));
        job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
        job->addMetaData(QStringLiteral("UserAgent"), userAgent());

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotCreatePost(KJob*)));
        // HACK: uuh this a bit ugly now... reenable the original publish argument,
        // since createPost should have parsed now
        post->setPrivate(publish);
    }
}

void WordpressBuggy::modifyPost(KBlog::BlogPost *post)
{
    // reimplemented because we do this:
    // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
    qCDebug(KBLOG_LOG);
    Q_D(WordpressBuggy);

    // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
    // names later, but we need to map them to categoryId of the blog
    d->loadCategories();
    if (d->mCategoriesList.isEmpty()) {
        qCDebug(KBLOG_LOG) << "No categories in the cache yet. Have to fetch them first.";
        d->mModifyPostCache << post;
        connect(this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                this, SLOT(slotTriggerModifyPost()));
        listCategories();
    } else {
        if (!post) {
            qCritical() << "WordpressBuggy::modifyPost: post is a null pointer";
            Q_EMIT error(Other, i18n("Post is a null pointer."));
            return;
        }

        qCDebug(KBLOG_LOG) << "Uploading Post with postId" << post->postId();

        QString xmlMarkup = QStringLiteral("<?xml version=\"1.0\"?>");
        xmlMarkup += QStringLiteral("<methodCall>");
        xmlMarkup += QStringLiteral("<methodName>metaWeblog.editPost</methodName>");
        xmlMarkup += QStringLiteral("<params><param>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->postId() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</param>");
        xmlMarkup += QStringLiteral("<param>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + username() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</param><param>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + password() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</param>");
        xmlMarkup += QStringLiteral("<param><struct>");
        xmlMarkup += QStringLiteral("<member><name>description</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->content() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>title</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->title() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");

        xmlMarkup += QStringLiteral("<name>lastModified</name>");
        xmlMarkup += QStringLiteral("<value><dateTime.iso8601>") +
                     post->modificationDateTime().toUTC().toString(QStringLiteral("yyyyMMddThh:mm:ss")) +
                     QStringLiteral("</dateTime.iso8601></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>dateCreated</name>");
        xmlMarkup += QStringLiteral("<value><dateTime.iso8601>") +
                     post->creationDateTime().toUTC().toString(QStringLiteral("yyyyMMddThh:mm:ss")) +
                     QStringLiteral("</dateTime.iso8601></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_allow_comments</name>");
        xmlMarkup += QStringLiteral("<value><int>%1</int></value>").arg((int)post->isCommentAllowed());
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_allow_pings</name>");
        xmlMarkup += QStringLiteral("<value><int>%1</int></value>").arg((int)post->isTrackBackAllowed());
        xmlMarkup += QStringLiteral("</member><member>");
        if (!post->additionalContent().isEmpty()) {
            xmlMarkup += QStringLiteral("<name>mt_text_more</name>");
            xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->additionalContent() + QStringLiteral("]]></string></value>");
            xmlMarkup += QStringLiteral("</member><member>");
        }
        xmlMarkup += QStringLiteral("<name>wp_slug</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->slug() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_excerpt</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->summary() + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member><member>");
        xmlMarkup += QStringLiteral("<name>mt_keywords</name>");
        xmlMarkup += QStringLiteral("<value><string><![CDATA[") + post->tags().join(QLatin1Char(',')) + QStringLiteral("]]></string></value>");
        xmlMarkup += QStringLiteral("</member></struct></param>");
        xmlMarkup += QStringLiteral("<param><value><boolean>") +
                     QStringLiteral("%1").arg((int)(!post->isPrivate())) +
                     QStringLiteral("</boolean></value></param>");
        xmlMarkup += QStringLiteral("</params></methodCall>");

        QByteArray postData;
        QDataStream stream(&postData, QIODevice::WriteOnly);
        stream.writeRawData(xmlMarkup.toUtf8().constData(), xmlMarkup.toUtf8().length());

        KIO::StoredTransferJob *job = KIO::storedHttpPost(postData, url(), KIO::HideProgressInfo);
        if (!job) {
            qCWarning(KBLOG_LOG) << "Failed to create job for: " << url().url();
            return;
        }

        d->mModifyPostMap[ job ] = post;


        job->addMetaData(
            QStringLiteral("customHTTPHeader"), QStringLiteral("X-hacker: Shame on you Wordpress, ") + QString() +
            QStringLiteral("you took another 4 hours of my life to work around the stupid dateTime bug."));
        job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: text/xml; charset=utf-8"));
        job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
        job->addMetaData(QStringLiteral("UserAgent"), userAgent());

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotModifyPost(KJob*)));
    }
}

QString WordpressBuggy::interfaceName() const
{
    return QStringLiteral("Movable Type");
}

WordpressBuggyPrivate::WordpressBuggyPrivate()
{
}

WordpressBuggyPrivate::~WordpressBuggyPrivate()
{
    qCDebug(KBLOG_LOG);
}

QList<QVariant> WordpressBuggyPrivate::defaultArgs(const QString &id)
{
    Q_Q(WordpressBuggy);
    QList<QVariant> args;
    if (!id.isEmpty()) {
        args << QVariant(id);
    }
    args << QVariant(q->username())
         << QVariant(q->password());
    return args;
}

void WordpressBuggyPrivate::slotCreatePost(KJob *job)
{
    qCDebug(KBLOG_LOG);

    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());

    Q_Q(WordpressBuggy);

    KBlog::BlogPost *post = mCreatePostMap[ job ];
    mCreatePostMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotCreatePost error:" << job->errorString();
        Q_EMIT q->errorPost(WordpressBuggy::XmlRpc, job->errorString(), post);
        return;
    }

    QRegExp rxError(QStringLiteral("faultString"));
    if (rxError.indexIn(data) != -1) {
        rxError = QRegExp(QStringLiteral("<string>(.+)</string>"));
        if (rxError.indexIn(data) != -1) {
            qCDebug(KBLOG_LOG) << "RegExp of faultString failed.";
        }
        qCDebug(KBLOG_LOG) << rxError.cap(1);
        Q_EMIT q->errorPost(WordpressBuggy::XmlRpc, rxError.cap(1), post);
        return;
    }

    QRegExp rxId(QStringLiteral("<string>(.+)</string>"));
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        Q_EMIT q->errorPost(WordpressBuggy::XmlRpc,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( \"<string>(.+)</string>\" ) matches" << rxId.cap(1);

    post->setPostId(rxId.cap(1));
    if (mSilentCreationList.contains(post)) {
        // set the categories and publish afterwards
        setPostCategories(post, !post->isPrivate());
    } else {
        qCDebug(KBLOG_LOG) << "emitting createdPost()"
                           << "for title: \"" << post->title();
        Q_EMIT q->createdPost(post);
        post->setStatus(KBlog::BlogPost::Created);
    }
}

void WordpressBuggyPrivate::slotModifyPost(KJob *job)
{
    qCDebug(KBLOG_LOG);

    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());

    KBlog::BlogPost *post = mModifyPostMap[ job ];
    mModifyPostMap.remove(job);
    Q_Q(WordpressBuggy);
    if (job->error() != 0) {
        qCritical() << "slotModifyPost error:" << job->errorString();
        Q_EMIT q->errorPost(WordpressBuggy::XmlRpc, job->errorString(), post);
        return;
    }

    QRegExp rxError(QStringLiteral("faultString"));
    if (rxError.indexIn(data) != -1) {
        rxError = QRegExp(QStringLiteral("<string>(.+)</string>"));
        if (rxError.indexIn(data) != -1) {
            qCDebug(KBLOG_LOG) << "RegExp of faultString failed.";
        }
        qCDebug(KBLOG_LOG) << rxError.cap(1);
        Q_EMIT q->errorPost(WordpressBuggy::XmlRpc, rxError.cap(1), post);
        return;
    }

    QRegExp rxId(QStringLiteral("<boolean>(.+)</boolean>"));
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        Q_EMIT q->errorPost(WordpressBuggy::XmlRpc,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( \"<boolean>(.+)</boolean>\" ) matches" << rxId.cap(1);

    if (rxId.cap(1).toInt() == 1) {
        qCDebug(KBLOG_LOG) << "Post successfully updated.";
        if (mSilentCreationList.contains(post)) {
            post->setStatus(KBlog::BlogPost::Created);
            Q_EMIT q->createdPost(post);
            mSilentCreationList.removeOne(post);
        } else {
            if (!post->categories().isEmpty()) {
                setPostCategories(post, false);
            }
        }
    }
}

#include "moc_wordpressbuggy.cpp"
