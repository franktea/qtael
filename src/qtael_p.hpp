#ifndef QTAEL_HPP_
#define QTAEL_HPP_

#include "qtael.hpp"
#include <boost/coroutine2/coroutine.hpp>
namespace qtael {

using Coroutine = boost::coroutines2::coroutine<void>;

class Async::Private : public QObject {
    Q_OBJECT
public:
    Private(Function task, QObject *parent);

signals:
    void finished();

public slots:
    void onResolve();

public:
    Function task;
    Coroutine::pull_type fork;
};

class Await::Private {
public:
    Private(Async &context, Coroutine::push_type &yield);

    Async &context;
    Coroutine::push_type &yield;
};

} // namespace qtael

#endif
