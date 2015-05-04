#ifndef _TSUNIQUEPTR_H_
#define _TSUNIQUEPTR_H_

#include <memory>
#include <functional>
#include <utility>

template<typename T>
class TSUniquePtr;

template<typename T, typename U>
void defaultDeleter(T *ptr)
{
    delete static_cast<U *>(ptr);
}

template<typename T, typename U>
TSUniquePtr<T> defaultCopier(T *ptr)
{
    return TSUniquePtr<T>(new U(*static_cast<U *>(ptr)));
}

template<typename T>
class TSUniquePtr : public std::unique_ptr<T, void(*)(T *)>
{
public:
    constexpr TSUniquePtr() :
        std::unique_ptr<T, void(*)(T *)>()
    {}

    template<typename U>
    inline TSUniquePtr(U *ptr) :
        std::unique_ptr<T, void(*)(T *)>(ptr, defaultDeleter<T, U>),
        copier(defaultCopier<T, U>)
    {}

    TSUniquePtr(const TSUniquePtr &ptr) = delete;
    inline TSUniquePtr(TSUniquePtr &&ptr) :
        std::unique_ptr<T, void(*)(T *)>(std::move(ptr)),
        copier(std::move(ptr.copier))
    {}

    TSUniquePtr &operator=(const TSUniquePtr &ptr) = delete;
    
    inline TSUniquePtr &operator=(TSUniquePtr &&ptr)
    {
        std::unique_ptr<T, void(*)(T *)>::operator=(std::move(ptr));
        copier = std::move(ptr.copier);

        return *this;
    }

    inline TSUniquePtr<T> copy() const
    {
        return copier(this->get());
    }
    
private:
    std::function<TSUniquePtr<T>(T *)> copier;
};

#endif
