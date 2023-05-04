#pragma once

#include <sstream>

namespace format {
    // Formatting
    std::wostream &reset(std::wostream &out) {
        out << "\033[0m";
        return out;
    }

    std::wostream &bright(std::wostream &out) {
        out << "\033[1m";
        return out;
    }

    std::wostream &dim(std::wostream &out) {
        out << "\033[2m";
        return out;
    }

    std::wostream &italic(std::wostream &out) {
        out << "\033[3m";
        return out;
    }

    std::wostream &underline(std::wostream &out) {
        out << "\033[4m";
        return out;
    }

    std::wostream &blink(std::wostream &out) {
        out << "\033[5m";
        return out;
    }

    std::wostream &invert(std::wostream &out) {
        out << "\033[7m";
        return out;
    }

    std::wostream &hidden(std::wostream &out) {
        out << "\033[8m";
        return out;
    }

    std::wostream &cross(std::wostream &out) {
        out << "\033[9m";
        return out;
    }

    std::wostream &doubleunderline(std::wostream &out) {
        out << "\033[21m";
        return out;
    }
}

namespace color {
  // Foreground colors
  std::wostream &black(std::wostream &out) {
    out << "\033[30m";
    return out;
  }

  std::wostream &red(std::wostream &out) {
    out << "\033[31m";
    return out;
  }

  std::wostream &green(std::wostream &out) {
    out << "\033[32m";
    return out;
  }

  std::wostream &yellow(std::wostream &out) {
    out << "\033[33m";
    return out;
  }

  std::wostream &blue(std::wostream &out) {
    out << "\033[34m";
    return out;
  }

  std::wostream &purple(std::wostream &out) {
    out << "\033[35m";
    return out;
  }

  std::wostream &cyan(std::wostream &out) {
    out << "\033[36m";
    return out;
  }

  std::wostream &white(std::wostream &out) {
    out << "\033[37m";
    return out;
  }

  namespace light {
    std::wostream &black(std::wostream &out) {
      out << "\033[90m";
      return out;
    }

    std::wostream &red(std::wostream &out) {
      out << "\033[91m";
      return out;
    }

    std::wostream &green(std::wostream &out) {
      out << "\033[92m";
      return out;
    }

    std::wostream &yellow(std::wostream &out) {
      out << "\033[93m";
      return out;
    }

    std::wostream &blue(std::wostream &out) {
      out << "\033[94m";
      return out;
    }

    std::wostream &purple(std::wostream &out) {
      out << "\033[95m";
      return out;
    }

    std::wostream &cyan(std::wostream &out) {
      out << "\033[96m";
      return out;
    }

    std::wostream &white(std::wostream &out) {
      out << "\033[97m";
      return out;
    }
  }

  // Background colors
  namespace background {
    std::wostream &black(std::wostream &out) {
      out << "\033[40m";
      return out;
    }

    std::wostream &red(std::wostream &out) {
      out << "\033[41m";
      return out;
    }

    std::wostream &green(std::wostream &out) {
      out << "\033[42m";
      return out;
    }

    std::wostream &yellow(std::wostream &out) {
      out << "\033[43m";
      return out;
    }

    std::wostream &blue(std::wostream &out) {
      out << "\033[44m";
      return out;
    }

    std::wostream &purple(std::wostream &out) {
      out << "\033[45m";
      return out;
    }

    std::wostream &cyan(std::wostream &out) {
      out << "\033[46m";
      return out;
    }

    std::wostream &white(std::wostream &out) {
      out << "\033[47m";
      return out;
    }

    namespace light {
      std::wostream &black(std::wostream &out) {
        out << "\033[100m";
        return out;
      }

      std::wostream &red(std::wostream &out) {
        out << "\033[101m";
        return out;
      }

      std::wostream &green(std::wostream &out) {
        out << "\033[102m";
        return out;
      }

      std::wostream &yellow(std::wostream &out) {
        out << "\033[103m";
        return out;
      }

      std::wostream &blue(std::wostream &out) {
        out << "\033[104m";
        return out;
      }

      std::wostream &purple(std::wostream &out) {
        out << "\033[105m";
        return out;
      }

      std::wostream &cyan(std::wostream &out) {
        out << "\033[106m";
        return out;
      }

      std::wostream &white(std::wostream &out) {
        out << "\033[107m";
        return out;
      }
    }
  }
}