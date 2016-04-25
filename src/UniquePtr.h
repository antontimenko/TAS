#ifndef _UNIQUEPTR_H_
#define _UNIQUEPTR_H_

#include <memory>
#include <functional>
#include <utility>

template<typename T>
class UniquePtr;

template<typename T, typename U>
void defaultDeleter(T *ptr) {
    delete static_cast<U *>(ptr);
}

template<typename T, typename U>
UniquePtr<T> defaultCopier(T *ptr) {
    return UniquePtr<T>(new U(*static_cast<U *>(ptr)));
}

template<typename T>
class UniquePtr : public std::unique_ptr<T, void(*)(T *)> {
public:
    constexpr UniquePtr() :
        std::unique_ptr<T, void(*)(T *)>()
    {}

    template<typename U>
    inline UniquePtr(U *ptr) :
        std::unique_ptr<T, void(*)(T *)>(ptr, defaultDeleter<T, U>),
        copier(defaultCopier<T, U>)
    {}

    UniquePtr(const UniquePtr &ptr) = delete;
    inline UniquePtr(UniquePtr &&ptr) :
        std::unique_ptr<T, void(*)(T *)>(std::move(ptr)),
        copier(std::move(ptr.copier))
    {}

    UniquePtr &operator=(const UniquePtr &ptr) = delete;
    
    inline UniquePtr &operator=(UniquePtr &&ptr) {
        std::unique_ptr<T, void(*)(T *)>::operator=(std::move(ptr));
        copier = std::move(ptr.copier);

        return *this;
    }

    inline UniquePtr<T> copy() const {
        return copier(this->get());
    }
    
private:
    std::function<UniquePtr<T>(T *)> copier;
};

#endif
