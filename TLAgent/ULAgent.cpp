//
// Created by ljw on 10/21/21.
//

#include "ULAgent.h"

using namespace tl_agent;

namespace tl_agent{

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class ULAgent : public BaseAgent<ReqField, RespField, EchoField, N> {
    private:
        Port<ReqField, RespField, EchoField, N> *port;
        ScoreBoard<std::array<uint8_t, N>> *globalBoard;
        //coreBoard<T> localBoard; TODO

    public:
        Resp send_a(ChnA<ReqField, EchoField> &a);
        void handle_b();
        Resp send_c(ChnC<ReqField, EchoField, N> &c);
        void handle_d();
        void fire_a();
        void fire_b();
        void fire_c();
        void fire_e();
        ULAgent(ScoreBoard<std::array<uint8_t, N>> * const gb);
        ~ULAgent() = default;

    private:

    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    Resp ULAgent<ReqField, RespField, EchoField, N>::send_a(ChnA<ReqField, EchoField> &a) {
        return OK;
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    Resp ULAgent<ReqField, RespField, EchoField, N>::send_c(ChnC<ReqField, EchoField, N> &c) {
        return OK;
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_a() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_b() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_c() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_e() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::handle_b() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::handle_d() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    ULAgent<ReqField, RespField, EchoField, N>::ULAgent(ScoreBoard<std::array<uint8_t, N>> *gb) {
        globalBoard = gb;
    }

}