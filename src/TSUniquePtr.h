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

template<typename T, typename U>
bool defaultComparer(T *ptr, T *comp)
{
    return (*static_cast<U *>(ptr)) == (*static_cast<U *>(comp));
}

template<typename T>
class TSUniquePtr : public std::unique_ptr<T, void(*)(T *)>
{
public:
    template<typename U>
    inline TSUniquePtr(U *ptr) :
        std::unique_ptr<T, void(*)(T *)>(ptr, defaultDeleter<T, U>),
        copier(defaultCopier<T, U>),
        comparer(defaultComparer<T, U>)
    {
    }
    TSUniquePtr(const TSUniquePtr &ptr) = delete;
    inline TSUniquePtr(TSUniquePtr &&ptr) :
        std::unique_ptr<T, void(*)(T *)>(std::move(ptr)),
        copier(std::move(ptr.copier))
    {
    }
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
    inline bool compareContents(const TSUniquePtr &comp)
    {
        return comparer(this->get(), comp.get());
    }
private:
    std::function<TSUniquePtr<T>(T *)> copier;
    std::function<bool(T *, T *)> comparer;
};

#endif
