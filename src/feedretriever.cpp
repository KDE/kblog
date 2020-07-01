/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedretriever.h"

#include <KIO/StoredTransferJob>

#include <QUrl>

using namespace KBlog;

FeedRetriever::FeedRetriever()
    : Syndication::DataRetriever()
{
}

void FeedRetriever::retrieveData(const QUrl &url)
{
    auto job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KJob::result, this, &FeedRetriever::getFinished);
    mJob = job;
    mJob->start();
}

int FeedRetriever::errorCode() const
{
    return mError;
}

void FeedRetriever::abort()
{
    if (mJob) {
        mJob->kill();
        mJob = nullptr;
    }
}

void FeedRetriever::getFinished(KJob *job)
{
    if (job->error()) {
        mError = job->error();
        Q_EMIT dataRetrieved({}, false);
        return;
    }

    Q_EMIT dataRetrieved(static_cast<KIO::StoredTransferJob*>(job)->data(), true);
}
