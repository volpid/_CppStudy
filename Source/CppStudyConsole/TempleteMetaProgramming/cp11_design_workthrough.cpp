
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>

#include <assert.h>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

//-------------------------------------------------------------------------------------------------
/*FSM*/

template <typename _Derived>
class stats_machine 
{
protected:
    stats_machine(void)
        : state(_Derived::initial_state)
    {}

    template <
        int CurrentState,
        typename _Event,
        int NextState,
        void (_Derived::*action)(const _Event&)
    >
    struct row
    {
        static const int current_state = CurrentState;
        static const int next_state = NextState;
        using event = _Event;
        using fsm_t = _Derived;

        static void execute(_Derived& fms, const _Event& e)
        {
            (fms.*action)(e);
        }
    };


    template <typename _Event>
    int no_transition(int state, const _Event& e)
    {
        assert(false);
        return state;
    }

public:
    template <typename _Event>
    int process_event(const _Event& evt)
    {
        using dispatcher = typename generate_dispatcher<typename _Derived::transition_table, _Event>::type;

        this->state = dispatcher::dispatch(
            *(static_cast<_Derived*> (this)),
            this->state,
            evt);

        return this->state;
    }

    template <typename _Event>
    int call_no_transition(int state, const _Event& e)
    {
        return (static_cast<_Derived*> (this))->no_transition(state, e);
    }

private:    
    int state;
};

/*
    @Note 
    - event disptch naive way
    
        void process_event(const play& e)
        {
            switch (this->state)
            {
                case Stopped:
                {
                    this->start_playback(e);
                    this->state = Playing;
                }
                break;
                case Paused:
                {
                    this->start_playback(e);
                    this->state = Playing;
                }
                default:
                {
                    this->state = no_transition(this->state, e);
                }
                break;
            }
        }
*/

template <typename _Transition, typename _Next>
struct event_dispatcher
{
    typedef typename _Transition::fsm_t fsm_t;
    typedef typename _Transition::event event;
    
    static int dispatch(fsm_t& fsm, int state, const event& e)
    {
        if (state == _Transition::current_state)
        {
            _Transition::execute(fsm, e);
            return _Transition::next_state;
        }
        else
        {
            return _Next::dispatch(fsm, state, e);
        }
    }
};

struct default_event_dispatcher
{
    template <typename _FSM, typename _Event>
    static int dispatch(stats_machine<_FSM>& m, int state, const _Event& e)
    {
        return m.call_no_transition(state, e);
    }
};

template <typename _Transition>
struct transition_event
{   
    using type = typename _Transition::event;
};

template <typename _Table, typename _Event>
struct generate_dispatcher
    : public mpl::fold<
        mpl::filter_view<
            _Table,
            boost::is_same<_Event, transition_event<mpl::_1>>
        >,
        default_event_dispatcher,
        event_dispatcher<mpl::_2, mpl::_1>
    >
    /*
        event_dispatcher<
            row<Stopped, play, Playing, &player::start_playback>,
            event_dispatcher<paused, Playing, &player::resume_playback>,
            default_event_dispatcher
        >
    */
{};

//-------------------------------------------------------------------------------------------------
/**/

//events
struct play {};
struct open_close {};
struct cd_detected {cd_detected(...) {}};
struct pause {};
struct stop {};

class Player : public stats_machine<Player>
{
    friend class stats_machine<Player>;

private:
    enum States
    {
        Stopped,
        Open, 
        Empty,
        Playing,
        Paused,
        initial_state = Empty
    };
    
    // transition behavior
    void start_playback(const play&)
    {
        cout << "void start_playback(const play&)" << endl;
    }
    void open_drawer(const open_close&)
    {
        cout << "void open_drawer(const open_close&)" << endl;
    }
    void close_drawer(const open_close&)
    {
        cout << "void close_drawer(const open_close&)" << endl;
    }
    void store_cd_info(const cd_detected&)
    {
        cout << "void store_cd_info(const cd_detected&)" << endl;
    }
    void stop_playback(const stop&)
    {
        cout << "void stop_playback(const stop&)" << endl;
    }
    void puase_playback(const pause&)
    {
        cout << "void puase_playback(const pause&)" << endl;
    }
    void resume_playbakc(const play&)
    {
        cout << "void resume_playbakc(const play&)" << endl;
    }
    void stop_and_open(const open_close&)
    {
        cout << "void stop_and_open(const open_close&)" << endl;
    }
    
    using P = Player;
    struct transition_table 
        :  public mpl::vector<
            //  cur state   event           next state  behavior
            row<Stopped,    play,           Playing,    &P::start_playback>,
            row<Stopped,    open_close,     Open,       &P::open_drawer>,            
            row<Open,       open_close,     Empty,      &P::close_drawer>,
            row<Empty,      open_close,     Open,       &P::open_drawer>,
            row<Empty,      cd_detected,    Stopped,    &P::store_cd_info>,
            row<Playing,    stop,           Stopped,    &P::stop_playback>,
            row<Playing,    pause,          Paused,     &P::puase_playback>,
            row<Playing,    open_close,     Open,       &P::stop_and_open>,
            row<Paused,     play,           Playing,    &P::resume_playbakc>,
            row<Paused,     stop,           Stopped,    &P::stop_playback>,
            row<Paused,     open_close,     Open,       &P::stop_and_open>
        >
    {};
};

int _main(int argc, char** argv)
{
    Player p;
    int state = 0;

    state = p.process_event(open_close());
    cout << "cur state : " << state << endl;
    state = p.process_event(open_close());
    cout << "cur state : " << state << endl;
    state = p.process_event(cd_detected("Louie, Louie", std::vector<std::clock_t>()));
    cout << "cur state : " << state << endl;
    state = p.process_event(play());
    cout << "cur state : " << state << endl;
    state = p.process_event(pause());
    cout << "cur state : " << state << endl;
    state = p.process_event(play());
    cout << "cur state : " << state << endl;
    state = p.process_event(stop());
    cout << "cur state : " << state << endl;

    return 0;
}