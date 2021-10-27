//
// Created by Kaifan Wang on 2021/10/25.
//

#include <map>
#include <array>
#include "Common.h"

#ifndef TLC_TEST_SCOREBOARD_H
#define TLC_TEST_SCOREBOARD_H

const int ERR_NOTFOUND = 1;
const int ERR_MISMATCH = 2;

template<typename Tk, typename Tv>
class ScoreBoard {
private:
    std::map<Tk, std::shared_ptr<Tv>> mapping;
public:
    ScoreBoard();
    ~ScoreBoard();
    void update(const Tk& key, std::shared_ptr<Tv>& data);
    std::shared_ptr<Tv> get(const Tk& key);
    void erase(const Tk& key);
    int verify(const Tk& key, const Tv& data) const;
};


/************************** Implementation **************************/

template<typename Tk, typename Tv>
ScoreBoard<Tk, Tv>::ScoreBoard() {
    mapping.clear();
}

template<typename Tk, typename Tv>
ScoreBoard<Tk, Tv>::~ScoreBoard() {
    delete mapping;
}

template<typename Tk, typename Tv>
void ScoreBoard<Tk, Tv>::update(const Tk& key, std::shared_ptr<Tv>& data) {
    mapping.insert(std::make_pair(key, data));
}

template<typename Tk, typename Tv>
std::shared_ptr<Tv> ScoreBoard<Tk, Tv>::get(const Tk& key) {
    if (mapping.count(key) > 0) {
        return mapping[key];
    } else {
        tlc_assert(false, "Key no found!");
    }
}

template<typename Tk, typename Tv>
void ScoreBoard<Tk, Tv>::erase(const Tk& key) {
    int num = mapping.erase(key);
    tlc_assert(num == 1, "Multiple value mapped to one key!");
}

template<typename Tk, typename Tv>
int ScoreBoard<Tk, Tv>::verify(const Tk& key, const Tv& data) const {
    if (mapping.count(key) > 0) {
        if (*mapping[key] != data) {
            return ERR_MISMATCH;
        }
        return 0;
    }
    return ERR_NOTFOUND;
}

#endif // TLC_TEST_SCOREBOARD_H
