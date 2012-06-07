#ifndef FORMAT_SUP_HPP
#define FORMAT_SUP_HPP

#include "subtitle.hpp"

#include <iostream>
#include <list>

bool load_sup(std::istream* in, std::list<Subtitle> subs);
bool save_sup(std::ostream* out, std::list<Subtitle> subs);

#endif /* FORMAT_SUP_HPP */
