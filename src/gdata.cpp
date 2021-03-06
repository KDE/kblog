/*
    This file is part of the kblog library.

    SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gdata.h"
#include "gdata_p.h"
#include "blogpost.h"
#include "blogcomment.h"
#include "feedretriever.h"

#include <syndication/loader.h>
#include <syndication/item.h>
#include <syndication/category.h>

#include <kio/job.h>
#include "kblog_debug.h"
#include <KLocalizedString>
#include <QUrl>
#include <QUrlQuery>

#include <QByteArray>
#include <QRegExp>

#define TIMEOUT 600

using namespace KBlog;

GData::GData(const QUrl &server, QObject *parent)
    : Blog(server, *new GDataPrivate, parent)
{
    qCDebug(KBLOG_LOG);
    setUrl(server);
}

GData::~GData()
{
    qCDebug(KBLOG_LOG);
}

QString GData::interfaceName() const
{
    qCDebug(KBLOG_LOG);
    return QStringLiteral("Google Blogger Data");
}

QString GData::fullName() const
{
    qCDebug(KBLOG_LOG);
    return d_func()->mFullName;
}

void GData::setFullName(const QString &fullName)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);
    d->mFullName = fullName;
}

QString GData::profileId() const
{
    qCDebug(KBLOG_LOG);
    return d_func()->mProfileId;
}

void GData::setProfileId(const QString &pid)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);
    d->mProfileId = pid;
}

void GData::fetchProfileId()
{
    qCDebug(KBLOG_LOG);
    QByteArray data;
    KIO::StoredTransferJob *job = KIO::storedGet(url(), KIO::NoReload, KIO::HideProgressInfo);
    QUrl blogUrl = url();
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotFetchProfileId(KJob*)));
}

void GData::listBlogs()
{
    qCDebug(KBLOG_LOG);
    Syndication::Loader *loader = Syndication::Loader::create();
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListBlogs(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QStringLiteral("http://www.blogger.com/feeds/%1/blogs").arg(profileId())), new FeedRetriever);
}

void GData::listRecentPosts(const QStringList &labels, int number,
                            const QDateTime &upMinTime, const QDateTime &upMaxTime,
                            const QDateTime &pubMinTime, const QDateTime &pubMaxTime)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);
    QString urlString(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default"));
    if (! labels.empty()) {
        urlString += QStringLiteral("/-/") + labels.join(QLatin1Char('/'));
    }
    qCDebug(KBLOG_LOG) << "listRecentPosts()";
    QUrl url(urlString);
    QUrlQuery q;

    if (!upMinTime.isNull()) {
        q.addQueryItem(QStringLiteral("updated-min"), upMinTime.toUTC().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")));
    }

    if (!upMaxTime.isNull()) {
        q.addQueryItem(QStringLiteral("updated-max"), upMaxTime.toUTC().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")));
    }

    if (!pubMinTime.isNull()) {
        q.addQueryItem(QStringLiteral("published-min"), pubMinTime.toUTC().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")));
    }

    if (!pubMaxTime.isNull()) {
        q.addQueryItem(QStringLiteral("published-max"), pubMaxTime.toUTC().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")));
    }
    url.setQuery(q);

    Syndication::Loader *loader = Syndication::Loader::create();
    if (number > 0) {
        d->mListRecentPostsMap[ loader ] = number;
    }
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListRecentPosts(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(url, new FeedRetriever);
}

void GData::listRecentPosts(int number)
{
    qCDebug(KBLOG_LOG);
    listRecentPosts(QStringList(), number);
}

void GData::listComments(KBlog::BlogPost *post)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);
    Syndication::Loader *loader = Syndication::Loader::create();
    d->mListCommentsMap[ loader ] = post;
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QLatin1Char('/') +
                                  post->postId() + QStringLiteral("/comments/default")), new FeedRetriever);
}

void GData::listAllComments()
{
    qCDebug(KBLOG_LOG);
    Syndication::Loader *loader = Syndication::Loader::create();
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListAllComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QStringLiteral("http://www.blogger.com/feeds/%1/comments/default").arg(blogId())), new FeedRetriever);
}

void GData::fetchPost(KBlog::BlogPost *post)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    qCDebug(KBLOG_LOG);
    Syndication::Loader *loader = Syndication::Loader::create();
    d->mFetchPostMap[ loader ] = post;
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotFetchPost(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QStringLiteral("http://www.blogger.com/feeds/%1/posts/default").arg(blogId())), new FeedRetriever);
}

void GData::modifyPost(KBlog::BlogPost *post)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        Q_EMIT errorPost(Atom, i18n("Authentication failed."), post);
        return;
    }

    QString atomMarkup = QStringLiteral("<entry xmlns='http://www.w3.org/2005/Atom'>");
    atomMarkup += QStringLiteral("<id>tag:blogger.com,1999:blog-") + blogId();
    atomMarkup += QStringLiteral(".post-") + post->postId() + QStringLiteral("</id>");
    atomMarkup += QStringLiteral("<published>") + post->creationDateTime().toString() + QStringLiteral("</published>");
    atomMarkup += QStringLiteral("<updated>") + post->modificationDateTime().toString() + QStringLiteral("</updated>");
    atomMarkup += QStringLiteral("<title type='text'>") + post->title() + QStringLiteral("</title>");
    if (post->isPrivate()) {
        atomMarkup += QStringLiteral("<app:control xmlns:app='http://purl.org/atom/app#'>");
        atomMarkup += QStringLiteral("<app:draft>yes</app:draft></app:control>");
    }
    atomMarkup += QStringLiteral("<content type='xhtml'>");
    atomMarkup += QStringLiteral("<div xmlns='http://www.w3.org/1999/xhtml'>");
    atomMarkup += post->content();
    atomMarkup += QStringLiteral("</div></content>");
    const auto tags = post->tags();
    for (const QString &tag : tags) {
        atomMarkup += QStringLiteral("<category scheme='http://www.blogger.com/atom/ns#' term='") + tag + QStringLiteral("' />");
    }
    atomMarkup += QStringLiteral("<author>");
    if (!fullName().isEmpty()) {
        atomMarkup += QStringLiteral("<name>") + fullName() + QStringLiteral("</name>");
    }
    atomMarkup += QStringLiteral("<email>") + username() + QStringLiteral("</email>");
    atomMarkup += QStringLiteral("</author>");
    atomMarkup += QStringLiteral("</entry>");
    QByteArray postData;
    QDataStream stream(&postData, QIODevice::WriteOnly);
    stream.writeRawData(atomMarkup.toUtf8().constData(), atomMarkup.toUtf8().length());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default/") + post->postId()),
                                  KIO::HideProgressInfo);

    Q_ASSERT(job);

    d->mModifyPostMap[ job ] = post;

    job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/atom+xml; charset=utf-8"));
    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString +
                     QStringLiteral("\r\nX-HTTP-Method-Override: PUT"));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotModifyPost(KJob*)));
}

void GData::createPost(KBlog::BlogPost *post)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        Q_EMIT errorPost(Atom, i18n("Authentication failed."), post);
        return;
    }

    QString atomMarkup = QStringLiteral("<entry xmlns='http://www.w3.org/2005/Atom'>");
    atomMarkup += QStringLiteral("<title type='text'>") + post->title() + QStringLiteral("</title>");
    if (post->isPrivate()) {
        atomMarkup += QStringLiteral("<app:control xmlns:app='http://purl.org/atom/app#'>");
        atomMarkup += QStringLiteral("<app:draft>yes</app:draft></app:control>");
    }
    atomMarkup += QStringLiteral("<content type='xhtml'>");
    atomMarkup += QStringLiteral("<div xmlns='http://www.w3.org/1999/xhtml'>");
    atomMarkup += post->content(); // FIXME check for Utf
    atomMarkup += QStringLiteral("</div></content>");
    QList<QString>::ConstIterator it = post->tags().constBegin();
    QList<QString>::ConstIterator end = post->tags().constEnd();
    for (; it != end; ++it) {
        atomMarkup += QStringLiteral("<category scheme='http://www.blogger.com/atom/ns#' term='") + (*it) + QStringLiteral("' />");
    }
    atomMarkup += QStringLiteral("<author>");
    if (!fullName().isEmpty()) {
        atomMarkup += QStringLiteral("<name>") + fullName() + QStringLiteral("</name>");
    }
    atomMarkup += QStringLiteral("<email>") + username() + QStringLiteral("</email>");
    atomMarkup += QStringLiteral("</author>");
    atomMarkup += QStringLiteral("</entry>");

    QByteArray postData;
    QDataStream stream(&postData, QIODevice::WriteOnly);
    stream.writeRawData(atomMarkup.toUtf8().constData(), atomMarkup.toUtf8().length());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default")),
                                  KIO::HideProgressInfo);

    Q_ASSERT(job);
    d->mCreatePostMap[ job ] = post;

    job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/atom+xml; charset=utf-8"));
    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotCreatePost(KJob*)));
}

void GData::removePost(KBlog::BlogPost *post)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        Q_EMIT errorPost(Atom, i18n("Authentication failed."), post);
        return;
    }

    QByteArray postData;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default/") + post->postId()),
                                  KIO::HideProgressInfo);
    if (!job) {
        qCWarning(KBLOG_LOG) << "Unable to create KIO job for http://www.blogger.com/feeds/"
                             << blogId() << QStringLiteral("/posts/default/") + post->postId();
        return;
    }

    d->mRemovePostMap[ job ] = post;


    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString +
                     QStringLiteral("\r\nX-HTTP-Method-Override: DELETE"));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotRemovePost(KJob*)));
}

void GData::createComment(KBlog::BlogPost *post, KBlog::BlogComment *comment)
{
    qCDebug(KBLOG_LOG);

    if (!comment) {
        qCritical() << "comment is null pointer";
        return;
    }

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    Q_D(GData);
    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        Q_EMIT errorComment(Atom, i18n("Authentication failed."), post, comment);
        return;
    }
    QString atomMarkup = QStringLiteral("<entry xmlns='http://www.w3.org/2005/Atom'>");
    atomMarkup += QStringLiteral("<title type=\"text\">") + comment->title() + QStringLiteral("</title>");
    atomMarkup += QStringLiteral("<content type=\"html\">") + comment->content() + QStringLiteral("</content>");
    atomMarkup += QStringLiteral("<author>");
    atomMarkup += QStringLiteral("<name>") + comment->name() + QStringLiteral("</name>");
    atomMarkup += QStringLiteral("<email>") + comment->email() + QStringLiteral("</email>");
    atomMarkup += QStringLiteral("</author></entry>");

    QByteArray postData;
    qCDebug(KBLOG_LOG) <<  postData;
    QDataStream stream(&postData, QIODevice::WriteOnly);
    stream.writeRawData(atomMarkup.toUtf8().constData(), atomMarkup.toUtf8().length());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/") + post->postId() + QStringLiteral("/comments/default")),
                                  KIO::HideProgressInfo);

    if (!job) {
        qCWarning(KBLOG_LOG) << "Unable to create KIO job for http://www.blogger.com/feeds/"
                             << blogId() << "/" << post->postId() << "/comments/default";
        return;
    }
    d->mCreateCommentMap[ job ][post] = comment;


    job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/atom+xml; charset=utf-8"));
    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString);
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotCreateComment(KJob*)));
}

void GData::removeComment(KBlog::BlogPost *post, KBlog::BlogComment *comment)
{
    qCDebug(KBLOG_LOG);
    Q_D(GData);
    qCDebug(KBLOG_LOG);

    if (!comment) {
        qCritical() << "comment is null pointer";
        return;
    }

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        Q_EMIT errorComment(Atom, i18n("Authentication failed."), post, comment);
        return;
    }

    QByteArray postData;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/") + post->postId() +
                                       QStringLiteral("/comments/default/") + comment->commentId()), KIO::HideProgressInfo);
    d->mRemoveCommentMap[ job ][ post ] = comment;

    if (!job) {
        qCWarning(KBLOG_LOG) << "Unable to create KIO job for http://www.blogger.com/feeds/"
                             << blogId() << post->postId()
                             << "/comments/default/" << comment->commentId();
    }

    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") +
                     d->mAuthenticationString + QStringLiteral("\r\nX-HTTP-Method-Override: DELETE"));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotRemoveComment(KJob*)));
}

GDataPrivate::GDataPrivate(): mAuthenticationString(), mAuthenticationTime()
{
    qCDebug(KBLOG_LOG);
}

GDataPrivate::~GDataPrivate()
{
    qCDebug(KBLOG_LOG);
}

bool GDataPrivate::authenticate()
{
    qCDebug(KBLOG_LOG);
    Q_Q(GData);
    QByteArray data;
    QUrl authGateway(QStringLiteral("https://www.google.com/accounts/ClientLogin"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("Email"), q->username());
    query.addQueryItem(QStringLiteral("Passwd"), q->password());
    query.addQueryItem(QStringLiteral("source"), q->userAgent());
    query.addQueryItem(QStringLiteral("service"), QStringLiteral("blogger"));
    authGateway.setQuery(query);
    if (!mAuthenticationTime.isValid() ||
            QDateTime::currentDateTime().currentSecsSinceEpoch() - mAuthenticationTime.currentSecsSinceEpoch() > TIMEOUT ||
            mAuthenticationString.isEmpty()) {
        KIO::TransferJob *job = KIO::http_post(authGateway, QByteArray(), KIO::HideProgressInfo);
        QObject::connect(job, &KIO::TransferJob::data,
                         q, [&data](KIO::Job *, const QByteArray &newdata) {
                            data.reserve(data.size() + newdata.size());
                            memcpy(data.data() + data.size(), newdata.data(), newdata.size());
                         });
        if (job->exec()) {
            QRegExp rx(QStringLiteral("Auth=(.+)"));
            if (rx.indexIn(QLatin1String(data)) != -1) {
                qCDebug(KBLOG_LOG) << "RegExp got authentication string:" << rx.cap(1);
                mAuthenticationString = rx.cap(1);
                mAuthenticationTime = QDateTime::currentDateTime();
                return true;
            }
        }
        return false;
    }
    return true;
}

void GDataPrivate::slotFetchProfileId(KJob *job)
{
    qCDebug(KBLOG_LOG);
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    Q_Q(GData);
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());
    if (!job->error()) {
        QRegExp pid(QStringLiteral("http://www.blogger.com/profile/(\\d+)"));
        if (pid.indexIn(data) != -1) {
            q->setProfileId(pid.cap(1));
            qCDebug(KBLOG_LOG) << "QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' matches" << pid.cap(1);
            Q_EMIT q->fetchedProfileId(pid.cap(1));
        } else {
            qCritical() << "QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' "
                        << " could not regexp the Profile ID";
            Q_EMIT q->error(GData::Other, i18n("Could not regexp the Profile ID."));
            Q_EMIT q->fetchedProfileId(QString());
        }
    } else {
        qCritical() << "Job Error: " << job->errorString();
        Q_EMIT q->error(GData::Other, job->errorString());
        Q_EMIT q->fetchedProfileId(QString());
    }
}

void GDataPrivate::slotListBlogs(Syndication::Loader *loader,
                                 const Syndication::FeedPtr &feed,
                                 Syndication::ErrorCode status)
{
    qCDebug(KBLOG_LOG);
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }
    if (status != Syndication::Success) {
        Q_EMIT q->error(GData::Atom, i18n("Could not get blogs."));
        return;
    }

    QList<QMap<QString, QString> > blogsList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        QRegExp rx(QStringLiteral("blog-(\\d+)"));
        QMap<QString, QString> blogInfo;
        if (rx.indexIn((*it)->id()) != -1) {
            qCDebug(KBLOG_LOG) << "QRegExp rx( 'blog-(\\d+)' matches" << rx.cap(1);
            blogInfo[QStringLiteral("id")] = rx.cap(1);
            blogInfo[QStringLiteral("title")] = (*it)->title();
            blogInfo[QStringLiteral("url")] = (*it)->link();
            blogInfo[QStringLiteral("summary")] = (*it)->description();   //TODO fix/add more
            blogsList << blogInfo;
        } else {
            qCritical() << "QRegExp rx( 'blog-(\\d+)' does not match anything in:"
                        << (*it)->id();
            Q_EMIT q->error(GData::Other, i18n("Could not regexp the blog id path."));
        }
    }
    qCDebug(KBLOG_LOG) << "Emitting listedBlogs(); ";
    Q_EMIT q->listedBlogs(blogsList);
}

void GDataPrivate::slotListComments(Syndication::Loader *loader,
                                    const Syndication::FeedPtr &feed,
                                    Syndication::ErrorCode status)
{
    qCDebug(KBLOG_LOG);
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }
    BlogPost *post = mListCommentsMap[ loader ];
    mListCommentsMap.remove(loader);

    if (status != Syndication::Success) {
        Q_EMIT q->errorPost(GData::Atom, i18n("Could not get comments."), post);
        return;
    }

    QList<KBlog::BlogComment> commentList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        BlogComment comment;
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) == -1) {
            qCritical() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
            Q_EMIT q->error(GData::Other, i18n("Could not regexp the comment id path."));
        } else {
            comment.setCommentId(rx.cap(1));
        }
        qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
        comment.setTitle((*it)->title());
        comment.setContent((*it)->content());
//  FIXME: assuming UTC for now
        comment.setCreationDateTime(QDateTime::fromSecsSinceEpoch((*it)->datePublished()));
        comment.setModificationDateTime(QDateTime::fromSecsSinceEpoch((*it)->dateUpdated()));
        commentList.append(comment);
    }
    qCDebug(KBLOG_LOG) << "Emitting listedComments()";
    Q_EMIT q->listedComments(post, commentList);
}

void GDataPrivate::slotListAllComments(Syndication::Loader *loader,
                                       const Syndication::FeedPtr &feed,
                                       Syndication::ErrorCode status)
{
    qCDebug(KBLOG_LOG);
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }

    if (status != Syndication::Success) {
        Q_EMIT q->error(GData::Atom, i18n("Could not get comments."));
        return;
    }

    QList<KBlog::BlogComment> commentList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        BlogComment comment;
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) == -1) {
            qCritical() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
            Q_EMIT q->error(GData::Other, i18n("Could not regexp the comment id path."));
        } else {
            comment.setCommentId(rx.cap(1));
        }

        qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
        comment.setTitle((*it)->title());
        comment.setContent((*it)->content());
//  FIXME: assuming UTC for now
        comment.setCreationDateTime(QDateTime::fromSecsSinceEpoch((*it)->datePublished()));
        comment.setModificationDateTime(QDateTime::fromSecsSinceEpoch((*it)->dateUpdated()));
        commentList.append(comment);
    }
    qCDebug(KBLOG_LOG) << "Emitting listedAllComments()";
    Q_EMIT q->listedAllComments(commentList);
}

void GDataPrivate::slotListRecentPosts(Syndication::Loader *loader,
                                       const Syndication::FeedPtr &feed,
                                       Syndication::ErrorCode status)
{
    qCDebug(KBLOG_LOG);
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }

    if (status != Syndication::Success) {
        Q_EMIT q->error(GData::Atom, i18n("Could not get posts."));
        return;
    }
    int number = 0;

    if (mListRecentPostsMap.contains(loader)) {
        number = mListRecentPostsMap[ loader ];
    }
    mListRecentPostsMap.remove(loader);

    QList<KBlog::BlogPost> postList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        BlogPost post;
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) == -1) {
            qCritical() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
            Q_EMIT q->error(GData::Other, i18n("Could not regexp the post id path."));
        } else {
            post.setPostId(rx.cap(1));
        }

        qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
        post.setTitle((*it)->title());
        post.setContent((*it)->content());
        post.setLink(QUrl((*it)->link()));
        QStringList labels;
        int catCount = (*it)->categories().count();
        QList< Syndication::CategoryPtr > cats = (*it)->categories();
        for (int i = 0; i < catCount; ++i) {
            if (cats[i]->label().isEmpty()) {
                labels.append(cats[i]->term());
            } else {
                labels.append(cats[i]->label());
            }
        }
        post.setTags(labels);
//  FIXME: assuming UTC for now
        post.setCreationDateTime(QDateTime::fromSecsSinceEpoch((*it)->datePublished()));
        post.setModificationDateTime(QDateTime::fromSecsSinceEpoch((*it)->dateUpdated()));
        post.setStatus(BlogPost::Fetched);
        postList.append(post);
        if (number-- == 0) {
            break;
        }
    }
    qCDebug(KBLOG_LOG) << "Emitting listedRecentPosts()";
    Q_EMIT q->listedRecentPosts(postList);
}

void GDataPrivate::slotFetchPost(Syndication::Loader *loader,
                                 const Syndication::FeedPtr &feed,
                                 Syndication::ErrorCode status)
{
    qCDebug(KBLOG_LOG);
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }

    bool success = false;

    BlogPost *post = mFetchPostMap.take(loader);
    qCritical() << "Post" << post;
    post->postId();

    if (status != Syndication::Success) {
        Q_EMIT q->errorPost(GData::Atom, i18n("Could not get posts."), post);
        return;
    }

    QString postId = post->postId();
    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) != -1 &&
                rx.cap(1) == postId) {
            qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
            post->setPostId(rx.cap(1));
            post->setTitle((*it)->title());
            post->setContent((*it)->content());
            post->setStatus(BlogPost::Fetched);
            post->setLink(QUrl((*it)->link()));
            post->setCreationDateTime(QDateTime::fromSecsSinceEpoch((*it)->datePublished()).toLocalTime());
            post->setModificationDateTime(QDateTime::fromSecsSinceEpoch((*it)->dateUpdated()).toLocalTime());
            qCDebug(KBLOG_LOG) << "Emitting fetchedPost( postId=" << postId << ");";
            success = true;
            Q_EMIT q->fetchedPost(post);
            break;
        }
    }
    if (!success) {
        qCritical() << "QRegExp rx( 'post-(\\d+)' does not match"
                    << mFetchPostMap[ loader ]->postId() << ".";
        Q_EMIT q->errorPost(GData::Other, i18n("Could not regexp the blog id path."), post);
    }
}

void GDataPrivate::slotCreatePost(KJob *job)
{
    qCDebug(KBLOG_LOG);
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());

    Q_Q(GData);

    KBlog::BlogPost *post = mCreatePostMap[ job ];
    mCreatePostMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotCreatePost error:" << job->errorString();
        Q_EMIT q->errorPost(GData::Atom, job->errorString(), post);
        return;
    }

    QRegExp rxId(QStringLiteral("post-(\\d+)"));   //FIXME check and do better handling, esp the creation date time
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

    QRegExp rxPub(QStringLiteral("<published>(.+)</published>"));
    if (rxPub.indexIn(data) == -1) {
        qCritical() << "Could not regexp the published time out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the published time out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

    QRegExp rxUp(QStringLiteral("<updated>(.+)</updated>"));
    if (rxUp.indexIn(data) == -1) {
        qCritical() << "Could not regexp the update time out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the update time out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);

    post->setPostId(rxId.cap(1));
    post->setCreationDateTime(QDateTime::fromString(rxPub.cap(1)));
    post->setModificationDateTime(QDateTime::fromString(rxUp.cap(1)));
    post->setStatus(BlogPost::Created);
    qCDebug(KBLOG_LOG) << "Emitting createdPost()";
    Q_EMIT q->createdPost(post);
}

void GDataPrivate::slotModifyPost(KJob *job)
{
    qCDebug(KBLOG_LOG);
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());

    KBlog::BlogPost *post = mModifyPostMap[ job ];
    mModifyPostMap.remove(job);
    Q_Q(GData);
    if (job->error() != 0) {
        qCritical() << "slotModifyPost error:" << job->errorString();
        Q_EMIT q->errorPost(GData::Atom, job->errorString(), post);
        return;
    }

    QRegExp rxId(QStringLiteral("post-(\\d+)"));   //FIXME check and do better handling, esp creation date time
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

    QRegExp rxPub(QStringLiteral("<published>(.+)</published>"));
    if (rxPub.indexIn(data) == -1) {
        qCritical() << "Could not regexp the published time out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the published time out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

    QRegExp rxUp(QStringLiteral("<updated>(.+)</updated>"));
    if (rxUp.indexIn(data) == -1) {
        qCritical() << "Could not regexp the update time out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the update time out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
    post->setPostId(rxId.cap(1));
    post->setCreationDateTime(QDateTime::fromString(rxPub.cap(1)));
    post->setModificationDateTime(QDateTime::fromString(rxUp.cap(1)));
    post->setStatus(BlogPost::Modified);
    Q_EMIT q->modifiedPost(post);
}

void GDataPrivate::slotRemovePost(KJob *job)
{
    qCDebug(KBLOG_LOG);
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());

    KBlog::BlogPost *post = mRemovePostMap[ job ];
    mRemovePostMap.remove(job);
    Q_Q(GData);
    if (job->error() != 0) {
        qCritical() << "slotRemovePost error:" << job->errorString();
        Q_EMIT q->errorPost(GData::Atom, job->errorString(), post);
        return;
    }

    post->setStatus(BlogPost::Removed);
    qCDebug(KBLOG_LOG) << "Emitting removedPost()";
    Q_EMIT q->removedPost(post);
}

void GDataPrivate::slotCreateComment(KJob *job)
{
    qCDebug(KBLOG_LOG);
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());
    qCDebug(KBLOG_LOG) << "Dump data: " << data;

    Q_Q(GData);

    KBlog::BlogComment *comment = mCreateCommentMap[ job ].cbegin().value();
    KBlog::BlogPost *post = mCreateCommentMap[ job ].cbegin().key();
    mCreateCommentMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotCreateComment error:" << job->errorString();
        Q_EMIT q->errorComment(GData::Atom, job->errorString(), post, comment);
        return;
    }

// TODO check for result and fit appropriately
    QRegExp rxId(QStringLiteral("post-(\\d+)"));
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

    QRegExp rxPub(QStringLiteral("<published>(.+)</published>"));
    if (rxPub.indexIn(data) == -1) {
        qCritical() << "Could not regexp the published time out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the published time out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

    QRegExp rxUp(QStringLiteral("<updated>(.+)</updated>"));
    if (rxUp.indexIn(data) == -1) {
        qCritical() << "Could not regexp the update time out of the result:" << data;
        Q_EMIT q->errorPost(GData::Atom,
                          i18n("Could not regexp the update time out of the result."), post);
        return;
    }
    qCDebug(KBLOG_LOG) << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
    comment->setCommentId(rxId.cap(1));
    comment->setCreationDateTime(QDateTime::fromString(rxPub.cap(1)));
    comment->setModificationDateTime(QDateTime::fromString(rxUp.cap(1)));
    comment->setStatus(BlogComment::Created);
    qCDebug(KBLOG_LOG) << "Emitting createdComment()";
    Q_EMIT q->createdComment(post, comment);
}

void GDataPrivate::slotRemoveComment(KJob *job)
{
    qCDebug(KBLOG_LOG);
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data().constData(), stj->data().size());

    Q_Q(GData);

    KBlog::BlogComment *comment = mRemoveCommentMap[ job ].cbegin().value();
    KBlog::BlogPost *post = mRemoveCommentMap[ job ].cbegin().key();
    mRemoveCommentMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotRemoveComment error:" << job->errorString();
        Q_EMIT q->errorComment(GData::Atom, job->errorString(), post, comment);
        return;
    }

    comment->setStatus(BlogComment::Created);
    qCDebug(KBLOG_LOG) << "Emitting removedComment()";
    Q_EMIT q->removedComment(post, comment);
}

#include "moc_gdata.cpp"
