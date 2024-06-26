#ifndef QTAEL_HPP
#define QTAEL_HPP

#include <functional>
#include <memory>
#include <tuple>
#include <utility>

#include <QtCore/QObject>
#include <QtCore/QVariantList>

#ifdef QTAEL_LIBRARY
#define QTAEL_DLL Q_DECL_EXPORT
#else
#define QTAEL_DLL Q_DECL_IMPORT
#endif

namespace qtael {

class Await;
using Function = std::function<void(const Await &)>;

class QTAEL_DLL Async : public QObject {
    Q_OBJECT
public:
    explicit Async(Function task, QObject *parent = 0);

    void start();
    void stop();

signals:
    void finished();

private:
    friend class Await;
    class Private;
    Private *d;
};

namespace detail {

class SignalIsolator : public QObject {
    Q_OBJECT
public:
    SignalIsolator() : QObject(), args() {}

    const QVariantList &getResult() const { return this->args; }

    template <typename... Args> void proxy(Args &&...args) {
        this->args = {args...};
        emit this->resolved();
    }

signals:
    void resolved();

private:
    QVariantList args;
};

template <typename T> using DecayType = typename std::decay<T>::type;

template <typename... Args>
using MIS = std::make_index_sequence<sizeof...(Args)>;

template <std::size_t... Indices> using IS = std::index_sequence<Indices...>;

template <typename... Args, std::size_t... Indices>
auto argsToTupleImpl(const QVariantList &args, IS<Indices...>) {
    return std::make_tuple(args.at(Indices).value<DecayType<Args>>()...);
}

template <typename... Args> auto argsToTuple(const QVariantList &args) {
    return argsToTupleImpl<Args...>(args, MIS<Args...>());
}
} // namespace detail

class QTAEL_DLL Await {
public:
    template <typename T, typename R, typename... Args>
    using SignalType = R (T::*)(Args...);

    // 等待一段时间，相当于协程的sleep
    void operator()(int interval) const;

    // 等待某一个signal的发生
    template <typename T, typename R, typename... Args>
    auto operator()(T *object, SignalType<T, R, Args...> signal_) const {
        detail::SignalIsolator proxy;
        QObject::connect(object, signal_, &proxy,
                         &detail::SignalIsolator::proxy<Args...>);
        this->yield(&proxy);
        return detail::argsToTuple<Args...>(proxy.getResult());
    }

private:
    friend class Async;
    class Private;

    Await(std::shared_ptr<Private> d);
    Await(const Await &);
    Await &operator=(const Await &);
    Await(Await &&);
    Await &operator=(Await &&);
    ~Await();

    void yield(QObject *object) const;

    std::shared_ptr<Private> d;
};

} // namespace qtael

#endif
