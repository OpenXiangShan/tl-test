//
// Created by Kaifan Wang on 2021/10/25.
//

#include <map>
#include <array>
#include <string>

#ifndef TLC_TEST_SCOREBOARD_H
#define TLC_TEST_SCOREBOARD_H

const int ERR_NOTFOUND = 1;
const int ERR_MISMATCH = 2;

template<typename T>
class ScoreBoard {
private:
    std::map<uint64_t, T> mapping;
public:
    ScoreBoard();
    ~ScoreBoard();
    void update(const uint64_t& address, const T& data);
    void erase(const uint64_t& address);
    int verify(const uint64_t& address, const T& data) const;
};


/************************** Implementation **************************/

template<typename T>
ScoreBoard<T>::ScoreBoard() {
    mapping.clear();
}

template<typename T>
ScoreBoard<T>::~ScoreBoard() {
    delete mapping;
}

template<typename T>
void ScoreBoard<T>::update(const uint64_t& address, const T& data) {
    mapping.insert(std::make_pair(address, data));
}

template<typename T>
void ScoreBoard<T>::erase(const uint64_t& address) {
    int num = mapping.erase(address);
    assert(num == 1);
}

template<typename T>
int ScoreBoard<T>::verify(const uint64_t& address, const T& data) const {
    if (mapping.count(address) > 0) {
        if (mapping[address] != data) {
            return ERR_MISMATCH;
        }
        return 0;
    }
    return ERR_NOTFOUND;
}

#endif // TLC_TEST_SCOREBOARD_H
