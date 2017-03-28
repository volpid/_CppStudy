
#include <iostream>
#include <memory>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

using FloatFunc = float (*) (float);

class screensaver
{
public:
    struct customization
    {
        virtual ~customization(void) {}
        virtual float operator()(float) const = 0;
    };

    explicit screensaver(std::unique_ptr<customization> c)
        : get_seed(std::move(c))
    {}

    void next_screen(void)
    {
        /*
            @Note
            - if we use function pointer, we are not free from arg!!
        */
        float dummyfloat = 0.0f;
        float seed = (*this->get_seed)(dummyfloat);
    }

private:
    std::unique_ptr<customization> get_seed;
};

struct hypnotic : public screensaver::customization
{
    float operator()(float x) const
    {
        cout << "struct hypnotic : public screensaver::customization" << endl;
        //use this->state
        return x;
    }
    
    std::vector<int> state;
};

struct funwrapper : public screensaver::customization
{
    funwrapper(FloatFunc _pf)
        : pf(_pf)
    {}

    float operator()(float x) const
    {
        cout << "struct funwrapper : public screensaver::customization" << endl;
        //use this->state
        return (this->pf)(x);
    }
    
    FloatFunc pf;
};

/*
    @Note
    - using wrapper
*/

class screensaverwrapper
{
private:
    struct customization
    {
        virtual ~customization(void) {}
        virtual float operator()(float) const = 0;
    };

    template <typename _F>
    struct wrapper : public customization
    {
        explicit wrapper(_F _f)
            : f(_f)
        {}

        float operator()(float x) const
        {
            cout << "struct wrapper : public customization" << endl;
            return (this->f)(x);
        }
    private:
        _F f;
    };

public:
    template <typename _F>
    explicit screensaverwrapper(const _F& f)
        : get_seed(new wrapper<_F>(f))
    {}

    void next_screen(void)
    {
        /*
            @Note
            - wrapper does calculate now
        */
        float dummyfloat = 0.0f;
        float seed = (*this->get_seed)(dummyfloat);
    }

private:
    std::unique_ptr<customization> get_seed;
};

/*
    @Note
    - try keep interface
    - use float_function
    - easy way to use float_function
        typedef boost::function<float(float x)> float_function
        typedef std::function<float(float x)> float_function
    
*/

struct float_function
{
private:
    struct impl
    {
        virtual ~impl(void) {}
        virtual impl* clone(void) const = 0;
        virtual float operator()(float) const = 0;
    };

    template <typename _F>
    struct wrapper : impl
    {
        explicit wrapper(const _F _f)
            : f(_f)
        {}

        virtual impl* clone(void) const
        {
            return new wrapper<_F>(this->f);
        }

        virtual float operator()(float x) const
        {
            cout << "struct wrapper : impl" << endl;
            return (this->f)(x);
        }

    private :
        _F f;
    };

public:
    
    template <typename _F>
    float_function(const _F& f)
        : pimpl(new wrapper<_F>(f))
    {}

    float_function(const float_function& rhs)
        : pimpl(rhs.pimpl->clone())
    {}

    float_function& operator=(const float_function& rhs)
    {
        this->pimpl.reset(rhs.pimpl->clone());
        return *this;
    }

    float operator()(float x) const
    {
        return (*this->pimpl)(x);
    }

private:
    std::unique_ptr<impl> pimpl;
};

class screensaverkeepinterface
{
public:
    explicit screensaverkeepinterface(float_function f)
        : get_seed(f)
    {}

    void next_screen(void)
    {
        /*
            @Note
            - wrapper does calculate now
        */
        float dummyfloat = 0.0f;
        float seed = (this->get_seed)(dummyfloat);
    }

private:
    float_function get_seed;
};

float DummyFunc(float x)
{
    cout << "float DummyFunc(float x)" << endl;
    return 0.0f;
}

float DummyFunc2(float x)
{
    cout << "float DummyFunc2(float x)" << endl;
    return 0.0f;
}

static int _main(int argc, char** argv)
{
    /*simple*/
    screensaver screensaver1(std::unique_ptr<hypnotic>(new hypnotic()));
    screensaver screensaver2(std::unique_ptr<funwrapper>(new funwrapper(DummyFunc)));
    
    screensaver1.next_screen();
    screensaver2.next_screen();
    cout << endl;

    /*wrapper*/
    screensaverwrapper screensaverwrapper1(&DummyFunc);
    screensaverwrapper screensaverwrapper2(&DummyFunc2);

    screensaverwrapper1.next_screen();
    screensaverwrapper2.next_screen();
    cout << endl;

    /*keep interface*/
    float_function ff1(&DummyFunc);
    float_function ff2(&DummyFunc2);
    screensaverkeepinterface screensaverkeepinterface1(ff1);
    screensaverkeepinterface screensaverkeepinterface2(ff2);

    screensaverkeepinterface1.next_screen();
    screensaverkeepinterface2.next_screen();

    return 0;
}

 