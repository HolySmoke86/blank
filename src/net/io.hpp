#ifndef BLANK_NET_IO_HPP
#define BLANK_NET_IO_HPP

#include <iosfwd>


namespace blank {

std::ostream &operator <<(std::ostream &, const IPaddress &);

}

#endif
