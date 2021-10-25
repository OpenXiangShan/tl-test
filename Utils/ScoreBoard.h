//
// Created by Kaifan Wang on 2021/10/25.
//

#include <map>
#include <array>
#include <string>

#ifndef TLC_TEST_SCOREBOARD_H
#define TLC_TEST_SCOREBOARD_H

#define ERR_NOTFOUND 1
#define ERR_MISMATCH 2

template<typename T>
class ScoreBoard {
private:
    std::map<uint64_t, T> *mapping;
public:
    ScoreBoard();
    ~ScoreBoard();
    void update(const uint64_t *address, T *data);
    int verify(const uint64_t *address, T *data);
};


/************************** Implementation **************************/

template<typename T>
ScoreBoard<T>::ScoreBoard() {
    mapping = new std::map<uint64_t, T>();
}

template<typename T>
ScoreBoard<T>::~ScoreBoard() {
    delete mapping;
}

template<typename T>
void ScoreBoard<T>::update(const uint64_t *address, T *data) {
    mapping->insert(std::make_pair(*address, *data));
}

template<typename T>
int ScoreBoard<T>::verify(const uint64_t *address, T *data) {
    if (mapping->count(*address) > 0) {
        if (mapping[address] != &data) {
            return ERR_MISMATCH;
        }
        return 0;
    }
    return ERR_NOTFOUND;
}

#endif // TLC_TEST_SCOREBOARD_H
