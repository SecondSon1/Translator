namespace color {
    // Formatting
    std::ostream& reset(std::ostream& out) {
        out << "\033[0m";
        return out;
    }

    std::ostream& bright(std::ostream& out) {
        out << "\033[1m";
        return out;
    }

    std::ostream& underline(std::ostream& out) {
        out << "\033[4m";
        return out;
    }

    std::ostream& flashing(std::ostream& out) {
        out << "\033[5m";
        return out;
    }

    // Foreground colors
    std::ostream& black(std::ostream& out) {
        out << "\033[30m";
        return out;
    }

    std::ostream& red(std::ostream& out) {
        out << "\033[31m";
        return out;
    }

    std::ostream& green(std::ostream& out) {
        out << "\033[32m";
        return out;
    }

    std::ostream& yellow(std::ostream& out) {
        out << "\033[33m";
        return out;
    }

    std::ostream& blue(std::ostream& out) {
        out << "\033[34m";
        return out;
    }

    std::ostream& purple(std::ostream& out) {
        out << "\033[35m";
        return out;
    }

    std::ostream& cyan(std::ostream& out) {
        out << "\033[36m";
        return out;
    }

    std::ostream& white(std::ostream& out) {
        out << "\033[37m";
        return out;
    }

    // Background colors
    namespace background {
        std::ostream &black(std::ostream &out) {
            out << "\033[30m";
            return out;
        }

        std::ostream &red(std::ostream &out) {
            out << "\033[31m";
            return out;
        }

        std::ostream &green(std::ostream &out) {
            out << "\033[32m";
            return out;
        }

        std::ostream &yellow(std::ostream &out) {
            out << "\033[33m";
            return out;
        }

        std::ostream &blue(std::ostream &out) {
            out << "\033[34m";
            return out;
        }

        std::ostream &purple(std::ostream &out) {
            out << "\033[35m";
            return out;
        }

        std::ostream &cyan(std::ostream &out) {
            out << "\033[36m";
            return out;
        }

        std::ostream &white(std::ostream &out) {
            out << "\033[37m";
            return out;
        }
    }
}
