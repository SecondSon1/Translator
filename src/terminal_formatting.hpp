#pragma once

#include <sstream>

namespace format {
    // Formatting
    std::wostream &reset(std::wostream &out);

    std::wostream &bright(std::wostream &out);

    std::wostream &dim(std::wostream &out);

    std::wostream &italic(std::wostream &out);

    std::wostream &underline(std::wostream &out);

    std::wostream &blink(std::wostream &out);

    std::wostream &invert(std::wostream &out);

    std::wostream &hidden(std::wostream &out);

    std::wostream &cross(std::wostream &out);

    std::wostream &doubleunderline(std::wostream &out);
}

namespace color {
  // Foreground colors
  std::wostream &black(std::wostream &out);

  std::wostream &red(std::wostream &out);

  std::wostream &green(std::wostream &out);

  std::wostream &yellow(std::wostream &out);

  std::wostream &blue(std::wostream &out);

  std::wostream &purple(std::wostream &out);

  std::wostream &cyan(std::wostream &out);

  std::wostream &white(std::wostream &out);

  namespace light {
    std::wostream &black(std::wostream &out);

    std::wostream &red(std::wostream &out);

    std::wostream &green(std::wostream &out);

    std::wostream &yellow(std::wostream &out);

    std::wostream &blue(std::wostream &out);

    std::wostream &purple(std::wostream &out);

    std::wostream &cyan(std::wostream &out);

    std::wostream &white(std::wostream &out);
  }

  // Background colors
  namespace background {
    std::wostream &black(std::wostream &out);

    std::wostream &red(std::wostream &out);

    std::wostream &green(std::wostream &out);

    std::wostream &yellow(std::wostream &out);

    std::wostream &blue(std::wostream &out);

    std::wostream &purple(std::wostream &out);

    std::wostream &cyan(std::wostream &out);

    std::wostream &white(std::wostream &out);

    namespace light {
      std::wostream &black(std::wostream &out);

      std::wostream &red(std::wostream &out);

      std::wostream &green(std::wostream &out);

      std::wostream &yellow(std::wostream &out);

      std::wostream &blue(std::wostream &out);

      std::wostream &purple(std::wostream &out);

      std::wostream &cyan(std::wostream &out);

      std::wostream &white(std::wostream &out);
    }
  }
}