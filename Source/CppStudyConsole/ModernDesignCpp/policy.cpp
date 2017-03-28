
#include <exception>
#include <iostream>
#include <stdio.h>

using namespace std;

template <typename _T>
struct OpNewCreator
{
    static _T* Create(void)
    {
        return new _T();
    }
};

template <typename _T>
struct MallocCreator
{
    static _T* Create(void)
    {
        void* buf = std::malloc(sizeof(_T));
        if (buf == nullptr) 
        {
            return nullptr;
        }

        return new(buf) _T();
    }
};

template <typename _T>
struct PrototypeCreator
{
public:
    PrototypeCreator(_T* pObj = nullptr) 
        : pPrototype_(pObj)
    {}

    static _T* Creator(void)
    {
        return (pPrototype_ == nullptr) ? (pPrototype_->Clone()) : (nullptr);
    }

    _T* GetPrototype(void) 
    {
        return pPrototype_;
    }

    void SetPrototype(_T* pObj)
    {
        pPrototype_ = pObj;
    }

private:
    _T* pPrototype_;
};

class Widget
{
public:
    Widget(void)
    {
        cout << (int) this << "Widget(void)" << endl;
    }
    
    ~Widget(void)
    {
        cout << (int) this << "~Widget(void)" << endl;
    }
    void Call(void)
    {
        cout << (int) this << "call" << endl;
    }
};

template <template <typename _Created> class CreationPolicy>
class WidgetManager : public CreationPolicy<Widget>
{
public:
    void DoSomething(void)
    {
        Widget* pW = CreationPolicy<Widget>::Create();
    }

    void SwitchPrototype(Widget* pNewPrototype)
    {
        CreationPolicy<Widget>& myPolicy = *this;
        delete myPolicy.GetPrototype();
        myPolicy.SetPrototype(pNewPrototype);
    }
};

typedef WidgetManager<OpNewCreator> MyWidgetMgr;

template <typename _T>
struct NoChecking
{
    static void Check(_T* ptr)
    {
        cout << "NoChecking" << endl;
    }
};

template <typename _T>
struct EnforceNotNull
{
    class NullPointException : public std::exception
    {
    };

    static void Check(_T* ptr)
    {
        cout << "EnforceNotNull" << endl;
        if (ptr == nullptr)
        {
            throw NullPointException();
        }
    }
};

template <typename _T>
struct EnsureNotNull
{
    static void Check(_T*& ptr)
    {
        cout << "EnsureNotNull" << endl;
        if (ptr == nullptr)
        {
            ptr = GetDefaultValue();
        }
    }

    static _T* GetDefaultValue(void)
    {   
        static _T dummy;
        return &dummy;
    }
};

template <typename _T>
struct SingleThreaded
{
    struct NoGuard
    {   
        NoGuard(_T&) 
        {
            cout << "No Guard" << endl;
        }
    };

    typedef NoGuard Lock;
};

template <typename _T>
struct Threaded
{
    struct Guard
    {   
        Guard(_T&) 
        {
            cout << "Guard with _T" << endl;
        }
    };

    typedef Guard Lock;
};

/*
    @Note 
        - use two type of policy
        1. CheckPolicy
        2. ThreadPolicy
*/

template <
    typename _T, 
    template <typename> class CheckPolicy,
    template <typename> class ThreadPolicy
>
class SmartPtr : public CheckPolicy<_T>, public ThreadPolicy<_T>
{
public:
    SmartPtr(_T* ptr)
        : pointee_(ptr)
    {}

    ~SmartPtr(void)
    {
        delete pointee_;
    }

    _T* operator-> (void)
    {
        typename ThreadPolicy<SmartPtr>::Lock guard(*this);
        CheckPolicy<_T>::Check(pointee_);
        return pointee_;
    }

private:
    _T* pointee_ = nullptr;
};

static int _main(int argc, char** argv)
{
    MyWidgetMgr testWidgetMgr;
    testWidgetMgr.DoSomething();

    typedef SmartPtr<Widget, NoChecking, SingleThreaded> WidgetPtr;
    typedef SmartPtr<Widget, EnsureNotNull, Threaded> SafeWidetPtr;

    WidgetPtr ptr1(new Widget());
    SafeWidetPtr ptr2(new Widget());

    ptr1->Call();
    ptr2->Call();
    return 0;
}