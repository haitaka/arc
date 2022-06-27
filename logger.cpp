#include "logger.h"

void log(std::string const & msg) {
    std::stringstream buf;
    buf << msg << std::endl;
    log(buf);
}

void log(std::stringstream const & buf) {
    // TODO move this check outside to prevent redundant string manipulations, if a compiler would fail to.
    if (ENABLED) {
        std::stringstream finalBuf;
        finalBuf << "[Thread " << std::hex << std::this_thread::get_id() << std::dec << "] " << buf.str();
        std::clog << finalBuf.str();
    }
}
