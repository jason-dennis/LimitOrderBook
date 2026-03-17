//
// Created by Ognean Jason Dennis on 17.03.2026.
//

/*
 * the app need to work like every trader have an account
 * generate a valid id
 * have a section with my order to can cancel/modify
 *
 * i need account? maybe not
 *
 *
 *
 *
 *  one engine for every symbol
 *
 *  i need:
 *      - to add an order
 *      - cancel an order
 *      - get history trades
 *

    UI-> UserEngine -> AppEngine -> MatchingEngine



 *
 */
#ifndef LIMITORDERBOOK_APPENGINE_H
#define LIMITORDERBOOK_APPENGINE_H
#include "MatchingEngine.h"
#include <unordered_map>

class AppEngine{
private:

    std::unordered_map<std::string,MatchingEngine&> Engine;
public:

    AppEngine() = default;
    ~AppEngine() = default;




};


#endif //LIMITORDERBOOK_APPENGINE_H
